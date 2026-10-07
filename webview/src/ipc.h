// SPDX-License-Identifier: GPL-3.0
/* ipc.h
 *
 * the Inter-process Communication
 * header file
 *
 * Author:
 * JRBlockkop <jrblockkop@gmail.com>
*/

#ifndef IPC_H
#define IPC_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t type;
    uint32_t size;
} MsgHeader;

int write_all(int fd, const void *buf, size_t size);
int read_all(int fd, void *buf, size_t size);
int send_msg(int fd, uint32_t type,const void *data, uint32_t size);

#endif