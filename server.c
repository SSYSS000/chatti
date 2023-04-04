#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>

#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "log.h"
#include "network.h"
#include "chat.h"

#define PORT                                            14023
#define MAX_NUM_CONNECTIONS                             16

struct server {
    int listenfd;
    struct net_endpoint *clients[MAX_NUM_CONNECTIONS];
    struct pollfd fds[MAX_NUM_CONNECTIONS + 1];
    unsigned num_clients;
    unsigned num_fds;
} server;

enum server_code {
    SERVER_FATAL,
    SERVER_DISCONNECT,
    SERVER_OK
};

static int init_server(struct server *serv, short port)
{
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    memset(serv, 0, sizeof(*serv));

    serv->listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv->listenfd == -1) {
        log_error("socket: %s\n", strerror(errno));
        return -1;
    }

    if (setsockopt(serv->listenfd, SOL_SOCKET, SO_REUSEADDR,
                &(int){1}, sizeof(int)) < 0) {
        log_error("setsockopt: %s\n", strerror(errno));
    }

    if (bind(serv->listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        log_error("bind: %s\n", strerror(errno));
        close(serv->listenfd);
        return -1;
    }

    if (listen(serv->listenfd, 0) == -1) {
        log_error("listen: %s\n", strerror(errno));
        close(serv->listenfd);
        return -1;
    }

    serv->fds[0].fd = serv->listenfd;
    serv->fds[0].events = POLLIN;
    serv->num_fds = 1;

    return 0;
}

static void deinit_server(struct server *serv)
{
    close(serv->listenfd);

    for (unsigned i = 0; i < serv->num_clients; ++i) {
        close(serv->clients[i]->fd);
        free(serv->clients[i]->identifier);
        net_endpoint_destroy(serv->clients[i]);
    }
}

static struct net_endpoint *accept_endpoint(struct server *serv)
{
    struct net_endpoint *endp;
    int sockfd;

    sockfd = accept(serv->listenfd, NULL, NULL);
    if (sockfd == -1) {
        log_error("accept: %s\n", strerror(errno));
        return NULL;
    }

    endp = net_endpoint_new(sockfd);
    if (!endp) {
        close(sockfd);
        log_error("Out of memory\n");
        return NULL;
    }

    return endp;
}

static int add_endpoint(struct server *serv, struct net_endpoint *endp)
{
    if (serv->num_clients == MAX_NUM_CONNECTIONS) {
        log_error("Unable to add endpoint: server is at capacity.\n");
        return -1;
    }

    serv->fds[serv->num_fds].fd = endp->fd;
    serv->fds[serv->num_fds].events = POLLIN;
    serv->num_fds++;

    serv->clients[serv->num_clients] = endp;
    serv->num_clients++;

    return 0;
}

static int remove_endpoint(struct server *serv, struct net_endpoint *endp)
{
    unsigned tail_len;

    for (int i = 0; i < serv->num_clients; ++i) {
        if (serv->clients[i] == endp) {
            /* Number of elements after the one being removed. */
            tail_len = serv->num_clients - i - 1;

            /* Shift arrays. */
            memmove(serv->clients + i, serv->clients + i + 1,
                    sizeof(endp) * tail_len);
            memmove(serv->fds + i + 1, serv->fds + i + 2,
                    sizeof(*serv->fds) * tail_len);

            serv->num_clients--;
            serv->num_fds--;
            return 0;
        }
    }

    log_debug("Cannot remove endpoint %p from server %p because it is not found\n",
            (void *)endp, (void *)serv);

    return -1;
}

static void broadcast_data(struct server *serv, const unsigned char *data, size_t len)
{
    struct net_message *msg;

    msg = net_message_new();
    if (!msg) {
        return;
    }

    if (net_message_set_body(msg, data, len) == -1) {
        net_message_unref(msg);
        return;
    }

    for (unsigned i = 0; i < serv->num_clients; ++i) {
        if (net_enqueue_message(serv->clients[i], msg) > 0) {
            serv->fds[1 + i].events |= POLLOUT;
        }
    }

    net_message_unref(msg);
}

static void disconnect_client(struct server *serv, struct net_endpoint *ept)
{
    struct chat_member_leave leave = {0};
    char buffer[1024];
    int len;

    strcpy(leave.sender, ept->identifier);

    remove_endpoint(serv, ept);
    close(ept->fd);
    free(ept->identifier);
    net_endpoint_destroy(ept);

    buffer[0] = CHAT_MEMBER_LEAVE;
    len = chat_member_leave_to_network(&leave, buffer + 1, sizeof(buffer) - 1);

    broadcast_data(serv, buffer, len + 1);

    log_info("Chat member %s disconnected.\n", leave.sender);
}

