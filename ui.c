#include <assert.h>
#include <ncurses.h>
#include <stdarg.h>
#include "ui.h"

#define UI_BG_DEFAULT_COLOR             COLOR_BLACK

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

static void ui_init_colors(void)
{
    if (has_colors() == FALSE) {
        return;
    }

    start_color();

    init_pair(UI_FG_DEFAULT, COLOR_WHITE, UI_BG_DEFAULT_COLOR);
    init_pair(UI_FG_RED, COLOR_RED, UI_BG_DEFAULT_COLOR);
    init_pair(UI_FG_BLUE, COLOR_BLUE, UI_BG_DEFAULT_COLOR);
    init_pair(UI_FG_YELLOW, COLOR_YELLOW, UI_BG_DEFAULT_COLOR);
    init_pair(UI_FG_MAGENTA, COLOR_MAGENTA, UI_BG_DEFAULT_COLOR);
    init_pair(UI_FG_CYAN, COLOR_CYAN, UI_BG_DEFAULT_COLOR);

    attron(COLOR_PAIR(UI_FG_RED));
}

int ui_init(void)
{
    /* TODO: handle errors. */
    int term_x, term_y;

    assert(message_window == NULL);

    initscr();
    ui_init_colors();

    echo();

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

    scrollok(text_input_window, TRUE);
    nodelay(text_input_window, 1);

    wnoutrefresh(message_window_b);
    wnoutrefresh(text_input_window_b);
    
    touchwin(message_window_b);
    wnoutrefresh(message_window);

    touchwin(text_input_window_b);
    wnoutrefresh(text_input_window);

    doupdate();
    
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

void ui_message_fg(int fgcolor)
{
    wattron(message_window, COLOR_PAIR(fgcolor));
}

/* Output functions */

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

/* Input functions */
static char input_buf[1024];
static int input_len;

static void clear_input_window(void)
{
    werase(text_input_window);
}

static void handle_backspace(int c)
{
    if (input_len > 0) {
        input_len--;

        /* Delete last character. */
        mvwdelch(text_input_window, 0, input_len);
    }
    else {
        wmove(text_input_window, 0, 0);
    }

    /* Delete echoed DEL character. */
    wdelch(text_input_window);
    wdelch(text_input_window);
}

char *ui_get_line(void)
{
    char *ret = NULL;
    int c;

    while(!ret && (c = wgetch(text_input_window)) != ERR) {
        if (c == KEY_BACKSPACE || c == KEY_DC || c == 127) {
            handle_backspace(c);
        }
        else {
            input_buf[input_len++] = (unsigned char) c;
        }

        if (c == '\n') {
            input_buf[input_len] = 0;
            input_len = 0;
            clear_input_window();
            ret = input_buf;
        }

        touchwin(text_input_window_b);
        wrefresh(text_input_window);
    }

    return ret;
}

