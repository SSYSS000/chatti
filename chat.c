#define _GNU_SOURCE
#include <string.h>

#include "chat.h"

static int length_of_null_terminated(const char *buf, size_t len)
{
    const char *nullpos = memchr(buf, 0, len);
    if (nullpos)
        return nullpos - buf;
    else
        return -1;
}

int chat_chat_message_to_network(const struct chat_message *msg, void *buffer, size_t len)
{
    size_t sender_len = strlen(msg->sender);
    size_t contents_len = strlen(msg->message);
    size_t serialised_len = sender_len + 1 + contents_len + 1;

    if (len < serialised_len) {
        return -1;
    }

    buffer = mempcpy(buffer, msg->sender, sender_len + 1);
    buffer = mempcpy(buffer, msg->message, contents_len + 1);

    return serialised_len;
}

int chat_network_to_chat_message(struct chat_message *msg, const void *buffer, size_t buf_len)
{
    /* Expecting two null-terminated strings in series. */
    size_t save_buf_len = buf_len;
    int slen;

    slen = length_of_null_terminated(buffer, buf_len);
    if (slen == -1 || slen > CHAT_MEMBER_NAME_MAX_LEN) {
        return -1;
    }

    memcpy(msg->sender, buffer, slen + 1);
    buffer = (const char *)buffer + slen + 1;
    buf_len -= slen + 1;

    slen = length_of_null_terminated(buffer, buf_len);
    if (slen == -1 || slen > CHAT_MESSAGE_MAX_LEN) {
        return -1;
    }

    memcpy(msg->message, buffer, slen + 1);
    buffer = (const char *)buffer + slen + 1;
    buf_len -= slen + 1;

    return save_buf_len - buf_len;
}

int chat_chat_member_join_to_network(const struct chat_member_join *msg, void *buffer, size_t len)
{
    size_t sender_len = strlen(msg->sender);
    size_t serialised_len = sender_len + 1;

    if (len < serialised_len) {
        return -1;
    }

    memcpy(buffer, msg->sender, sender_len + 1);

    return serialised_len;
}

int chat_network_to_chat_member_join(struct chat_member_join *msg, const void *buffer, size_t buf_len)
{
    /* Expecting one null-terminated string. */
    size_t save_buf_len = buf_len;
    int slen;

    slen = length_of_null_terminated(buffer, buf_len);
    if (slen == -1 || slen > CHAT_MEMBER_NAME_MAX_LEN) {
        return -1;
    }

    memcpy(msg->sender, buffer, slen + 1);
    buffer = (const char *)buffer + slen + 1;
    buf_len -= slen + 1;

    return save_buf_len - buf_len;
}

int chat_chat_member_leave_to_network(const struct chat_member_leave *msg, void *buffer, size_t len)
{
    size_t sender_len = strlen(msg->sender);
    size_t serialised_len = sender_len + 1;

    if (len < serialised_len) {
        return -1;
    }

    memcpy(buffer, msg->sender, sender_len + 1);

    return serialised_len;
}

int chat_network_to_chat_member_leave(struct chat_member_leave *msg, const void *buffer, size_t buf_len)
{
    /* Expecting one null-terminated string. */
    size_t save_buf_len = buf_len;
    int slen;

    slen = length_of_null_terminated(buffer, buf_len);
    if (slen == -1 || slen > CHAT_MEMBER_NAME_MAX_LEN) {
        return -1;
    }

    memcpy(msg->sender, buffer, slen + 1);
    buffer = (const char *)buffer + slen + 1;
    buf_len -= slen + 1;

    return save_buf_len - buf_len;
}
