#include <libwebsockets.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

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

char *base64_encode(const unsigned char *input, int length) {
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    // Disable newline character in output
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // Write the data to the BIO chain
    BIO_write(b64, input, length);
    BIO_flush(b64);

    // Get the memory pointer from the BIO and copy it to a C string
    BUF_MEM *buffer_ptr;
    BIO_get_mem_ptr(b64, &buffer_ptr);
    char *encoded_data = (char *)malloc(buffer_ptr->length + 1);
    memcpy(encoded_data, buffer_ptr->data, buffer_ptr->length);
    encoded_data[buffer_ptr->length] = '\0';

    // Clean up
    BIO_free_all(b64);

    return encoded_data;
}