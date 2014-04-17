#ifndef _MOD_H
#define _MOD_H

#include <stddef.h>
#include <stdint.h>

#define PORT_FLAG_R 1
#define PORT_FLAG_W 2
#define PORT_FLAG_B 4
#define PORT_FLAG_S 8

typedef struct _port_info {
    uint8_t flags;
    uint8_t boundary;
} port_info_t;

typedef struct _mod_init_return {
    uint64_t launch_fn;
    uint64_t port_remove_fn;
    uint64_t port_add_fn;
    port_info_t ports[6];
    uint8_t requested_priority;
    uint8_t unused;
} mod_init_return_t;

#endif
