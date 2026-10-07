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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ipc.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        return 1;
    }

    int fd = atoi(argv[1]);

    char buffer[4096];

    while (1) {
        MsgHeader header;

        if (read_all(fd, &header, sizeof(header)) < 0)
            break;

        if (header.size > sizeof(buffer)) {
            fprintf(stderr, "Message too large\n");
            break;
        }

        if (header.size > 0) {
            if (read_all(fd, buffer, header.size) < 0)
                break;
        }

        printf("received: type=%u size=%u data=\"%s\"\n",
               header.type,
               header.size,
               buffer
        );

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