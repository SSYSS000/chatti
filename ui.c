#include <assert.h>
#include <ncurses.h>
#include <stdarg.h>
#include "ui.h"

#define PADDING                         1

#define TEXT_INPUT_NLINES               2
#define TEXT_INPUT_B_NLINES             (TEXT_INPUT_NLINES + PADDING * 2)

#define MESSAGE_B_NLINES(term_y)        ((term_y) - TEXT_INPUT_B_NLINES)
#define MESSAGE_NLINES(term_y)          (MESSAGE_B_NLINES(term_y) - PADDING * 2)

#define MESSAGE_B_NCOLS(term_x)         (term_x)
#define MESSAGE_NCOLS(term_x)           (MESSAGE_B_NCOLS(term_x) - PADDING * 2)

#define TEXT_INPUT_B_NCOLS(term_x)      (term_x)
#define TEXT_INPUT_NCOLS(term_x)        (TEXT_INPUT_B_NCOLS(term_x) - PADDING * 2)

static WINDOW *message_window, *message_window_b;
static WINDOW *text_input_window, *text_input_window_b;

int ui_init(void)
{
    /* TODO: handle errors. */
    int term_x, term_y;

    assert(message_window == NULL);

    initscr();

    getmaxyx(stdscr, term_y, term_x);

    /* Initialise message window. */
    message_window_b = newwin(
            MESSAGE_B_NLINES(term_y),
            MESSAGE_B_NCOLS(term_x), 0, 0);
    assert(message_window_b);
    box(message_window_b, 0, 0);

    message_window = derwin(message_window_b,
            MESSAGE_NLINES(term_y),
            MESSAGE_NCOLS(term_x), PADDING, PADDING);
    assert(message_window);
    scrollok(message_window, TRUE);

    /* Initialise text input window. */
    text_input_window_b = newwin(
            TEXT_INPUT_B_NLINES,
            TEXT_INPUT_B_NCOLS(term_x),
            MESSAGE_B_NLINES(term_y), 0);
    assert(text_input_window_b);
    box(text_input_window_b, 0, 0);
    text_input_window = derwin(text_input_window_b,
            TEXT_INPUT_NLINES,
            TEXT_INPUT_NCOLS(term_x), PADDING, PADDING);
    assert(text_input_window);

    wrefresh(message_window_b);
    wrefresh(text_input_window_b);
    
    touchwin(message_window_b);
    wrefresh(message_window);

    touchwin(text_input_window_b);
    wrefresh(text_input_window);

    return 0;
}

void ui_deinit(void)
{
    delwin(message_window);
    delwin(text_input_window);
    delwin(message_window_b);
    delwin(text_input_window_b);
    endwin();
}

int ui_message_printf(const char *fmt, ...)
{
    va_list va;
    int ret;

    va_start(va, fmt);
    ret = ui_message_vprintf(fmt, va);
    va_end(va);

    return ret;
}

int ui_message_vprintf(const char *fmt, va_list va)
{
    int ret;

    assert(message_window != NULL);
    
    ret = vw_printw(message_window, fmt, va);
    touchwin(message_window_b);
    wrefresh(message_window);
    
    return ret == OK ? 0 : -1;
}

