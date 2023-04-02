#ifndef NETWORK_H
#define NETWORK_H

#define NET_MSG_DATA_SIZE                       2048u
#define NET_MSG_LEN_DATA_SIZE                   2u
#define NET_MSG_HEADER_LEN                      NET_MSG_LEN_DATA_SIZE
#define NET_ENDP_SEND_QUEUE_SIZE                16u

struct net_message {
    unsigned ref_count;
    unsigned char data[NET_MSG_DATA_SIZE];
};

struct net_endpoint {
    void *identifier;
    int fd;
    unsigned num_bytes_sent;
    unsigned num_bytes_received;
    unsigned send_queue_count;
    struct net_message *receive_msg;
    struct net_message *send_queue[NET_ENDP_SEND_QUEUE_SIZE];
};

struct net_endpoint *net_endpoint_new(int fd);

void net_endpoint_destroy(struct net_endpoint *endpoint);

/**
 * @brief Allocate memory for a network message buffer.
 *
 * @return Network message of ref count 1 or NULL on memory allocation error.
 */
struct net_message *net_message_new(void);

/**
 * @brief Increment the reference count of a network message.
 *
 * @param msg Network message.
 *
 * @return msg
 */
struct net_message *net_message_ref(struct net_message *msg);

/**
 * @brief Decrement the reference count of a network message,
 *        possibly freeing it.
 *
 * @note If the reference count reaches 0, further usage of msg is not allowed.
 *
 * @param msg Network message.
 */
void net_message_unref(struct net_message *msg);

/**
 * @brief Set a body to the network message by copying it from a buffer.
 *
 * @param net_message Network message.
 * @param body Network message body.
 * @param len Length of body.
 *
 * @return On success, zero. If body is too long, return -1. 
 */
int net_message_set_body(struct net_message *msg, const void *body, unsigned len);

/**
 * @brief Get a pointer to the message body.
 *
 * @param msg Network message.
 *
 * @return Pointer to message body.
 */
unsigned char *net_message_body(struct net_message *msg);

/**
 * @brief Get the length of the message body.
 *
 * @param msg Network message.
 *
 * @return Message body length.
 */
unsigned net_message_body_length(const struct net_message *msg);

/**
 * @brief Enqueue a network message to be sent to an endpoint.
 *
 * @param endpoint Endpoint.
 * @param msg Valid positive length network message.
 *
 * @return On success, the new queue length. On error, -1.
 */
int net_enqueue_message(struct net_endpoint *endpoint, struct net_message *msg);

/**
 * @brief Send as much queued data as possible to an endpoint.
 *
 * @param endpoint Endpoint.
 *
 * @return Send queue length or -1 if an error occurred (check errno).
 */
int net_process_send(struct net_endpoint *endpoint);

/**
 * @brief Resume or begin receiving a network message.
 *
 * @description Resume or begin receiving a network message. When the message
 * is complete, it is stored in msg. If an error occurs, errno is set and the
 * function returns -1. If the peer has shutdown, return 0.
 *
 * @param endpoint Endpoint.
 * @param msg Pointer to storage for the received message.
 *
 * @return Positive network message length of *msg on success,
 *         zero indicating peer has shutdown,
 *         -1 indicating failure (check errno).
 */
int net_receive(struct net_endpoint *endpoint, struct net_message **msg);

#endif /* NETWORK_H */
