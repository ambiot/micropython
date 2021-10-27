#include "py/mpconfig.h"
#include "py/runtime.h"
#include "py/stream.h"

typedef struct _socket_obj_t{
    mp_obj_base_t base;
    int fd;
    uint8_t domain;
    uint8_t type;
    uint8_t proto;
    bool peer_closed;
    unsigned int retries;
} socket_obj_t;
