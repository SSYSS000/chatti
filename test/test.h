#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>

#define EXPECT_TRUE(cond, fmt, ...) do {                                    \
    if (!(cond)) {                                                          \
        fprintf(stderr, "Expect failed on line %d of file %s\n"             \
                        "Expected true: " #cond "\nMessage:\n\t",           \
                __LINE__, __FILE__);                                        \
        fprintf(stderr, fmt, ##__VA_ARGS__);                                \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
} while (0)

#endif /* TEST_H */