static void handle_new_chat_message(struct server *serv, 
        struct net_endpoint *sender, struct chat_message *cm)
{
    unsigned char data[1024];
    int conv;

    if (!sender->identifier) {
        /* Sender never joined. */
        return;
    }

    /* Client is not required to put anything in sender field. */
    strcpy(cm->sender, sender->identifier);

    data[0] = CHAT_MESSAGE;
    conv = chat_message_to_network(cm, data + 1, sizeof(data) - 1);
    if (conv == -1) {
        return;
    }

    broadcast_data(serv, data, conv + 1);
}

static void handle_new_chat_member_join(struct server *serv,
        struct net_endpoint *sender, struct chat_member_join *cm)
{
    unsigned char data[1024];
    int conv;

    if (sender->identifier) {
        /* Sender already joined. */
        return;
    }

    sender->identifier = strdup(cm->sender);
    if (!sender->identifier) {
        /* Fatal: no memory. */
        return;
    }

    data[0] = CHAT_MEMBER_JOIN;
    conv = chat_member_join_to_network(cm, data + 1, sizeof(data) - 1);
    if (conv == -1) {
        return;
    }

    broadcast_data(serv, data, conv + 1);
}

static int handle_endpoint_input(struct server *serv,
        struct net_endpoint *endpoint)
{
    union chat_object cm;
    struct net_message *msg;
    int rc, type;

    rc = net_receive(endpoint, &msg);
    if (rc < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return SERVER_OK;
        }

        log_error("Unable to receive data: %s\n", strerror(errno));
        return SERVER_DISCONNECT;
    }
    else if (rc == 0) {
        log_debug("Peer closed connection.\n");
        return SERVER_DISCONNECT;
    }

    type = network_to_chat_object(&cm, net_message_body(msg),
            net_message_body_length(msg));
    net_message_unref(msg);

    if (type == -1) {
        log_error("Received corrupted message.\n");
        return SERVER_DISCONNECT;
    }

    switch ((enum chat_object_type) type) {
    case CHAT_MESSAGE:
        handle_new_chat_message(serv, endpoint, &cm.chat);
        break;
    case CHAT_MEMBER_JOIN:
        handle_new_chat_member_join(serv, endpoint, &cm.join);
        break;
    case CHAT_MEMBER_LEAVE:
        log_info("Received an illegal chat object from client.\n");
        break;
    }

    return SERVER_OK;
}

int handle_incoming_connection(struct server *serv)
{
    struct net_endpoint *endpt;

    endpt = accept_endpoint(serv);
    if (!endpt) {
        return SERVER_FATAL;
    }

    if (add_endpoint(serv, endpt) == -1) {
        close(endpt->fd);
        net_endpoint_destroy(endpt);
    }

    return SERVER_OK;
}

static int loop(struct server *serv)
{
    int n, err;

    if ((n = poll(serv->fds, serv->num_fds, -1)) == -1) {
        log_error("poll: %s\n", strerror(errno));
        return -1;
    }

    if (serv->fds[0].revents & POLLIN) {
        /* incoming connection. */
        if (handle_incoming_connection(serv) == SERVER_FATAL) {
            return -1;
        }
        n--;
    }

    for (unsigned i = 1; n > 0 && i < serv->num_fds; ++i) {
        struct net_endpoint *endp;
        int revents;

        revents = serv->fds[i].revents;
        endp = serv->clients[i - 1];

        if (revents)
            n--;
        else
            continue;

        if (revents & POLLIN) {
            switch (handle_endpoint_input(serv, endp)) {
            case SERVER_DISCONNECT:
                disconnect_client(serv, endp);
                break;
            case SERVER_FATAL:
                return -1;
                break;
            case SERVER_OK:
                break;
            }
        }

        if (revents & POLLOUT) {
            int queue_len = net_process_send(endp);
            if (queue_len < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    log_error("Unable to send data: %s\n", strerror(errno));
                }
            }
            else if (queue_len == 0) {
                /* nothing more to send at this time. */
                serv->fds[i].events &= ~POLLOUT;
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (init_server(&server, PORT) == -1) {
        return 1;
    }

    for (;;) {
        if (loop(&server) == -1) {
            log_info("Exiting due to fatal error.\n");
            break;
        }
    }

    deinit_server(&server);

    return 0;
}
