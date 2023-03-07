#ifndef UI_H
#define UI_H

#include <stdarg.h>

/**
 * @brief Initialise the user interface. Required before using ui_* functions.
 *
 * @return 0 on success, negative value on error. 
 */
int ui_init(void);

/**
 * @brief Deinitialise the user interface.
 *
 * @note Failing to call this function before the program exits may result
 * in a broken terminal.
 */
void ui_deinit(void);

/**
 * @brief Like C library's printf, but write text to the message window.
 *
 * @param fmt Interpretation of data.
 * @param ... Arguments specifying data to print.
 *
 * @return 0 on success, negative value on error.
 */
int ui_message_printf(const char *fmt, ...);

/**
 * @brief Like ui_message_printf, but the data is in a variadic argument list.
 *
 * @param fmt Interpretation of data.
 * @param va Argument list specifying data to print.
 *
 * @return 0 on success, negative value on error.
 */
int ui_message_vprintf(const char *fmt, va_list va);

char *ui_get_line(void);

#endif /* UI_H */
