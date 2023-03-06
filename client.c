#include <unistd.h>

#include "ui.h"

int main(int argc, char *argv[])
{
    ui_init();

    for(int i = 0;; i++) {
        ui_message_printf("HELLO %d\n", i);
        usleep(60000);
    }
    
    ui_deinit();

    return 0;
}
