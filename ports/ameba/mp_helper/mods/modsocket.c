/*                                                                    
* Name:    modsocket.c                                                  
* Use:     Implement socket communication on micropython RTL8722 port    
* Author:  SimonXI                                                      
* Github:  https://github.com/xidameng
* License: MIT
* 
* Copyright(c) 2020 Simon XI
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.                                                                     
*/


#include <platform_opts.h>
#include <lwip/sockets.h> 
#include <lwip/netif.h>
#include <lwip/api.h>
#include <platform/platform_stdlib.h>
#include "main.h"
#include "modsocket.h"
#include "wireless/objwlan.h"


/////////////////////////////////////////////////
//                                             //
//            Global  Variables                //
//                                             //
/////////////////////////////////////////////////

/* all global variables start with a "amb" prefix to distinguish from others*/

#define DATA_LENTH      128

static uint32_t amb_ip_address = 0;
static bool amb_is_connected = false;
static int amb_client_sock = -1;
static int amb_server_sock;
static int amb_recvTimeout = 3000;
static struct sockaddr_in amb_cli_addr;


/////////////////////////////////////////////////
//                                             //
//            Internal Functions               //
//                                             //
/////////////////////////////////////////////////
    
int start_server(uint16_t port, uint8_t protMode)
{
    int _sock;
    int timeout;

    if (protMode == SOCK_STREAM) { // TCP
        timeout = 3000;
        _sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    } else { // UDP
        timeout = 1000;
        _sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    }

    if (_sock < 0) {
        printf("\r\nERROR opening socket\r\n");
        return -1;
    }

    struct sockaddr_in localHost;
    memset(&localHost, 0, sizeof(localHost));

    localHost.sin_family = AF_INET;
    localHost.sin_port = htons(port);
    localHost.sin_addr.s_addr = INADDR_ANY;

    if (lwip_bind(_sock, ((struct sockaddr *)&localHost), sizeof(localHost)) < 0) {
        printf("\r\nERROR on binding\r\n");
        return -1;
    }

    return _sock;
}

int sock_listen(int sock, int max)
{
    if (lwip_listen(sock , max) < 0) {
        printf("\r\nERROR on listening\r\n");
        return -1;
    }
    return 0;
}

int get_available(int sock)
{
    int enable = 1;
    int timeout;
    int client_fd;
    int err;

    socklen_t client = sizeof(amb_cli_addr);

    do {
        client_fd = lwip_accept(sock, ((struct sockaddr *)&amb_cli_addr), &client);
        if (client_fd < 0) {
            err = get_sock_errno(sock);
            if (err != EAGAIN) {
                break;
            }
        }
    } while (client_fd < 0);

    if (client_fd < 0) {
        printf("\r\nERROR on accept\r\n");
        return -1;
    } else {
        timeout = 3000;
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        timeout = 30000;
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
        lwip_setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
        printf("\r\nA client connected to this server :\r\n[PORT]: %d\r\n[IP]:%s\r\n", ntohs(amb_cli_addr.sin_port), inet_ntoa(amb_cli_addr.sin_addr.s_addr));
        return client_fd;
    }
}

int get_receive(int sock, uint8_t* data, int length, int flag, uint32_t *peer_addr, uint16_t *peer_port)
{
    int ret = 0;
    struct sockaddr from;
    socklen_t fromlen;

    uint8_t backup_recvtimeout = 0;
    int backup_recv_timeout, recv_timeout;
    socklen_t len;

    if (flag & 0x01) {
        // for MSG_PEEK, we try to peek packets by changing receiving timeout to 10ms
        ret = lwip_getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &backup_recv_timeout, &len);
        if (ret >= 0) {
            recv_timeout = 10;
            ret = lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
            if (ret >= 0) {
                backup_recvtimeout = 1;
            }
        }
    }
    ret = lwip_recvfrom(sock, data, length, flag, &from, &fromlen);
    if (ret >= 0) {
        if (peer_addr != NULL) {
            *peer_addr = ((struct sockaddr_in *)&from)->sin_addr.s_addr;
        }
        if (peer_port != NULL) {
            *peer_port = ntohs(((struct sockaddr_in *)&from)->sin_port);
        }
    }

    if ((flag & 0x01) && (backup_recvtimeout == 1)) {
        // restore receiving timeout
        lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &backup_recv_timeout, sizeof(recv_timeout));
    }

    return ret;
}

