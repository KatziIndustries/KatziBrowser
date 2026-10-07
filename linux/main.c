#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>

typedef struct {
    uint32_t type;
    uint32_t size;
} MsgHeader;

static int write_all(int fd, const void *buf, size_t size)
{
    const char *p = buf;

    while (size > 0) {
        ssize_t n = write(fd, p, size);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        p += n;
        size -= n;
    }

    return 0;
}

static int read_all(int fd, void *buf, size_t size)
{
    char *p = buf;

    while (size > 0) {
        ssize_t n = read(fd, p, size);

        if (n == 0)
            return -1;

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        p += n;
        size -= n;
    }

    return 0;
}

static int send_msg(int fd, uint32_t type,
                    const void *data, uint32_t size)
{
    MsgHeader header = {
        .type = type,
        .size = size
    };

    if (write_all(fd, &header, sizeof(header)) < 0)
        return -1;

    if (size > 0 && write_all(fd, data, size) < 0)
        return -1;

    return 0;
}

static int recv_msg(int fd, MsgHeader *header,
                    void *data, size_t capacity)
{
    if (read_all(fd, header, sizeof(*header)) < 0)
        return -1;

    if (header->size > capacity)
        return -1;

    if (header->size > 0 &&
        read_all(fd, data, header->size) < 0)
        return -1;

    return 0;
}

int main(void)
{
    int ipc[2];

    /*
     * ipc[0] = parent
     * ipc[1] = child
     */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, ipc) < 0) {
        perror("socketpair");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /*
         * Child process.
         *
         * Pass ipc[1] to child using argv.
         */
        close(ipc[0]);

        char fd_string[32];

        snprintf(fd_string, sizeof(fd_string),
                 "%d", ipc[1]);

        execl("./child",
              "./child",
              fd_string,
              NULL);

        perror("execl");
        _exit(127);
    }

    /*
     * Parent.
     */
    close(ipc[1]);

    int fd = ipc[0];

    const char message[] = "Hello from parent!";

    printf("Parent: sending message...\n");

    if (send_msg(fd,
                 1,
                 message,
                 sizeof(message) - 1) < 0) {
        perror("send_msg");
        return 1;
    }

    /*
     * Receive child's echo.
     */
    char buffer[4096];
    MsgHeader header;

    if (recv_msg(fd, &header,
                  buffer, sizeof(buffer)) < 0) {
        fprintf(stderr, "Failed to receive message\n");
        return 1;
    }

    printf("Parent: received:\n");
    printf("  type = %u\n", header.type);
    printf("  size = %u\n", header.size);

    printf("  data = ");
    fwrite(buffer, 1, header.size, stdout);
    printf("\n");

    /*
     * Tell child to exit.
     */
    send_msg(fd, 2, NULL, 0);

    close(fd);

    waitpid(pid, NULL, 0);

    printf("Parent: child exited\n");

    return 0;
}