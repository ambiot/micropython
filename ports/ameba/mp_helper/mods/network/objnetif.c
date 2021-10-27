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
#include "objnetif.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

/*****************************************************************************
 *                              Local functions
 * ***************************************************************************/

void netif_init0(void) {
    // DO nothing here
}

STATIC void netif_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    netif_obj_t *self = self_in;
    struct ip_addr ipaddr  = self->piface->ip_addr;
    struct ip_addr netmask = self->piface->netmask;
    struct ip_addr gateway = self->piface->gw;
    mp_printf(print, "NETIF(%c%c, ", self->piface->name[0], self->piface->name[1]);
    mp_printf(print, "ip=%s, ", ip_ntoa(&ipaddr));
    mp_printf(print, "netmask=%s, ", ip_ntoa(&netmask));
    mp_printf(print, "gateway=%s)", ip_ntoa(&gateway));
}

STATIC mp_obj_t ip_get(mp_uint_t n_args, const mp_obj_t *args) {
    netif_obj_t *self = args[0];
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gateway;
    if (n_args == 1) {
        mp_obj_t tuple[3];
        ipaddr = self->piface->ip_addr;   
        netmask = self->piface->netmask;   
        gateway = self->piface->gw;
        tuple[0] = mp_obj_new_str(ip_ntoa(&ipaddr), strlen(ip_ntoa(&ipaddr)));
        tuple[1] = mp_obj_new_str(ip_ntoa(&netmask), strlen(ip_ntoa(&netmask)));
        tuple[2] = mp_obj_new_str(ip_ntoa(&gateway), strlen(ip_ntoa(&gateway)));
        return mp_obj_new_tuple(3, tuple);
    } else {
        mp_obj_t *sec;
        mp_obj_get_array_fixed_n(args[1], 3, &sec);
        int8_t *paddr = mp_obj_str_get_str(sec[0]);
        int8_t *pnetmask = mp_obj_str_get_str(sec[1]);
        int8_t *pgw = mp_obj_str_get_str(sec[2]);
        if (ipaddr_aton(paddr, &ipaddr) == 0)
            mp_raise_ValueError("NETIF ip format invalid");
        if (ipaddr_aton(pnetmask, &netmask) == 0)
            mp_raise_ValueError("NETIF netmask format invalid");
        if (ipaddr_aton(pgw, &gateway) == 0)
            mp_raise_ValueError("NETIF gateway format invalid");

        netif_set_addr(self->piface, &ipaddr, &netmask, &gateway);

        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ip_get_obj, 1, 2, ip_get);

STATIC mp_obj_t netif_set_default0(mp_obj_t self_in) {
    netif_obj_t *self = self_in;
    
    netif_set_default(self->piface);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(netif_set_default_obj, netif_set_default0);

STATIC mp_obj_t netif_set_linkup0(mp_obj_t self_in) {
    netif_obj_t *self = self_in;
    
    netif_set_link_up(self->piface);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(netif_set_linkup_obj, netif_set_linkup0);

STATIC mp_obj_t netif_set_linkdown0(mp_obj_t self_in) {
    netif_obj_t *self = self_in;
    
    netif_set_link_down(self->piface);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(netif_set_linkdown_obj, netif_set_linkdown0);

STATIC mp_obj_t netif_flags(mp_uint_t n_args, const mp_obj_t *args) {
    netif_obj_t *self = args[0];
    if (n_args == 1) {
        // get the value
        return mp_obj_new_int(self->piface->flags);
    } else {
        uint8_t flags = mp_obj_get_int(args[1]);
        self->piface->flags = flags;
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(netif_flags_obj, 1, 2, netif_flags);

STATIC mp_obj_t netif_hostname(mp_uint_t n_args, const mp_obj_t *args) {
    netif_obj_t *self = args[0];
    char* hostname = NULL;
    uint8_t len = 0;
    if (n_args == 1) {
        // get the value
        hostname = self->piface->hostname;
        return mp_obj_new_str(hostname, strlen(hostname));
    } else {
        hostname = mp_obj_str_get_data(args[1], &len);
        self->piface->hostname = hostname;
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(netif_hostname_obj, 1, 2, netif_hostname);

STATIC mp_obj_t dhcp_request0(mp_obj_t self_in, mp_obj_t timeout_in) {
    netif_obj_t *self = self_in;
    self->piface->ip_addr.addr = 0;
    self->piface->netmask.addr = 0;
    self->piface->gw.addr = 0;

    int8_t err = dhcp_start(self->piface);

    if (err != ERR_OK) {
        mp_raise_msg(&mp_type_OSError, "NETIF dhcp start error");
    }

    uint16_t timeout = 0;
    uint16_t counter = 0;

    timeout = mp_obj_get_int(timeout_in);

    while (timeout) {
        if (self->piface->ip_addr.addr != 0) {
            return mp_const_none;
        }
        mp_hal_delay_ms(DHCP_FINE_TIMER_MSECS);
        dhcp_fine_tmr();
        counter += DHCP_FINE_TIMER_MSECS;
        if (counter >= (DHCP_COARSE_TIMER_SECS * 1000)) {
            dhcp_coarse_tmr();
            counter = 0;
            timeout--;
        }
    }
    mp_raise_msg(&mp_type_TimeoutError, "NETIF DHCP request timeout");
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(dhcp_request_obj, dhcp_request0);

STATIC mp_obj_t dhcp_renew0(mp_obj_t self_in) {
    int8_t err ;
    netif_obj_t *self = self_in;
    err = dhcp_renew(self->piface);
    if (err != ERR_OK) {
        mp_raise_ValueError("NETIF DHCP renew failed");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(dhcp_renew_obj, dhcp_renew0);

STATIC mp_obj_t dhcp_release0(mp_obj_t self_in) {
    int8_t err;
    netif_obj_t *self = self_in;
    err = dhcp_release(self->piface);
    if (err != ERR_OK) {
        mp_raise_ValueError("NETIF DHCP release failed");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(dhcp_release_obj, dhcp_release0);

STATIC mp_obj_t dhcp_inform0(mp_obj_t self_in) {
    netif_obj_t *self = self_in;
    dhcp_inform(self->piface);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(dhcp_inform_obj, dhcp_inform0);

STATIC mp_obj_t dhcp_stop0(mp_obj_t self_in) {
    netif_obj_t *self = self_in;
    dhcp_stop(self->piface);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(dhcp_stop_obj, dhcp_stop0);

STATIC mp_obj_t netif_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    STATIC const mp_arg_t netif_init_args[] = {
        { MP_QSTR_index, MP_ARG_REQUIRED | MP_ARG_INT,  {.u_int = 0} },
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(netif_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), netif_init_args, args);

    netif_obj_t *self;

    return (mp_obj_t)self;
}

STATIC const mp_map_elem_t netif_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_ip),           MP_OBJ_FROM_PTR(&ip_get_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_default),      MP_OBJ_FROM_PTR(&netif_set_default_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_linkup),       MP_OBJ_FROM_PTR(&netif_set_linkup_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_linkdown),     MP_OBJ_FROM_PTR(&netif_set_linkdown_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_hostname),     MP_OBJ_FROM_PTR(&netif_hostname_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_flags),        MP_OBJ_FROM_PTR(&netif_flags_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcp_request), MP_OBJ_FROM_PTR(&dhcp_request_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcp_renew),   MP_OBJ_FROM_PTR(&dhcp_renew_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcp_release), MP_OBJ_FROM_PTR(&dhcp_release_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcp_inform),  MP_OBJ_FROM_PTR(&dhcp_inform_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dhcp_stop),    MP_OBJ_FROM_PTR(&dhcp_stop_obj) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_UP),            MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_UP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_BROADCAST),     MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_BROADCAST) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_POINTTOPOINT),  MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_POINTTOPOINT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_DHCP),          MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_DHCP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_LINK_UP),       MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_LINK_UP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_ETHARP),        MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_ETHARP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_ETHERNET),      MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_ETHERNET) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLAG_IGMP),          MP_OBJ_NEW_SMALL_INT(NETIF_FLAG_IGMP) },
};
STATIC MP_DEFINE_CONST_DICT(netif_locals_dict, netif_locals_dict_table);

const mp_obj_type_t netif_type = {
    { &mp_type_type },
    .name        = MP_QSTR_NETIF,
    .print       = netif_print,
    .make_new    = netif_make_new,
    .locals_dict = (mp_obj_dict_t *)&netif_locals_dict,
};
