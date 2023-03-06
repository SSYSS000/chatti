#ifndef UI_H
#define UI_H

#include <stdarg.h>

int ui_init(void);
void ui_deinit(void);

int ui_message_printf(const char *fmt, ...);
int ui_message_vprintf(const char *fmt, va_list va);

#endif /* UI_H */
