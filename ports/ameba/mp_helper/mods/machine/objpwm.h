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
#ifndef OBJPWM_H_
#define OBJPWM_H_

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "pwmout_api.h"
#include "PinNames.h"
#include "objpin.h"

extern const mp_obj_type_t pwm_type;


/**
  * @brief  Table elements express the pin to PWM channel number, they are:
  *           {pinName, km0_pin2chan, km4_pin2chan}
  */
static const PinMap PinMap_PWM[] = {
    {PA_13,  0, 0},
    {PA_13,  1, 1},
    {PA_23,  2, 2},
    {PA_24,  3, 3},
    {PA_25,  4, 4},
    {PA_26,  5, 5},
    {PA_28,  6, 6},
    {PA_30,  7, 7},
    {PB_4,  2, 8},
    {PB_5,  3, 9},
    {PB_7,  5, 17},
    {PB_18,  4, 10},
    {PB_19,  5, 11},
    {PB_20,  0, 12},
    {PB_21,  1, 13},
    {PB_22,  2, 14},
    {PB_23,  3, 15},
    {NC,    NC,     0}
};

typedef struct {
    mp_obj_base_t base;
    pwmout_t  obj;
    uint8_t   unit;
    pin_obj_t *pin;
} pwm_obj_t;

#endif  // OBJPWM_H_
