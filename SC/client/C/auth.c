#include <libwebsockets.h>
#include <lmcons.h>

void authenticate(struct lws *wsi) {
    char username[UNLEN+1];
    DWORD username_len = UNLEN+1;
    GetUserName(username, &username_len);
    char id_msg[128];

    snprintf(id_msg, sizeof(id_msg), "{ \"type\": \"id\", \"value\": \"%s\", \"accessCode\": \"tvojemama\" }", username);

    // Allocate a buffer for the message with LWS_PRE bytes padding for the header
    size_t msg_len = strlen(id_msg);
    unsigned char buf[LWS_PRE + 512];
    memcpy(&buf[LWS_PRE], id_msg, msg_len);

    // Send the message
    int bytes_sent = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
    if (bytes_sent < (int)msg_len) {
        fprintf(stderr, "Failed to send message completely\n");
    } else {
        printf("Sent message: %s\n", id_msg);
    }
}