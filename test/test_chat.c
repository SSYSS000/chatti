#include "../chat.c"
#include "test.h"

#define SENDER                  "Billy"
#define MESSAGE                 "Hello, I'm Billy!"

#define EXPECTED_NET_CM         "Billy\0Hello, I'm Billy!\0"
#define EXPECTED_NET_CM_LEN     (sizeof(EXPECTED_NET_CM) - 1)

#define EXPECTED_NET_CMJ         "Billy\0"
#define EXPECTED_NET_CMJ_LEN     (sizeof(EXPECTED_NET_CMJ) - 1)

#define EXPECTED_NET_CML         "Billy\0"
#define EXPECTED_NET_CML_LEN     (sizeof(EXPECTED_NET_CMJ) - 1)

static void test_chat_message(void)
{
    unsigned char buffer[512];
    struct chat_message cm = {
        .sender  = SENDER,
        .message = MESSAGE
    };
    int rc;

    // To network
    rc = chat_chat_message_to_network(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected buffer to be too small\n");

    rc = chat_chat_message_to_network(&cm, buffer, 10);
    EXPECT_TRUE(rc < 0, "Expected buffer to be too small\n");

    rc = chat_chat_message_to_network(&cm, buffer, sizeof buffer);
    EXPECT_TRUE(rc > 0, "Expected successful conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CM_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CM_LEN);
    EXPECT_TRUE(!memcmp(buffer, EXPECTED_NET_CM, EXPECTED_NET_CM_LEN), "Converted data is invalid\n");

    // From network
    rc = chat_network_to_chat_message(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected error for corrupt message\n");

    rc = chat_network_to_chat_message(&cm, buffer, 10);
    EXPECT_TRUE(rc < 0, "Expected error for corrupt message\n");

    rc = chat_network_to_chat_message(&cm, buffer, EXPECTED_NET_CM_LEN);
    EXPECT_TRUE(rc > 0, "Expected successful conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CM_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CM_LEN);
    EXPECT_TRUE(!strcmp(cm.sender, SENDER), "Converted data is invalid\n");
    EXPECT_TRUE(!strcmp(cm.message, MESSAGE), "Converted data is invalid\n");
}

void test_chat_member_join(void)
{
    unsigned char buffer[512];
    struct chat_member_join cm = {
        .sender  = SENDER,
    };
    int rc;

    // To network
    rc = chat_chat_member_join_to_network(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected buffer to be too small\n");

    rc = chat_chat_member_join_to_network(&cm, buffer, sizeof buffer);
    EXPECT_TRUE(rc > 0, "Expected successful conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CMJ_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CMJ_LEN);
    EXPECT_TRUE(!memcmp(buffer, EXPECTED_NET_CMJ, EXPECTED_NET_CMJ_LEN), "Converted data is invalid\n");

    // From network
    rc = chat_network_to_chat_member_join(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected error for corrupt message\n");

    rc = chat_network_to_chat_member_join(&cm, buffer, EXPECTED_NET_CMJ_LEN);
    EXPECT_TRUE(rc > 0, "Expected successful conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CMJ_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CMJ_LEN);
    EXPECT_TRUE(!strcmp(cm.sender, SENDER), "Converted data is invalid\n");
}

void test_chat_member_leave(void)
{
    unsigned char buffer[512];
    struct chat_member_leave cm = {
        .sender  = SENDER,
    };
    int rc;

    // To network
    rc = chat_chat_member_leave_to_network(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected buffer to be too small\n");

    rc = chat_chat_member_leave_to_network(&cm, buffer, sizeof buffer);
    EXPECT_TRUE(rc > 0, "Expected successful conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CML_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CML_LEN);
    EXPECT_TRUE(!memcmp(buffer, EXPECTED_NET_CML, EXPECTED_NET_CML_LEN), "Converted data is invalid\n");

    // From network
    rc = chat_network_to_chat_member_leave(&cm, buffer, 1);
    EXPECT_TRUE(rc < 0, "Expected error for corrupt message\n");

    rc = chat_network_to_chat_member_leave(&cm, buffer, EXPECTED_NET_CML_LEN);
    EXPECT_TRUE(rc > 0, "Expected successful conversion\n");
    EXPECT_TRUE(rc == EXPECTED_NET_CML_LEN, "Incorrect converted length (%d), expected %d\n", rc, EXPECTED_NET_CML_LEN);
    EXPECT_TRUE(!strcmp(cm.sender, SENDER), "Converted data is invalid\n");
}

int main(int argc, char *argv[])
{
    test_chat_message();    
    test_chat_member_join();    
    test_chat_member_leave();    
    return 0;
}
