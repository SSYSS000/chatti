#define main fakemain
#include "../client.c"
#undef main

#include "test.h"

int main(int argc, char *argv[])
{
    struct prog_args pargs = {0};

    EXPECT_TRUE(
            0 != scan_arguments(&pargs, 1, (char*[]){ "client" }),
            "Arguments should contain server, port and username\n");
    EXPECT_TRUE(
            0 != scan_arguments(&pargs, 2, (char*[]){ "client", "127.0.0.1" }),
            "Arguments should contain server, port and username\n");
    EXPECT_TRUE(
            0 != scan_arguments(&pargs, 3, (char*[]){ "client", "127.0.0.1", "14000" }),
            "Arguments should contain server, port and username\n");
    EXPECT_TRUE(
            0 == scan_arguments(&pargs, 4, (char*[]){ "client", "127.0.0.1", "14000", "Joe" }),
            "Valid arguments should be scanned successfully\n");

    EXPECT_TRUE(!strcmp(pargs.addr, "127.0.0.1"),
        "Server should match the one that was given\n");
    EXPECT_TRUE(!strcmp(pargs.port, "14000"),
        "Port should match the one that was given\n");
    EXPECT_TRUE(!strcmp(pargs.username, "Joe"),
        "Username should match the one that was given\n");

    return 0;
}
