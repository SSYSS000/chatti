#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <sys/socket.h>

#include "log.h"
#include "network.h"

struct net_message *net_message_new(void)
{
    struct net_message *ptr = malloc(sizeof(*ptr));

    if (ptr) {
        ptr->ref_count = 1;
    }

    return ptr;
}

struct net_message *net_message_ref(struct net_message *msg)
{
    ++msg->ref_count;
    return msg;
}

void net_message_unref(struct net_message *msg)
{
    assert(msg->ref_count > 0);

    if (--msg->ref_count == 0) {
        free(msg);
    }
}

struct net_endpoint *net_endpoint_new(int fd)
{
    struct net_endpoint *endpoint = calloc(1, sizeof(*endpoint));
    
    if (endpoint) {
        endpoint->fd = fd;
    }

    return endpoint;
}

void net_endpoint_destroy(struct net_endpoint *endpoint)
{
    for (unsigned i = 0; i < endpoint->send_queue_count; ++i) {
        net_message_unref(endpoint->send_queue[i]);
    }

    if (endpoint->receive_msg)
        net_message_unref(endpoint->receive_msg);

    free(endpoint);
}

int net_enqueue_message(struct net_endpoint *endpoint, struct net_message *msg)
{
    if (endpoint->send_queue_count == NET_ENDP_SEND_QUEUE_SIZE) {
        log_debug("Network endpoint send queue is full!\n");
        return -1;
    }

    net_message_ref(msg);
    endpoint->send_queue[endpoint->send_queue_count++] = msg;
    return endpoint->send_queue_count;
}

static unsigned net_message_length(const struct net_message *msg)
{
    unsigned len;
    len = (msg->data[0] & 0xffu) << 8 | msg->data[1] & 0xffu;
    return len;
}

unsigned net_message_body_length(const struct net_message *msg)
{
    return net_message_length(msg) - NET_MSG_HEADER_LEN;
}

unsigned char *net_message_body(struct net_message *msg)
{
    return msg->data + NET_MSG_HEADER_LEN;
}

/**
 * @brief Set the body length of the network message (length without header).
 *
 * @param msg Network message.
 * @param len Body length.
 */
static void net_message_set_body_length(struct net_message *msg, unsigned len)
{
    len += NET_MSG_HEADER_LEN;
    msg->data[0] = (len >> 8) & 0xffu;
    msg->data[1] = len & 0xffu;
}

int net_message_set_body(struct net_message *msg, const void *body, unsigned len)
{
    if (NET_MSG_HEADER_LEN + len > NET_MSG_DATA_SIZE) {
        return -1;
    }

    net_message_set_body_length(msg, len);
    memcpy(net_message_body(msg), body, len);
    return 0;
}

/**
 * @brief Resume or begin sending a network message.
 *
 * @param endp Endpoint.
 * @param msg Partially sent network message.
 * @param num_sent Pointer to the number of bytes sent so far.
 *                 Value shall be updated before returning.
 *
 * @return Positive integer indicating fully sent message,
 *         -1 indicating failure (check errno).
 */
static int net_process_one_send(
        struct net_endpoint *endp,
        struct net_message *msg,
        unsigned *num_sent)
{
    unsigned msg_len;
    ssize_t n;

    msg_len = net_message_length(msg);

    while (*num_sent < msg_len) {
        n = send(endp->fd, msg->data + *num_sent, msg_len - *num_sent, 0);
        if (n < 0) {
            break;
        }

        *num_sent += n;
    }

    return n;
}

int net_process_send(struct net_endpoint *endp)
{
    struct net_message *msg;
    unsigned n_processed;
    int err = 0;

    for (n_processed = 0; n_processed < endp->send_queue_count; ++n_processed) {
        msg = endp->send_queue[n_processed];
        if (net_process_one_send(endp, msg, &endp->num_bytes_sent) > 0) {
            /* Message sent fully. */
            net_message_unref(msg);
            endp->num_bytes_sent = 0;
        }
        else {
            err = errno;
            break;
        }
    }

    /* Shift the queue. */
    endp->send_queue_count -= n_processed;
    if (endp->send_queue_count > 0 && n_processed > 0) {
        memmove(endp->send_queue, endp->send_queue + n_processed,
                endp->send_queue_count * sizeof(*endp->send_queue));
    }

    if (err) {
        errno = err;
        return -1;
    }

    return endp->send_queue_count;
}

int net_receive(struct net_endpoint *endp, struct net_message **msg)
{
    unsigned needed;
    ssize_t n;

    /* Get receive buffer. */
    if (!endp->receive_msg) {
        endp->receive_msg = net_message_new();
        if (!endp->receive_msg) {
            return -ENOMEM;
        }
    }

    if (endp->num_bytes_received < NET_MSG_HEADER_LEN) {
        /* Need to receive header first. */
        needed = NET_MSG_HEADER_LEN;
    }
    else {
        needed = net_message_length(endp->receive_msg);
    }

    while (endp->num_bytes_received < needed) {
        n = recv(endp->fd, endp->receive_msg->data + endp->num_bytes_received,
                needed - endp->num_bytes_received, 0);
        if (n <= 0) {
            return -1;
        }

        endp->num_bytes_received += n;
        if (endp->num_bytes_received == NET_MSG_HEADER_LEN) {
            needed = net_message_length(endp->receive_msg);
        }
    }

    /* Message received fully. */
    *msg = endp->receive_msg;
    endp->receive_msg = NULL;
    endp->num_bytes_received = 0;

    return needed;
}
