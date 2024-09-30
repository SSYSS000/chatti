#define NCURSES_WIDECHAR 1
#include <assert.h>
#include <stdlib.h>
#include <wchar.h>
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

    noecho();

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
enum { INPUT_BUF_SIZE = 256 };
static wchar_t input_buf[INPUT_BUF_SIZE];
static int input_len;

static char *mb_from_wcstring(const wchar_t *wcstring)
{
    char *mb_string;
    size_t mb_size;

    /* Find the length of the multibyte string. */
    mb_size = wcstombs(NULL, wcstring, 0);
    if (mb_size == (size_t)-1) {
        return NULL;
    }
    mb_size += 1; /* Include null terminator. */

    mb_string = malloc(mb_size);
    if (mb_string == NULL) {
        return NULL;
    }

    if (wcstombs(mb_string, wcstring, mb_size) == (size_t)-1) {
        free(mb_string);
        return NULL;
    }

    return mb_string;
}

char *ui_get_line(void)
{
    wint_t c = 0;
    int status;

    while (c != L'\n') {
        status = wget_wch(text_input_window, &c);
        if (status == ERR) {
            /* Stop waiting for input. */
            return NULL;
        }

        int is_backspace =
            status == KEY_CODE_YES &&
            (c == KEY_BACKSPACE || c == KEY_DC) ||
            c == 127; /* For macOS. */

        if (is_backspace) {
            /* Remove last input character.  */
            if (input_len > 0) {
                input_len--;
            }
        }
        else if (status == KEY_CODE_YES) {}
        else if (c == L'\n') {
            /* Enter received; line is complete. */
            input_buf[input_len] = L'\0'; 
            input_len = 0; /* Reset length for the next input line. */
        }
        else if (input_len + 1 < INPUT_BUF_SIZE) {
            /* Add character to the input buffer. */
            input_buf[input_len++] = c;
        }
        else {
            /* Buffer full. */
            beep();
        }

        /* Sync text input window. */
        werase(text_input_window);
        waddnwstr(text_input_window, input_buf, input_len);
        touchwin(text_input_window_b);
        wrefresh(text_input_window);
    }

    /* Convert to multibyte string and return it. */
    return mb_from_wcstring(input_buf);
}

