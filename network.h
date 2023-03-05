#ifndef NETWORK_H
#define NETWORK_H

#define NET_MSG_DATA_SIZE                       2048u
#define NET_MSG_LEN_DATA_SIZE                   2u
#define NET_MSG_HEADER_LEN                      NET_MSG_LEN_DATA_SIZE
#define NET_ENDP_SEND_QUEUE_SIZE                8u
#define NET_ENDP_RECEIVE_QUEUE_SIZE             8u

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
    unsigned receive_queue_count;
    struct net_message *receive_msg;
    struct net_message *send_queue[NET_ENDP_SEND_QUEUE_SIZE];
    struct net_message *receive_queue[NET_ENDP_RECEIVE_QUEUE_SIZE];
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
 * @brief Set the body length of the network message (length without header).
 *
 * @param msg Network message.
 * @param len Body length.
 */
void net_message_set_body_length(struct net_message *msg, unsigned len);

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
 * @brief Send as much queued data as possible to an endpoint without blocking.
 *
 * @param endpoint Endpoint.
 *
 * @return Send queue length or a negative errno if an error occurred.
 */
int net_process_send(struct net_endpoint *endpoint);

/**
 * @brief Receive as much message data as possible to a queue without blocking.
 *
 * @param endpoint Endpoint.
 *
 * @return Positive integer indicating success,
 *         zero indicating endpoint has shutdown,
 *         a negative errno value indicating failure.
 */
int net_process_receive(struct net_endpoint *endpoint);

/**
 * @brief Get a network message from the receive queue.
 *
 * @param endpoint Endpoint.
 *
 * @return Network message or NULL if queue is empty.
 */
struct net_message *net_receive(struct net_endpoint *endpoint);

#endif /* NETWORK_H */
