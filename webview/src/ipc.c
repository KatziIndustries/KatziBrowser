// SPDX-License-Identifier: GPL-3.0
/* ipc.c
 *
 * the Inter-process Communication
 * read/write functions
 *
 * Author:
 * JRBlockkop <jrblockkop@gmail.com>
*/

#include "ipc.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int write_all(int fd, const void *buf, size_t size)
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

int read_all(int fd, void *buf, size_t size)
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

int send_msg(int fd, uint32_t type,const void *data, uint32_t size)
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