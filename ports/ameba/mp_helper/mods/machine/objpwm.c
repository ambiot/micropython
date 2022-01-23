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
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "exception.h"

#include "objpin.h"
#include "objpwm.h"

STATIC machine_pwm_obj_t pwm_obj[9] = {
    {.base.type = &machine_pwm_type, .unit = 0},
    {.base.type = &machine_pwm_type, .unit = 1},
    {.base.type = &machine_pwm_type, .unit = 2},
    {.base.type = &machine_pwm_type, .unit = 3},
    {.base.type = &machine_pwm_type, .unit = 4},
    {.base.type = &machine_pwm_type, .unit = 5},
    {.base.type = &machine_pwm_type, .unit = 6},
    {.base.type = &machine_pwm_type, .unit = 7},
    {.base.type = &machine_pwm_type, .unit = 8}
};

STATIC void mp_machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pwm_obj_t *self = self_in;
    mp_printf(print, "PWM(%d, pin=%q)", self->unit, self->pin->name);
}


STATIC mp_obj_t mp_machine_pwm_freq_set(mp_obj_t self_in, mp_int_t freq_in) {
    machine_pwm_obj_t *self = self_in;
    
    self->freq = freq_in;

    if (self->freq > 1000000 | self->freq < 1) {
        mp_raise_ValueError("frequency not supported, try 1 - 1MHz");
    }
    
    float sec = 1 / (float)self->freq;

    pwmout_period(&(self->obj), sec);
    return mp_const_none;
}
//STATIC MP_DEFINE_CONST_FUN_OBJ_2(pwm_freq_obj, pwm_freq);

STATIC mp_obj_t mp_machine_pwm_freq_get(machine_pwm_obj_t *self) {
    return MP_OBJ_NEW_SMALL_INT(self->freq);
}


#if 0
STATIC mp_obj_t pwm_write(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 2) {
        pwmout_write(&(self->obj), mp_obj_get_float(args[1]));
    } else {
        pwmout_write(&(self->obj), 0);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pwm_write_obj, 1, 2, pwm_write);
#else
STATIC mp_obj_t mp_machine_pwm_duty_get_u16(machine_pwm_obj_t *self) {
    
    return MP_OBJ_NEW_SMALL_INT((int)(self->duty * 65535));
}

STATIC void mp_machine_pwm_duty_set_u16(machine_pwm_obj_t *self, mp_int_t duty_u16) {
    self->duty = (float)(duty_u16/65535.0);
    //printf("duty cycle now is %f\n", self->duty);
    pwmout_write(&(self->obj), self->duty);
}

STATIC mp_obj_t mp_machine_pwm_duty_get_ns(machine_pwm_obj_t *self) {
    
    return MP_OBJ_NEW_SMALL_INT(self->pulseWidth);
}

STATIC void mp_machine_pwm_duty_set_ns(machine_pwm_obj_t *self, mp_int_t duty_ns) {
    printf("Note: Due to hardware limitation, only micro second pulse width can be generated!\n");
    printf("You have entered %d micro seconds\n",duty_ns);
    self->pulseWidth = duty_ns;
    pwmout_pulsewidth_us(&(self->obj), duty_ns); // duty_ns is treated at micro second here
}
#endif

STATIC mp_obj_t mp_machine_pwm_deinit(mp_obj_t self_in) {
    machine_pwm_obj_t *self = self_in;
    pwmout_free(&(self->obj));
    return mp_const_none;
}
//STATIC MP_DEFINE_CONST_FUN_OBJ_1(pwm_deinit_obj, pwm_deinit);


STATIC mp_obj_t mp_machine_pwm_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_pin, ARG_unit};
    const mp_arg_t pwm_init_args[] = {
        { MP_QSTR_pin,  MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_unit, MP_ARG_INT, {.u_int = 0} },
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(pwm_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), pwm_init_args, args);

    pin_obj_t *pin = pin_find(args[ARG_pin].u_obj);

    PinName pn_pin = (PinName)pinmap_peripheral(pin->id, PinMap_PWM);

    if (pn_pin == NC)
        mp_raise_ValueError("PWM pin not match");

    machine_pwm_obj_t *self = &pwm_obj[args[ARG_unit].u_int];
    self->pin = pin;

    pwmout_init(&(self->obj), self->pin->id);
    pwmout_period_ms(&(self->obj), 1); //default 1KHz
    pwmout_write(&(self->obj), 0.0);
    return (mp_obj_t)self;
}

#if 0
STATIC const mp_map_elem_t pwm_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_freq),     MP_OBJ_FROM_PTR(&pwm_freq_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),    MP_OBJ_FROM_PTR(&pwm_write_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),    MP_OBJ_FROM_PTR(&pwm_deinit_obj) },
};
STATIC MP_DEFINE_CONST_DICT(pwm_locals_dict, pwm_locals_dict_table);

const mp_obj_type_t pwm_type = {
    { &mp_type_type },
    .name        = MP_QSTR_PWM,
    .print       = pwm_print,
    .make_new    = pwm_make_new,
    .locals_dict = (mp_obj_dict_t *)&pwm_locals_dict,
};
#endif