int get_sock_errno(int sock)
{
    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
    return so_error;
}

int set_sock_recv_timeout(int sock, int timeout)
{
    return lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void stop_socket(int sock)
{
    lwip_close(sock);
}

int send_data(int sock, const uint8_t *data, uint16_t len)
{
    int ret;
    ret = lwip_write(sock, data, len);
    return ret;
}

int sendto_data(int sock, const uint8_t *data, uint16_t len, uint32_t peer_ip, uint16_t peer_port)
{
    int ret;
    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = peer_ip;
    peer_addr.sin_port = htons(peer_port);

    ret = lwip_sendto(sock, data, len, 0, ((struct sockaddr*)&peer_addr), sizeof(struct sockaddr_in));

    return ret;
}

int start_client(uint32_t ipAddress, uint16_t port, uint8_t protMode)
{
    int enable = 1;
    int timeout;
    int _sock;

    if (protMode == SOCK_STREAM) {//tcp
        _sock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else { //udp
        _sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }

    if (_sock < 0) {
        printf("\r\nERROR opening socket\r\n");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ipAddress;
    serv_addr.sin_port = htons(port);

    if (protMode == SOCK_STREAM) {//TCP MODE
        if (lwip_connect(_sock, ((struct sockaddr *)&serv_addr), sizeof(serv_addr)) == 0) {
            printf("\r\nConnect to Server successfully!\r\n");

            timeout = 3000;
            lwip_setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            timeout = 30000;
            lwip_setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            lwip_setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
            lwip_setsockopt(_sock, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

            return _sock;
        } else {
            printf("\r\nConnect to Server failed!\r\n");
            stop_socket(_sock);
            return -1;
        }
    } else {
        printf("\r\nUdp client setup Server's information successful!\r\n");
    }

    return _sock;
}


// This API can take in both Domain name as well IPv4 address
int getHostByName(const char* aHostname)
{
    ip_addr_t ip_addr;
    err_t err;
    err = netconn_gethostbyname(aHostname, &ip_addr);

    if (err != ERR_OK) {
        return WL_FAILURE;
    } else {
        amb_ip_address = ip_addr.addr; // update global IP variable (uint32_t)
        return WL_SUCCESS;
    }
}


/////////////////////////////////////////////////
//                                             //
//            External Functions               //
//                                             //
/////////////////////////////////////////////////


/********************************/
/*            Common            */
/********************************/

STATIC mp_obj_t socket_send(const mp_obj_t self_in, const mp_obj_t arg1) {
    socket_obj_t *sock = self_in;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(arg1, &bufinfo, MP_BUFFER_READ);
    int r = send_data(sock->fd, bufinfo.buf, bufinfo.len);
    return mp_obj_new_int(r);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_send_obj, socket_send);



STATIC mp_obj_t socket_recv(const mp_obj_t self_in, const mp_obj_t arg_len) {
    socket_obj_t *sock = self_in;
    size_t len = mp_obj_get_int(arg_len);
    vstr_t vstr;
    vstr_init_len(&vstr, len);

    mp_uint_t ret = get_receive(sock->fd, vstr.buf, len, NULL, NULL, NULL);
    if (ret == -1) {
        mp_raise_ValueError("socket failed to recv");
    }
    vstr.len = ret;
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_recv_obj, socket_recv);


STATIC mp_obj_t socket_close(const mp_obj_t self_in) {
    socket_obj_t *sock = self_in;
    stop_socket(sock->fd);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(socket_close_obj, socket_close);



STATIC mp_obj_t socket_settimeout(const mp_obj_t self_in, const mp_obj_t arg_time) {
    socket_obj_t *sock = self_in;
    int timeout = mp_obj_get_int(arg_time); // timeout in seconds
    set_sock_recv_timeout(sock->fd, timeout*1000);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(socket_settimeout_obj, socket_settimeout);



/********************************/
/*            Client            */
/********************************/

STATIC mp_obj_t client_connect(mp_obj_t self_in, mp_obj_t arg_host,  mp_obj_t arg_port) {

    socket_obj_t *self = self_in;
    char * host = mp_obj_str_get_str(arg_host);
    uint16_t port = mp_obj_get_int(arg_port);

    if (getHostByName(host) == WL_SUCCESS) {
        if (self->type == SOCK_DGRAM) { //udp
            amb_client_sock = start_client(amb_ip_address, port, SOCK_DGRAM);
        } else { //tcp
            amb_client_sock = start_client(amb_ip_address, port, SOCK_STREAM);
        }
    } else {
        printf("Get host IP address failed\n");
        return mp_const_false;
    }

    if (amb_client_sock < 0) {
        amb_is_connected = false;
        mp_raise_ValueError("client start failed");
        return mp_const_false;
    } else {
        amb_is_connected = true;
        set_sock_recv_timeout(amb_client_sock, amb_recvTimeout);
        printf("client connect successfully\n");
    }
    return mp_const_true;
}
MP_DEFINE_CONST_FUN_OBJ_3(socket_connect_obj, client_connect);





/********************************/
/*            Server            */
/********************************/

STATIC mp_obj_t server_bind(mp_obj_t self_in, mp_obj_t arg_port) {
    socket_obj_t *sock = self_in;
    // default using localhost as server addr, thus only port is required
    amb_server_sock = start_server(mp_obj_get_int(arg_port), SOCK_STREAM);
    if ( amb_server_sock >= 0) { // default server TCP mode
        printf("server bind success\n");
        return mp_const_none;
    } else {
        mp_raise_ValueError("server bind failed");
    }
}
MP_DEFINE_CONST_FUN_OBJ_2(socket_bind_obj, server_bind);


STATIC mp_obj_t server_listen(mp_obj_t self_in) {

    if ( amb_server_sock >= 0) {
        sock_listen( amb_server_sock, 1 );  // default backlog set to 1, only 1 connection allows to queue
        return mp_const_none;
    } else {
        mp_raise_ValueError("make sure server is successfully binded before re-attemp");
    }
}
MP_DEFINE_CONST_FUN_OBJ_1(socket_listen_obj, server_listen);



STATIC mp_obj_t server_accept(mp_obj_t self_in) {
    socket_obj_t *self = self_in;

    int client_fd = get_available(amb_server_sock);

    // create new socket object for communicating with client
    socket_obj_t *sock = m_new_obj_with_finaliser(socket_obj_t);
    sock->base.type = self->base.type;
    sock->fd = client_fd;
    sock->domain = self->domain;
    sock->type = self->type;
    sock->proto = self->proto;
    sock->peer_closed = false;
    set_sock_recv_timeout(sock->fd, amb_recvTimeout);

    mp_obj_tuple_t *client = mp_obj_new_tuple(2, NULL);
    client->items[0] = sock;

    mp_obj_t tuple[2] = {
        tuple[0] = mp_obj_new_str(inet_ntoa(amb_cli_addr.sin_addr.s_addr), strlen(inet_ntoa(amb_cli_addr.sin_addr.s_addr))),
        tuple[1] = mp_obj_new_int(ntohs(amb_cli_addr.sin_port)),
    };
    client->items[1] = mp_obj_new_tuple(2, tuple);

    return client;
}
MP_DEFINE_CONST_FUN_OBJ_1(socket_accept_obj, server_accept);





/////////////////////////////////////////////////
//                                             //
//            MicroPython API                  //
//                                             //
/////////////////////////////////////////////////


STATIC mp_obj_t socket_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 2, false);

    socket_obj_t *sock = m_new_obj_with_finaliser(socket_obj_t);
    sock->base.type = type_in;
    sock->domain = AF_INET;     //IPv4
    sock->type = SOCK_STREAM;   //Stream (TCP)
    sock->proto = 0;            //TCP
    sock->peer_closed = false;
    if (n_args > 0) {
        sock->domain = mp_obj_get_int(args[0]);
        if (n_args > 1) {
            sock->type = mp_obj_get_int(args[1]);
        }
    }
    return MP_OBJ_FROM_PTR(sock);
}



STATIC const mp_rom_map_elem_t socket_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_connect),     MP_ROM_PTR(&socket_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_bind),        MP_ROM_PTR(&socket_bind_obj) },
    { MP_ROM_QSTR(MP_QSTR_listen),      MP_ROM_PTR(&socket_listen_obj) },
    { MP_ROM_QSTR(MP_QSTR_accept),      MP_ROM_PTR(&socket_accept_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv),        MP_ROM_PTR(&socket_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_send),        MP_ROM_PTR(&socket_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_settimeout),  MP_ROM_PTR(&socket_settimeout_obj) },
    { MP_ROM_QSTR(MP_QSTR_close),       MP_ROM_PTR(&socket_close_obj) },
#if 0
    { MP_ROM_QSTR(MP_QSTR_sendall),     MP_ROM_PTR(&socket_sendall_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendto),      MP_ROM_PTR(&socket_sendto_obj) },
    { MP_ROM_QSTR(MP_QSTR_recvfrom),    MP_ROM_PTR(&socket_recvfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_setsockopt),  MP_ROM_PTR(&socket_setsockopt_obj) },
    { MP_ROM_QSTR(MP_QSTR_setblocking), MP_ROM_PTR(&socket_setblocking_obj) },
    { MP_ROM_QSTR(MP_QSTR_makefile),    MP_ROM_PTR(&socket_makefile_obj) },
    { MP_ROM_QSTR(MP_QSTR_fileno),      MP_ROM_PTR(&socket_fileno_obj) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(socket_locals_dict, socket_locals_dict_table);

STATIC const mp_obj_type_t socket_type = {
    { &mp_type_type },
    .name = MP_QSTR_SOCK,
    .make_new = socket_make_new,
    .locals_dict = (mp_obj_t)&socket_locals_dict,
};



STATIC const mp_rom_map_elem_t mp_module_socket_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),            MP_ROM_QSTR(MP_QSTR_usocket) },
    { MP_ROM_QSTR(MP_QSTR_SOCK),              MP_ROM_PTR(&socket_type) },
    // module constants
    { MP_ROM_QSTR(MP_QSTR_AF_INET),             MP_ROM_INT(AF_INET) },
    { MP_ROM_QSTR(MP_QSTR_AF_INET6),            MP_ROM_INT(AF_INET6) },
    { MP_ROM_QSTR(MP_QSTR_SOCK_STREAM),         MP_ROM_INT(SOCK_STREAM) },
    { MP_ROM_QSTR(MP_QSTR_SOCK_DGRAM),          MP_ROM_INT(SOCK_DGRAM) },
    //{ MP_ROM_QSTR(MP_QSTR_SOCK_RAW),            MP_ROM_INT(SOCK_RAW) },
    //{ MP_ROM_QSTR(MP_QSTR_IPPROTO_TCP),         MP_ROM_INT(IPPROTO_TCP) },
    //{ MP_ROM_QSTR(MP_QSTR_IPPROTO_UDP),         MP_ROM_INT(IPPROTO_UDP) },
    //{ MP_ROM_QSTR(MP_QSTR_IPPROTO_IP),          MP_ROM_INT(IPPROTO_IP) },
    //{ MP_ROM_QSTR(MP_QSTR_SOL_SOCKET),          MP_ROM_INT(SOL_SOCKET) },
    //{ MP_ROM_QSTR(MP_QSTR_SO_REUSEADDR),        MP_ROM_INT(SO_REUSEADDR) },
    //{ MP_ROM_QSTR(MP_QSTR_IP_ADD_MEMBERSHIP),   MP_ROM_INT(IP_ADD_MEMBERSHIP) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_socket_globals, mp_module_socket_globals_table);

const mp_obj_module_t mp_module_usocket = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_socket_globals,
};
