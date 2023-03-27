#include "../chat.c"
#include "test.h"

#define SENDER                  "Billy"
#define MESSAGE                 "Hello, I'm Billy!"
#define EXPECTED_NET_CM         "Billy\0Hello, I'm Billy!\0"
#define EXPECTED_NET_CM_LEN     (sizeof(EXPECTED_NET_CM) - 1)

static void test_chat_message(void)
{
    unsigned char buffer[512];
    struct chat_message cm = {
        .sender  = SENDER,
        .message = MESSAGE
    };
    int rc;

    rc = chat_chat_message_to_network(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected buffer to be too small\n");

    rc = chat_chat_message_to_network(&cm, buffer, 10);
    EXPECT_TRUE(rc < 0, "Expected buffer to be too small\n");

    rc = chat_chat_message_to_network(&cm, buffer, sizeof buffer);
    EXPECT_TRUE(rc > 0, "Expected successful chat message conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CM_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CM_LEN);
    EXPECT_TRUE(!memcmp(buffer, EXPECTED_NET_CM, EXPECTED_NET_CM_LEN), "Converted data is invalid\n");
}

void test_chat_member_join(void)
{

}

void test_chat_member_leave(void)
{

}

int main(int argc, char *argv[])
{
    test_chat_message();    
    test_chat_member_join();    
    test_chat_member_leave();    
    return 0;
}
