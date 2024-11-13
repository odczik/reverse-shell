#include <libwebsockets.h>
#include <lmcons.h>

#include "tools.h"

void authenticate(struct lws *wsi) {
    char username[UNLEN+1];
    DWORD username_len = UNLEN+1;
    GetUserName(username, &username_len);
    char id_msg[128];

    snprintf(id_msg, sizeof(id_msg), "{ \"type\": \"id\", \"value\": \"%s\", \"accessCode\": \"tvojemama\" }", username);

    // Encode the message in Base64
    char *encoded_msg = base64_encode((unsigned char *)id_msg, strlen(id_msg));

    // Allocate a buffer for the message with LWS_PRE bytes padding for the header
    size_t msg_len = strlen(encoded_msg);
    unsigned char buf[LWS_PRE + 512];
    memcpy(&buf[LWS_PRE], encoded_msg, msg_len);
    
    free(encoded_msg);

    // Send the message
    int bytes_sent = lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_BINARY);
    if (bytes_sent < (int)msg_len) {
        fprintf(stderr, "Failed to send message completely\n");
    } else {
        printf("Sent message: %s\n", id_msg);
    }
}