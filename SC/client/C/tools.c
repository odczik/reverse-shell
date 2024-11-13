#include <libwebsockets.h>

// Function to dynamically escape special characters in a JSON string
char *escape_json(const char *input) {
    size_t input_len = strlen(input);
    // Allocate buffer for output, assuming worst case where every char needs escaping
    size_t max_output_len = input_len * 2 + 1;  // +1 for null terminator
    char *escaped = (char *)malloc(max_output_len);
    if (!escaped) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    size_t j = 0;  // Index for the escaped string
    for (size_t i = 0; i < input_len; i++) {
        switch (input[i]) {
            case '\\':  // Backslash
                escaped[j++] = '\\';
                escaped[j++] = '\\';
                break;
            case '"':   // Double quotes
                escaped[j++] = '\\';
                escaped[j++] = '"';
                break;
            case '\b':  // Backspace
                escaped[j++] = '\\';
                escaped[j++] = 'b';
                break;
            case '\f':  // Form feed
                escaped[j++] = '\\';
                escaped[j++] = 'f';
                break;
            case '\n':  // Newline
                escaped[j++] = '\\';
                escaped[j++] = 'n';
                break;
            case '\r':  // Carriage return
                escaped[j++] = '\\';
                escaped[j++] = 'r';
                break;
            case '\t':  // Tab
                escaped[j++] = '\\';
                escaped[j++] = 't';
                break;
            default:
                // Copy any other character as is
                escaped[j++] = input[i];
                break;
        }
    }
    escaped[j] = '\0';  // Null-terminate the escaped string

    return escaped;  // Caller must free the memory allocated
}