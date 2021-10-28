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

#include "objdac.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/
STATIC const dac_obj_t dac_obj = {.base.type = &dac_type, .unit = 0, .resolution = 10};

/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/
STATIC mp_obj_t dac_write(mp_obj_t self_in, mp_obj_t val_in) {
    dac_obj_t *self = self_in;
    if (MP_OBJ_IS_INT(val_in)) {
        uint32_t value = mp_obj_get_int(val_in);
        value %= (1<<self->resolution);
        analogout_write(&(self->obj), value * 1.0 / (1<<self->resolution));
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, mpexception_value_invalid_arguments));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(dac_write_obj, dac_write);

STATIC void dac_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    dac_obj_t *self = self_in;
    mp_printf(print, "DAC(%d)", self->unit);
}

STATIC mp_obj_t dac_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    const mp_arg_t dac_init_args[] = {
        { MP_QSTR_unit,       MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_resolution, MP_ARG_INT, {.u_int = 10} },
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(dac_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), dac_init_args, args);

    if ((args[0].u_int < 0) || (args[0].u_int > 0)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, mpexception_os_resource_not_avaliable));
    }

    dac_obj_t *self = &dac_obj;

    self->resolution = args[1].u_int;

    analogout_init(&(self->obj), DA_0);

    return self;
}

STATIC const mp_map_elem_t dac_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),    MP_OBJ_FROM_PTR(&dac_write_obj) },
};
STATIC MP_DEFINE_CONST_DICT(dac_locals_dict, dac_locals_dict_table);

const mp_obj_type_t dac_type = {
    { &mp_type_type },
    .name        = MP_QSTR_DAC,
    .print       = dac_print,
    .make_new    = dac_make_new,
    .locals_dict = (mp_obj_t)&dac_locals_dict,
};
