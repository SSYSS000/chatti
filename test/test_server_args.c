#define main fakemain
#include "../server.c"
#undef main

#include "test.h"

int main(int argc, char *argv[])
{
    struct arguments pargs = {0};

    EXPECT_TRUE(
            0 != scan_arguments(&pargs, 1, (char*[]){ "server" }),
            "Arguments should contain port\n");
    /*
     EXPECT_TRUE(
            0 != scan_arguments(&pargs, 2, (char*[]){ "server", "asdf" }),
            "Should reject non-numeric port\n");
    */
    EXPECT_TRUE(
            0 == scan_arguments(&pargs, 2, (char*[]){ "server", "14000" }),
            "Valid arguments should be scanned successfully\n");
    EXPECT_TRUE(pargs.port == 14000,
            "Port should match the one that was given\n");

    return 0;
}
