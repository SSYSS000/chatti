#ifndef CHAT_H
#define CHAT_H

#define CHAT_MEMBER_NAME_MAX_LEN        36
#define CHAT_MESSAGE_MAX_LEN            512

struct net_message;

enum message_type {
    MSG_CHAT_MESSAGE,
    MSG_CHAT_MEMBER_JOIN,
    MSG_CHAT_MEMBER_LEAVE
};

struct chat_message {
    char sender[CHAT_MEMBER_NAME_MAX_LEN + 1];
    char message[CHAT_MESSAGE_MAX_LEN + 1];
};

struct chat_member_join {
    char sender[CHAT_MEMBER_NAME_MAX_LEN + 1];
};

struct chat_member_leave {
    char sender[CHAT_MEMBER_NAME_MAX_LEN + 1];
};

union chat_any_message {
    struct chat_message chat;
    struct chat_member_join join;
    struct chat_member_leave leave;
};

/*
 * These functions convert network formatted data to messages or vice versa.
 * On success, they return the number of bytes read or written to the buffer.
 * On failure, they return a negative integer.
 */
int chat_chat_message_to_network(const struct chat_message *msg, void *buffer, size_t len);
int chat_network_to_chat_message(struct chat_message *msg, const void *buffer, size_t len);

int chat_chat_member_join_to_network(const struct chat_member_join *msg, void *buffer, size_t len);
int chat_network_to_chat_member_join(struct chat_member_join *msg, const void *buffer, size_t len);

int chat_chat_member_leave_to_network(const struct chat_member_leave *msg, void *buffer, size_t len);
int chat_network_to_chat_member_leave(struct chat_member_leave *msg, const void *buffer, size_t len);

/**
 * @brief Convert net message to a chat message object.
 *
 * @param cm Any chat message object.
 * @param net_msg Network message.
 *
 * @return enum message_type or -1 on conversion error.
 */
int net_message_to_chat_object(union chat_any_message *cm, struct net_message *net_msg);

#endif /* CHAT_H */
