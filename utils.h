#ifndef UTILS_H
#define UTILS_H

/* ============================================================
   utils.h — Trade Hub System
   Utility / helper function declarations
   ============================================================ */

/* Fills buffer with current timestamp "YYYY-MM-DD HH:MM" */
void get_timestamp(char *buffer, int size);

/* Trims leading/trailing whitespace and newline characters */
void trim_input(char *input);

/* Returns the next unique item ID (auto-increments) */
int generate_id(void);

/* Flushes leftover characters from stdin after scanf() */
void clear_input_buffer(void);

/* Returns 1 if price is valid (> 0), else 0 */
int validate_price(float price);

/* Prints a styled section divider with a centered title */
void print_header(const char *title);

/* Prints a centered header with border */
void print_centered_header(const char *title);

/* Converts a string to lowercase in-place */
void to_lowercase(char *str);

#endif /* UTILS_H */
