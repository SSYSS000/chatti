#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <locale.h>

#include "ui.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    char *b;

    ui_init();

    struct pollfd fds[] = {
        {.fd = 0, .events = POLLIN}
    };

    for(;;) {
        int ret = poll(fds, 1, -1);
        if (ret > 0) {
            b = ui_get_line();
            if (b && b[0] != '\n') {
                ui_message_printf("%s", b);
            }
        }
        else {
            perror("poll");
        }
    }
    
    ui_deinit();

    return 0;
}
