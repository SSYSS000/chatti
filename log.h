#ifndef LOG_H
#define LOG_H

/**
 * @brief Log a formatted informational message.
 *
 * @param fmt Format of the message.
 * @param ... Message data.
 *
 * @return 0 on success, negative value on error.
 */
int log_info(const char *fmt, ...);

/**
 * @brief Log a formatted error message.
 *
 * @param fmt Format of the message.
 * @param ... Message data.
 *
 * @return 0 on success, negative value on error.
 */
int log_error(const char *fmt, ...);

#ifdef DEBUG
/**
 * @brief Log a formatted debug message.
 *
 * @note This function should not exist in Release builds.
 *
 * @param fmt Format of the message.
 * @param ... Message data.
 */
void log_debug(const char *fmt, ...);
#else
# define log_debug
#endif

#endif /* LOG_H */
