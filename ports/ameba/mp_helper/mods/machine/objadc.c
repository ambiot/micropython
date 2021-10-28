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
#include "objadc.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

/*****************************************************************************
 *                              Inernal variables
 * ***************************************************************************/
#define ADC_MAX_NUM  7

STATIC adc_obj_t adc_obj[ADC_MAX_NUM] = {{.base.type = &adc_type, .unit = 0, .pin = AD_0 },
                                         {.base.type = &adc_type, .unit = 1, .pin = AD_1 },
                                         {.base.type = &adc_type, .unit = 2, .pin = AD_2 },
                                         {.base.type = &adc_type, .unit = 3, .pin = AD_3 },
                                         {.base.type = &adc_type, .unit = 4, .pin = AD_4 },
                                         {.base.type = &adc_type, .unit = 5, .pin = AD_5 },
                                         {.base.type = &adc_type, .unit = 6, .pin = AD_6 }};

/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/
STATIC uint16_t offset = 0;
STATIC uint16_t gain = 0;

void adc_init0(void) {
	//sys_adc_calibration(0, &offset, &gain);
}

STATIC void adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    adc_obj_t *self = self_in;
    mp_printf(print, "ADC(%d)", self->unit);
}

STATIC mp_obj_t adc_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    const mp_arg_t adc_init_args[] = {
        { MP_QSTR_unit, MP_ARG_INT, {.u_int = 0} },
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(adc_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), adc_init_args, args);
    
    if ((args[0].u_int < 0) || (args[0].u_int > 6)) {
        mp_raise_ValueError("Pin function not avaliable");
    }

    int unit = args[0].u_int;

    adc_obj_t *self = &adc_obj[unit];

    analogin_init(&(self->obj), (PinName)(self->pin));

    return self;
}

STATIC mp_obj_t adc_read(mp_obj_t self_in) {
    adc_obj_t *self = self_in;
    uint16_t value = analogin_read_u16(&(self->obj));
    return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(adc_read_obj, adc_read);

#if 0
STATIC mp_obj_t adc_int2flt(mp_obj_t self_in, mp_obj_t ad_in) {
    adc_obj_t *self = self_in;
#if MICROPY_FLOAT_IMPL
    uint16_t ad_value = mp_obj_get_int(ad_in);
    return mp_obj_new_float(((ad_value / 16) - offset) * 1000 / gain);
#else
    mp_raise_ValueError("float not support");
    return mp_const_none;
#endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(adc_int2flt_obj, adc_int2flt);
#endif

STATIC const mp_map_elem_t adc_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),       MP_OBJ_FROM_PTR(&adc_read_obj) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_int2flt),    MP_OBJ_FROM_PTR(&adc_int2flt_obj) },
};
STATIC MP_DEFINE_CONST_DICT(adc_locals_dict, adc_locals_dict_table);

const mp_obj_type_t adc_type = {
    { &mp_type_type },
    .name        = MP_QSTR_ADC,
    .print       = adc_print,
    .make_new    = adc_make_new,
    .locals_dict = (mp_obj_t)&adc_locals_dict,
};
