#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logging.h"

/*need to:
create a fifo (char **argv)
send request to server (int argc)

listen for input
send it to the fifo*/

#define BUFFER_SIZE (128)

// Helper function to send messages
// Retries to send whatever was not sent in the beginning
void send_msg(int tx, char const *str) {
    size_t len = strlen(str);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(tx, str + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        written += ret;
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Needs the name of the pipe.\n");
        return 1;
    }
    // remove pipe if it does exist
    if (unlink(FIFO_PATHNAME) != 0 && errno != ENOENT) {
        fprintf(stderr, "[ERR]: unlink(%s) failed: %s\n", FIFO_PATHNAME,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    // create pipe
    if (mkfifo(FIFO_PATHNAME, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // open pipe for writing
    // this waits for someone to open it for reading
    int tx = open(FIFO_PATHNAME, O_WRONLY);
    if (tx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

    ssize_t ret;

    char *second_pipe = argv[1];
    // sends the pipename to the other side
    send_msg(tx, second_pipe);      //!sends second pipe name

    // needs to wait for the other pipe is created
    sleep(1);

    // open pipe for reading
    // this waits for someone to open it for writing
    int rx = open(second_pipe, O_RDONLY);
    if (rx == -1) {
        fprintf(stderr, "[ERR]: open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	// The parent likes classic rock:
	// https://www.youtube.com/watch?v=Kc71KZG87X4
	send_msg(tx, "In the burning heart\n");
    ret = read(rx, buffer, BUFFER_SIZE - 1);
    buffer[ret] = 0;
    fputs(buffer, stdout);

	send_msg(tx, "Just about to burst\n");
    ret = read(rx, buffer, BUFFER_SIZE - 1);
    buffer[ret] = 0;
    fputs(buffer, stdout);

	send_msg(tx, "There's a quest for answers\n");
    ret = read(rx, buffer, BUFFER_SIZE - 1);
    buffer[ret] = 0;
    fputs(buffer, stdout);

	fprintf(stderr, "[INFO]: closing pipe\n");
    close(tx);
	close(tx);

    unlink(FIFO_PATHNAME);
}
