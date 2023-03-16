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

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

static struct net_endpoint *connect_server(const char *addr, const char *port)
{
    struct net_endpoint *server;
    struct addrinfo *res, *addrinfo;
    int save_errno;
    int sockfd;
    int err = -1;

    err = getaddrinfo(addr, port, NULL, &res);
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
        perror("Unable to connect");
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

static int handle_user_input(struct net_endpoint *server)
{
    char *line; 
    line = ui_get_line();

    if (line && line[0] != '\n') {
        ui_message_printf("%s", line);
    }
}

static int handle_server_input(struct net_endpoint *server)
{

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
            err = handle_user_input();
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

        if (serverpoll->revents & POLLOUT) {

        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct net_endpoint *server;

    setlocale(LC_ALL, "");

    server = connect_server("127.0.0.1", "14023");
    if (!server) {
        return EXIT_FAILURE;
    }

    ui_init();
    main_loop(server);
    ui_deinit();
    disconnect_server(server);

    return 0;
}
