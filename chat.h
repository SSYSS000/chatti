#ifndef CHAT_H
#define CHAT_H

#define CHAT_MEMBER_NAME_MAX_LEN        36
#define CHAT_MESSAGE_MAX_LEN            512

enum chat_object_type {
    CHAT_MESSAGE,
    CHAT_MEMBER_JOIN,
    CHAT_MEMBER_LEAVE
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

union chat_object {
    struct chat_message chat;
    struct chat_member_join join;
    struct chat_member_leave leave;
};

/*
 * These functions convert network formatted data to messages or vice versa.
 * On success, they return the number of bytes read or written to the buffer.
 * On failure, they return a negative integer.
 */
int chat_message_to_network(const struct chat_message *msg, unsigned char *buffer, size_t len);
int network_to_chat_message(struct chat_message *msg, const unsigned char *buffer, size_t len);

int chat_member_join_to_network(const struct chat_member_join *msg, unsigned char *buffer, size_t len);
int network_to_chat_member_join(struct chat_member_join *msg, const unsigned char *buffer, size_t len);

int chat_member_leave_to_network(const struct chat_member_leave *msg, unsigned char *buffer, size_t len);
int network_to_chat_member_leave(struct chat_member_leave *msg, const unsigned char *buffer, size_t len);

/**
 * @brief Convert to a chat message object from network format.
 *
 * @param cm Any chat message object.
 * @param data Chat object network format data.
 * @param len Length of data.
 *
 * @return Chat object type or -1 on conversion error.
 */
int network_to_chat_object(union chat_object *cm, const unsigned char *data, size_t len);

#endif /* CHAT_H */
