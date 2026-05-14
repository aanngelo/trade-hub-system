#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "utils.h"

// ============================================================
// utils.c â€” Trade Hub System
// Utility / helper function implementations
// ============================================================

// Static counter for generating unique item IDs
static int id_counter = 5000;

// get_timestamp
// Fills buffer with current date and time.
// Format: "YYYY-MM-DD HH:MM"
void get_timestamp(char *buffer, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M", t);
}

// trim_input
// Trims whitespace and newline characters from a string.
void trim_input(char *input) {
    if (!input) {
        return;
    }

    size_t len = strlen(input);
    while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r' || isspace((unsigned char)input[len - 1]))) {
        input[len - 1] = '\0';
        len--;
    }

    size_t start = 0;
    while (input[start] != '\0' && isspace((unsigned char)input[start])) {
        start++;
    }

    if (start > 0) {
        memmove(input, input + start, len - start + 1);
    }
}

// generate_id
// Returns the next unique item ID starting from 5001.
int generate_id(void) {
    return id_counter++;
}

// clear_input_buffer
// Clears leftover characters in stdin.
// Always call this after scanf() to avoid input bugs.
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// validate_price
// Returns 1 if price is positive, 0 otherwise.
int validate_price(float price) {
    return price > 0;
}

// print_header
// Prints a styled divider with a centered title using 80-char border.
void print_header(const char *title) {
    printf("\n");
    printf("================================================================================\n");
    
    int title_len = strlen(title);
    int total_width = 80;
    int padding = (total_width - title_len) / 2;
    
    printf("%*s%s\n", padding, "", title);
    printf("================================================================================\n");
}

// print_centered_header
// Prints a centered header with 80-char decorative border.
void print_centered_header(const char *title) {
    printf("\n");
    printf("================================================================================\n");
    int title_len = strlen(title);
    int total_width = 80;
    int padding = (total_width - title_len) / 2;
    printf("%*s%s\n", padding, "", title);
    printf("================================================================================\n");
}

// to_lowercase
// Converts all characters in str to lowercase in-place.
// Used for case-insensitive keyword search.
void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}
