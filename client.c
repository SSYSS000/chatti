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

#include "ui.h"
#include "log.h"
#include "network.h"
#include "chat.h"

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

static char username[CHAT_MEMBER_NAME_MAX_LEN + 1];

static struct net_endpoint *connect_server(const char *addr, const char *port)
{
    struct net_endpoint *server;
    struct addrinfo *res, *addrinfo;
    struct addrinfo hints = {0};
    int save_errno;
    int sockfd;
    int err = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(addr, port, &hints, &res);
    if (err != 0) {
        eprintf("getaddrinfo: %s\n", gai_strerror(err));
        return NULL;
    }

    for (addrinfo = res; addrinfo != NULL; addrinfo = res->ai_next) {
        sockfd = err = socket(addrinfo->ai_family, addrinfo->ai_socktype,
                addrinfo->ai_protocol);
        if (err == -1) {
            continue;
        }

        err = connect(sockfd, addrinfo->ai_addr, addrinfo->ai_addrlen);
        if (err == -1) {
            save_errno = errno;
            close(sockfd);
            errno = save_errno;
            continue;
        }

        break;
    }

    freeaddrinfo(res);

    if (err == -1) {
        perror("Unable to connect to server");
        return NULL;
    }

    server = net_endpoint_new(sockfd);
    if (!server) {
        close(sockfd);
        eprintf("Out of memory\n");
    }

    return server;
}

static void disconnect_server(struct net_endpoint *server)
{
    close(server->fd);
    net_endpoint_destroy(server);
}

static int send_chat_message(struct net_endpoint *server, const struct chat_message *chat_msg)
{
    struct net_message *net_msg;
    char buffer[1024];
    int n, ret = 0;

    buffer[0] = CHAT_MESSAGE;
    n = chat_message_to_network(chat_msg, buffer + 1, sizeof(buffer) - 1);
    if (n == -1) {
        log_debug("Chat message serialisation buffer too small (%zu).\n",
                sizeof(buffer)); 
        return -1;
    }

    net_msg = net_message_new();
    if (!net_msg) {
        return -1;
    }

    if (net_message_set_body(net_msg, buffer, 1 + n) == -1) {
        log_debug("Network message body is too long!\n");
        net_message_unref(net_msg);
        return -1;
    }

    if (net_enqueue_message(server, net_msg) == -1) {
        /* Queue full */
        ret = -1;
    }

    net_message_unref(net_msg);

    return ret;
}

static int handle_user_input(struct net_endpoint *server)
{
    struct chat_message chat_msg;
    char *line;
    int err;

    line = ui_get_line();
    if (!line) {
        /* Line not finished yet. */
        return 0;
    }
    
    if (line[0] == '\n') {
        log_debug("User entered empty message\n");
        return 0;
    }

    log_debug("User entered message: %s", line);

    line[strlen(line) - 1] = '\0'; /* Remove '\n'. */

    /* Create a new chat message. */
    strcpy(chat_msg.sender, username);
    strncpy(chat_msg.message, line, CHAT_MESSAGE_MAX_LEN);
    chat_msg.message[CHAT_MESSAGE_MAX_LEN + 1] = '\0';


    if (send_chat_message(server, &chat_msg) != 0) {
        log_error("Failed to send chat message.\n");
    }

    return 0;
}

static void handle_new_chat_message(const struct chat_message *cm)
{
    ui_message_printf("%s: %s\n", cm->sender, cm->message); 
}

static void handle_new_chat_member_join(const struct chat_member_join *cm)
{
    ui_message_printf("%s joined. Say hi!\n", cm->sender);
}

static void handle_new_chat_member_leave(const struct chat_member_leave *cm)
{
    ui_message_printf("%s left.\n", cm->sender);
}

static int handle_server_input(struct net_endpoint *server)
{
    struct net_message *msg;
    union chat_object cm;
    int process_rc, type;

    process_rc = net_process_receive(server);
    if (process_rc < 0) {
        log_error("Unable to receive data: %s\n", strerror(-process_rc));
        return -1;
    }

    msg = net_receive(server);
    if (msg) {
        type = network_to_chat_object(&cm, net_message_body(msg),
                net_message_body_length(msg));
        net_message_unref(msg);
        msg = NULL;
    
        if (type == -1) {
            log_error("Received corrupted message.\n");
            return -1;
        }

        switch ((enum chat_object_type) type) {
        case CHAT_MESSAGE:
            handle_new_chat_message(&cm.chat);
            break;
        case CHAT_MEMBER_JOIN:
            handle_new_chat_member_join(&cm.join);
            break;
        case CHAT_MEMBER_LEAVE:
            handle_new_chat_member_leave(&cm.leave);
            break;
        }
    }

    if (process_rc == 0) {
        log_info("Server disconnected.\n");
        return -1;
    }

    return 0;
}

int main_loop(struct net_endpoint *server)
{
    struct pollfd fds[] = {
        { .fd = 0,          .events = POLLIN },
        { .fd = server->fd, .events = POLLIN },
    };
    struct pollfd *stdinpoll = &fds[0];
    struct pollfd *serverpoll = &fds[1];
    int pollret, err;

    for(;;) {
        pollret = poll(fds, 2, -1);
        if (pollret == -1) {
            log_error("poll: %s", strerror(errno));
            continue;
        }

        if (stdinpoll->revents & POLLIN) {
            err = handle_user_input(server);
            if (err) {
                return err;
            }
        }

        if (serverpoll->revents & POLLIN) {
            err = handle_server_input(server);
            if (err) {
                return err;
            }
        }

        if (serverpoll->revents & POLLHUP) {
            log_info("Server disconnected.\n");
            return 0;
        }

        if (server->send_queue_count > 0) {
            serverpoll->events |= POLLOUT;
        }

        if (serverpoll->revents & POLLOUT) {
            int queue_len = net_process_send(server);
            if (queue_len < 0) {
                log_error("Unable to send to server: %s\n", strerror(-queue_len));
            }
            else if (queue_len == 0) {
                /* nothing more to send at this time. */
                serverpoll->events &= ~POLLOUT;
            }
        }
    }

    return 0;
}

#define server_addr "127.0.0.1"
#define server_port "14023"

int main(int argc, char *argv[])
{
    struct net_endpoint *server;

    setlocale(LC_ALL, "");

    eprintf("Connecting to %s:%s ...\n", server_addr, server_port);
    server = connect_server(server_addr, server_port);
    if (!server) {
        return EXIT_FAILURE;
    }

    ui_init();

    log_info("You are now connected. Press CTRL+C to disconnect.\n");

    main_loop(server);
    sleep(1);
    ui_deinit();
    disconnect_server(server);

    return 0;
}
