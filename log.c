#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"

#ifdef BUILD_TARGET_CLIENT
# include "ui.h"

# define DEFAULT_COLOR          UI_FG_DEFAULT
# define ERROR_COLOR            UI_FG_RED
# define INFO_COLOR             UI_FG_BLUE
# define DEBUG_COLOR            UI_FG_MAGENTA

#elif defined(BUILD_TARGET_SERVER)
# define DEFAULT_COLOR          0
# define ERROR_COLOR            0
# define INFO_COLOR             0
# define DEBUG_COLOR            0
#endif

static void set_print_color(int color)
{
#ifdef BUILD_TARGET_CLIENT
    ui_message_fg(color);
#elif defined(BUILD_TARGET_SERVER)
#endif
}

/**
 * @brief Allocate enough memory and call vsnprintf.
 *
 * @param fmt Format.
 * @param va Data.
 *
 * @return Output of vsnprintf or NULL on memory allocation error or on
 *         output error.
 */
static char *dynamic_vsnprintf(const char *fmt, va_list va)
{
    char *buf = NULL, *temp;
    int buf_size = 8192;
    int n_attempts = 2;
    int n_written;
    bool success = false;

    while (!success && n_attempts-- > 0) {
        temp = realloc(buf, buf_size);
        if (!temp) {
            break;
        }
        
        buf = temp;

        n_written = vsnprintf(buf, buf_size, fmt, va);
        if (n_written >= buf_size) {
            /* Truncated. */
            buf_size = n_written + 1;
        }
        else if (n_written < 0) {
            /* Output error */
            break;
        }
        else {
            success = true;
        }
    }

    if (!success) {
        free(buf);
        buf = NULL;
    }
    else if (buf_size > n_written) {
        /* Shrink to fit */
        temp = realloc(buf, n_written);
        if (temp) {
            buf = temp;
        }
    }

    return buf;
}

/**
 * @brief Log any type of message to the correct location (UI/stderr).
 *
 * @param category Message category.
 * @param fmt Message format.
 * @param va Message data.
 *
 * @return 0 on success, negative value on error.
 */
static int log_vmsg(const char *category, const char *fmt, va_list va)
{
    char *buffer;
    int n_written;
    int err, ret = 0;

    buffer = dynamic_vsnprintf(fmt, va);
    if (!buffer) {
        return -1;
    }

#if defined(BUILD_TARGET_CLIENT)
    err = ui_message_printf("[%s] %s", category, buffer);
    if (err != 0) {
        ret = -1;
    }
#elif defined(BUILD_TARGET_SERVER)
    n_written = fprintf(stderr, "[%s] %s", category, buffer);
    if (n_written < 0) {
        ret = -1;
    }
#else
# error "Unknown build target"
#endif

    free(buffer);

    return ret;
}

int log_error(const char *fmt, ...)
{
    va_list va;
    int ret;
    va_start(va, fmt);
    set_print_color(ERROR_COLOR);
    ret = log_vmsg("error", fmt, va);
    set_print_color(DEFAULT_COLOR);
    va_end(va);
    return ret;
}

int log_info(const char *fmt, ...)
{
    va_list va;
    int ret;
    va_start(va, fmt);
    set_print_color(INFO_COLOR);
    ret = log_vmsg("info", fmt, va);
    set_print_color(DEFAULT_COLOR);
    va_end(va);
    return ret;
}

#ifdef DEBUG

void log_debug(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    set_print_color(DEBUG_COLOR);
    log_vmsg("debug", fmt, va);
    set_print_color(DEFAULT_COLOR);
    va_end(va);
}

#endif /* DEBUG */
