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

#include "objwdt.h"

// singleton Watchdog object
STATIC wdt_obj_t wdt_obj = {
    .base.type = &wdt_type,
};

STATIC void wdt_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    wdt_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "WDT()");
}

STATIC mp_obj_t wdt_feed(mp_obj_t self_in) {
    watchdog_refresh();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(wdt_feed_obj, wdt_feed);

STATIC mp_obj_t wdt_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_id, ARG_timeout};
    const mp_arg_t wdt_init_args[] = {
        { MP_QSTR_id,       MP_ARG_INT,  {.u_int = 0} },
        { MP_QSTR_timeout,  MP_ARG_INT,  {.u_int = 5000} },
    };
    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(wdt_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), wdt_init_args, args);

    // check unit
    uint id = args[ARG_id].u_int;
    if (0 != id) {
        mp_raise_ValueError("Watchdog timer wrong unit, must use unit 0");
    }
    uint timeout = args[ARG_timeout].u_int;

    // init watchdog timer
    watchdog_init(timeout);
    watchdog_start();
    return (mp_obj_t)&wdt_obj;
}

// This is the entry point and is called when the module is imported
STATIC const mp_map_elem_t wdt_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),  MP_OBJ_NEW_QSTR(MP_QSTR_wdt) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_feed),  MP_OBJ_FROM_PTR(&wdt_feed_obj) },
};
STATIC MP_DEFINE_CONST_DICT(wdt_locals_dict, wdt_locals_dict_table);

const mp_obj_type_t wdt_type = {
    { &mp_type_type},
    .name        = MP_QSTR_WDT,
    .print       = wdt_print,
    .make_new    = wdt_make_new,
    .locals_dict = (mp_obj_t)&wdt_locals_dict,
};


