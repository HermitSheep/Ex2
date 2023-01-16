#include "../producer-consumer/producer-consumer.h"
#include "../utils/utility_funcs.h"

int main() {
    unsigned long int size = 0;
    uint8_t code = 3;
    char request[MAX_REQUEST]; // code (1-10) | pipe name | box name \0

    char pipe[MAX_PIPE_NAME];
    char boc[MAX_BOX_NAME];

    strcpy(pipe, "session_pipe");
    strcpy(boc, "box_name");

    if (strlen(pipe) <= MAX_PIPE_NAME) {
        memset(pipe + strlen(pipe), 0,
               sizeof(pipe) - sizeof(char) * strlen(pipe));
    }
    if (strlen(boc) < MAX_BOX_NAME) {
        memset(boc + strlen(boc), 0, sizeof(boc) - sizeof(char) * strlen(boc));
    }
    //*FORMAT REQUEST
    memcpy(request, &code, sizeof(code));
    size += sizeof(code);
    printf("request: %s\n", request);
    strncpy(request + size, pipe, MAX_PIPE_NAME);
    size += strlen(pipe);
    request[size] = '\0';
    printf("request: %s\n", request);

    strncpy(request + size, boc, MAX_BOX_NAME);
    size += strlen(boc);
    request[size] = '\0';
    printf("request: %s\n", request);
}