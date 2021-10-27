/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Chester Tseng
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

/*****************************************************************************
 *                              Header includes
 * ***************************************************************************/
#include "objpacket.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

STATIC mp_obj_t packet_free(mp_obj_t self_in) {
    packet_obj_t *self = self_in;
    if (self->pkt != NULL) {
        pbuf_free(self->pkt);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(packet_free_obj, packet_free);

STATIC mp_obj_t packet_eth_src(mp_uint_t n_args, const mp_obj_t *args) {
    packet_obj_t *self = args[0];
    struct eth_hdr *ethhdr = (struct eth_hdr *)self->pkt->payload;
    if (n_args == 1) {
        byte mac_addr[ETHARP_HWADDR_LEN] = {0};
        mac_addr[0] = ethhdr->src.addr[0];
        mac_addr[1] = ethhdr->src.addr[1];
        mac_addr[2] = ethhdr->src.addr[2];
        mac_addr[3] = ethhdr->src.addr[3];
        mac_addr[4] = ethhdr->src.addr[4];
        mac_addr[5] = ethhdr->src.addr[5];
        return mp_obj_new_bytes(mac_addr, ETHARP_HWADDR_LEN);
    } else {
        struct eth_addr *mac = (struct eth_addr*)mp_obj_str_get_data(args[1], ETHARP_HWADDR_LEN);
        ETHADDR32_COPY(&ethhdr->src, mac);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(packet_eth_src_obj, 1, 2, packet_eth_src);

STATIC mp_obj_t packet_eth_dst(mp_uint_t n_args, const mp_obj_t *args) {
    packet_obj_t *self = args[0];
    struct eth_hdr *ethhdr = (struct eth_hdr *)self->pkt->payload;
    if (n_args == 1) {
        byte mac_addr[ETHARP_HWADDR_LEN] = {0};
        mac_addr[0] = ethhdr->dest.addr[0];
        mac_addr[1] = ethhdr->dest.addr[1];
        mac_addr[2] = ethhdr->dest.addr[2];
        mac_addr[3] = ethhdr->dest.addr[3];
        mac_addr[4] = ethhdr->dest.addr[4];
        mac_addr[5] = ethhdr->dest.addr[5];
        return mp_obj_new_bytes(mac_addr, ETHARP_HWADDR_LEN);
    } else {
        struct eth_addr *mac = (struct eth_addr*)mp_obj_str_get_data(args[1], ETHARP_HWADDR_LEN);
        ETHADDR32_COPY(&ethhdr->dest, mac);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(packet_eth_dst_obj, 1, 2, packet_eth_dst);

STATIC mp_obj_t packet_eth_type(mp_uint_t n_args, const mp_obj_t *args) {
    packet_obj_t *self = args[0];
    struct eth_hdr *ethhdr = (struct eth_hdr *)self->pkt->payload;
    if (n_args == 1) {
        return mp_obj_new_int(PP_HTONL(ethhdr->type));
    } else {
        uint type = mp_obj_get_int(args[1]);
        ethhdr->type = PP_HTONS(type);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(packet_eth_type_obj, 1, 2, packet_eth_type);

STATIC mp_obj_t packet_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    STATIC const mp_arg_t packet_init_args[] = {
        { MP_QSTR_length,  MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 64} },
    };
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(packet_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), packet_init_args, args);

    packet_obj_t *self = m_new_obj(packet_obj_t);
    self->base.type = (mp_obj_t)&packet_type;

    self->pkt = pbuf_alloc(PBUF_RAW, args[0].u_int, PBUF_POOL);

    if (self->pkt == NULL) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Alloc packet buffer failed"));
    }

    return self;
}

STATIC const mp_map_elem_t packet_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_free),        MP_OBJ_FROM_PTR(&packet_free_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_eth_dst),     MP_OBJ_FROM_PTR(&packet_eth_dst_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_eth_src),     MP_OBJ_FROM_PTR(&packet_eth_src_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_eth_type),    MP_OBJ_FROM_PTR(&packet_eth_type_obj) },
#if 0
    { MP_OBJ_NEW_QSTR(MP_QSTR_eth_payload), MP_OBJ_FROM_PTR(&packet_free_obj) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(packet_locals_dict, packet_locals_dict_table);

const mp_obj_type_t packet_type = {
    { &mp_type_type },
    .name           = MP_QSTR_PKT,
    .make_new       = packet_make_new,
    .locals_dict    = (mp_obj_t)&packet_locals_dict,
};
