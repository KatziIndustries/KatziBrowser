// SPDX-License-Identifier: GPL-3.0
/* main.c
 *
 * the main function of the webview
 * process that controls the actions
 * the browser takes
 *
 * Author:
 * JRBlockkop <jrblockkop@gmail.com>
*/

#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ipc.h"

#define BUFFER_SIZE 4096

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;

    if (strlen(argv[1]) >= sizeof(addr.sun_path)) {
        fprintf(stderr, "Socket path too long\n");
        close(fd);
        return 1;
    }

    strcpy(addr.sun_path, argv[1]);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    char buffer[BUFFER_SIZE + 1];

    for (;;) {
        MsgHeader header;

        if (read_all(fd, &header, sizeof(header)) < 0)
            break;

        if (header.size > BUFFER_SIZE) {
            fprintf(stderr, "Message too large\n");
            break;
        }

        if (header.size > 0 &&
            read_all(fd, buffer, header.size) < 0)
            break;

        buffer[header.size] = '\0';

        printf("received: type=%u size=%u data=\"%.*s\"\n",
               header.type,
               header.size,
               (int)header.size,
               buffer);

        //later parse the message
        if (header.type == 1) {

            send_msg(
                fd,
                header.type,
                buffer,
                header.size
            );

        } else if (header.type == 2) {
            break;
        }
    }

    close(fd);

    return 0;
}
