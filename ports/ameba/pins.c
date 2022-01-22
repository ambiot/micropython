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

#include "objpin.h"


#define PIN(p_pin_name, p_pull, p_mode, p_value) \
{ \
    {&pin_type },                     \
    .name   = MP_QSTR_ ## p_pin_name, \
    .id     = (p_pin_name),           \
    .pull   = (p_pull),               \
    .mode   = (p_mode),               \
    .value  = (p_value),              \
}


#define AF(pin_name, af_name, af_index, pull) \
{ \
    .pin        = pin_name,                                 \
    .peripheral = af_name ## af_index,                      \
    .function   = PIN_DATA(pull, PINMUX_FUNCTION_ ## af_name) \
}


/////////////////////////////////////////
//                                     //
//     Pin Definition for RTL8722DM    //
//                                     //
/////////////////////////////////////////
#if defined(RTL8722DM)
const PinMap PinMap_UART_TX[] = {
    AF(PA_21, UART, _0, PullUp),
    AF(PA_26, UART, _3, PullUp),
    
    {NC,    NC,     0}
};

const PinMap PinMap_UART_RX[] = {
    AF(PA_22, UART, _0, PullUp),
    AF(PA_25, UART, _3, PullUp),
    
    {NC,    NC,     0}
};


const PinMap PinMap_I2C_SDA[] = {
    AF(PA_26, I2C, _0, PullUp),
    //AF(PB_6, I2C, _0, PullUp),

    {NC,    NC,     0}
};

const PinMap PinMap_I2C_SCL[] = {
    AF(PA_25, I2C, _0, PullUp),
    //AF(PB_5, I2C, _0, PullUp),
   
    {NC,    NC,     0}
};

const PinMap PinMap_PWM[] = {
    AF(PA_23,  PWM, _2, PullNone),
    AF(PA_24, PWM, _3, PullNone),
    AF(PA_25,  PWM, _4, PullNone),
    AF(PA_26, PWM, _5, PullNone),
/*
    AF(PB_4, PWM, _8, PullNone),
    AF(PB_5, PWM, _9, PullNone),
    AF(PB_7, PWM, _17, PullNone),
    AF(PB_18, PWM, _10, PullNone),
    AF(PB_19, PWM, _11, PullNone),
    AF(PB_20,  PWM, _12, PullNone),
    AF(PB_21, PWM, _13, PullNone),
    AF(PB_22, PWM, _14, PullNone),
    AF(PB_23, PWM, _15, PullNone),
*/
    {NC,    NC,     0}
};

// BF is only for SPI, right now only support master mode
#define BF(pin_name, af_name, af_index, pull) \
{ \
    .pin        = pin_name,                                 \
    .peripheral = af_name ## af_index,                      \
    .function   = PIN_DATA(pull, PINMUX_FUNCTION_ ## af_name ## M) \
}

const PinMap PinMap_SPI_MOSI[] = {
    BF(PB_18, SPI, _0, 0),
    BF(PB_4, SPI, _1, 1),

    {NC,    NC,     0}
};

const PinMap PinMap_SPI_MISO[] = {
    BF(PB_19, SPI, _0, 0),
    BF(PB_5, SPI, _1, 1),

    {NC,    NC,     0}
};


/////////////////////////////////////////
//                                     //
//  Pin Definition for RTL8722DM_MINI  //
//                                     //
/////////////////////////////////////////
#elif defined(RTL8722DM_MINI)
const PinMap PinMap_UART_TX[] = {
    AF(PB_1, UART, _2, PullUp),
    AF(PA_21, UART, _1, PullUp),
    
    {NC,    NC,     0}
};

const PinMap PinMap_UART_RX[] = {
    AF(PB_2, UART, _2, PullUp),
    AF(PA_22, UART, _1, PullUp),

    {NC,    NC,     0}
};


const PinMap PinMap_I2C_SDA[] = {
    AF(PB_0, I2C, _0, PullUp),
    //AF(PB_6, I2C, _0, PullUp),

    {NC,    NC,     0}
};

const PinMap PinMap_I2C_SCL[] = {
    AF(PA_31, I2C, _0, PullUp),
    //AF(PB_5, I2C, _0, PullUp),
   
    {NC,    NC,     0}
};


typedef enum {
    PWM_6 = 7,
    PWM_7,
    PWM_8,
    PWM_9
} PWMName_MP;

const PinMap PinMap_PWM[] = {
    AF(PB_4, PWM, _0, PullNone),
    AF(PB_5, PWM, _1, PullNone),
    AF(PB_7, PWM, _2, PullNone),
    AF(PA_12,  PWM, _3, PullNone),
    AF(PA_13, PWM, _4, PullNone),
    AF(PA_23,  PWM, _5, PullNone),
    AF(PA_24, PWM, _6, PullNone),
    AF(PA_28,  PWM, _7, PullNone),
    AF(PA_30, PWM, _8, PullNone),
    {NC,    NC,     0}
};

// BF is only for SPI, right now only support master mode
#define BF(pin_name, af_name, af_index, pull) \
{ \
    .pin        = pin_name,                                 \
    .peripheral = af_name ## af_index,                      \
    .function   = PIN_DATA(pull, PINMUX_FUNCTION_ ## af_name ## M) \
}

const PinMap PinMap_SPI_MOSI[] = {
    BF(PA_12, SPI, _0, 0),

    {NC,    NC,     0}
};

const PinMap PinMap_SPI_MISO[] = {
    BF(PA_13, SPI, _0, 0),

    {NC,    NC,     0}
};
#else
#error "Please specify the correct board name before re-try"
#endif 


pin_obj_t pin_PA_0  = PIN(PA_0,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_1  = PIN(PA_1,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_2  = PIN(PA_2,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_3  = PIN(PA_3,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_4  = PIN(PA_4,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_5  = PIN(PA_5,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_6  = PIN(PA_6,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_7  = PIN(PA_7,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_8  = PIN(PA_8,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_9  = PIN(PA_9,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_10 = PIN(PA_10, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_11 = PIN(PA_11, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_12 = PIN(PA_12, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_13 = PIN(PA_13, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_14 = PIN(PA_14, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_15 = PIN(PA_15, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_16 = PIN(PA_16, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_17 = PIN(PA_17, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_18 = PIN(PA_18, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_19 = PIN(PA_19, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_20 = PIN(PA_20, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_21 = PIN(PA_21, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_22 = PIN(PA_22, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_23 = PIN(PA_23, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_24 = PIN(PA_24, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_25 = PIN(PA_25, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_26 = PIN(PA_26, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_27 = PIN(PA_27, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_28 = PIN(PA_28, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_29 = PIN(PA_29, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_30 = PIN(PA_30, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PA_31 = PIN(PA_31, PullNone, PIN_OUTPUT, 0);


pin_obj_t pin_PB_0  = PIN(PB_0,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_1  = PIN(PB_1,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_2  = PIN(PB_2,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_3  = PIN(PB_3,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_4  = PIN(PB_4,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_5  = PIN(PB_5,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_6  = PIN(PB_6,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_7  = PIN(PB_7,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_8  = PIN(PB_8,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_9  = PIN(PB_9,  PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_10 = PIN(PB_10, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_11 = PIN(PB_11, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_12 = PIN(PB_12, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_13 = PIN(PB_13, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_14 = PIN(PB_14, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_15 = PIN(PB_15, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_16 = PIN(PB_16, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_17 = PIN(PB_17, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_18 = PIN(PB_18, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_19 = PIN(PB_19, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_20 = PIN(PB_20, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_21 = PIN(PB_21, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_22 = PIN(PB_22, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_23 = PIN(PB_23, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_24 = PIN(PB_24, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_25 = PIN(PB_25, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_26 = PIN(PB_26, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_27 = PIN(PB_27, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_28 = PIN(PB_28, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_29 = PIN(PB_29, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_30 = PIN(PB_30, PullNone, PIN_OUTPUT, 0);
pin_obj_t pin_PB_31 = PIN(PB_31, PullNone, PIN_OUTPUT, 0);



STATIC const mp_map_elem_t pin_board_pins_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_0), MP_OBJ_FROM_PTR(&pin_PA_0)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_1), MP_OBJ_FROM_PTR(&pin_PA_1)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_2), MP_OBJ_FROM_PTR(&pin_PA_2)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_3), MP_OBJ_FROM_PTR(&pin_PA_3)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_4), MP_OBJ_FROM_PTR(&pin_PA_4)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_5), MP_OBJ_FROM_PTR(&pin_PA_5)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_6), MP_OBJ_FROM_PTR(&pin_PA_6)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_7), MP_OBJ_FROM_PTR(&pin_PA_7)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_8), MP_OBJ_FROM_PTR(&pin_PA_8)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_9), MP_OBJ_FROM_PTR(&pin_PA_9)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_10), MP_OBJ_FROM_PTR(&pin_PA_10)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_11), MP_OBJ_FROM_PTR(&pin_PA_11)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_12), MP_OBJ_FROM_PTR(&pin_PA_12)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_13), MP_OBJ_FROM_PTR(&pin_PA_13)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_14), MP_OBJ_FROM_PTR(&pin_PA_14)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_15), MP_OBJ_FROM_PTR(&pin_PA_15)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_16), MP_OBJ_FROM_PTR(&pin_PA_16)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_17), MP_OBJ_FROM_PTR(&pin_PA_17)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_18), MP_OBJ_FROM_PTR(&pin_PA_18)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_19), MP_OBJ_FROM_PTR(&pin_PA_19)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_20), MP_OBJ_FROM_PTR(&pin_PA_20)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_21), MP_OBJ_FROM_PTR(&pin_PA_21)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_22), MP_OBJ_FROM_PTR(&pin_PA_22)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_23), MP_OBJ_FROM_PTR(&pin_PA_23)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_24), MP_OBJ_FROM_PTR(&pin_PA_24)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_25), MP_OBJ_FROM_PTR(&pin_PA_25)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_26), MP_OBJ_FROM_PTR(&pin_PA_26)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_27), MP_OBJ_FROM_PTR(&pin_PA_27)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_28), MP_OBJ_FROM_PTR(&pin_PA_28)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_29), MP_OBJ_FROM_PTR(&pin_PA_29)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_30), MP_OBJ_FROM_PTR(&pin_PA_30)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PA_31), MP_OBJ_FROM_PTR(&pin_PA_31)},

    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_0), MP_OBJ_FROM_PTR(&pin_PB_0)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_1), MP_OBJ_FROM_PTR(&pin_PB_1)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_2), MP_OBJ_FROM_PTR(&pin_PB_2)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_3), MP_OBJ_FROM_PTR(&pin_PB_3)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_4), MP_OBJ_FROM_PTR(&pin_PB_4)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_5), MP_OBJ_FROM_PTR(&pin_PB_5)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_6), MP_OBJ_FROM_PTR(&pin_PB_6)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_7), MP_OBJ_FROM_PTR(&pin_PB_7)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_8), MP_OBJ_FROM_PTR(&pin_PB_8)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_9), MP_OBJ_FROM_PTR(&pin_PB_9)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_10), MP_OBJ_FROM_PTR(&pin_PB_10)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_11), MP_OBJ_FROM_PTR(&pin_PB_11)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_12), MP_OBJ_FROM_PTR(&pin_PB_12)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_13), MP_OBJ_FROM_PTR(&pin_PB_13)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_14), MP_OBJ_FROM_PTR(&pin_PB_14)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_15), MP_OBJ_FROM_PTR(&pin_PB_15)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_16), MP_OBJ_FROM_PTR(&pin_PB_16)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_17), MP_OBJ_FROM_PTR(&pin_PB_17)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_18), MP_OBJ_FROM_PTR(&pin_PB_18)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_19), MP_OBJ_FROM_PTR(&pin_PB_19)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_20), MP_OBJ_FROM_PTR(&pin_PB_20)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_21), MP_OBJ_FROM_PTR(&pin_PB_21)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_22), MP_OBJ_FROM_PTR(&pin_PB_22)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_23), MP_OBJ_FROM_PTR(&pin_PB_23)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_24), MP_OBJ_FROM_PTR(&pin_PB_24)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_25), MP_OBJ_FROM_PTR(&pin_PB_25)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_26), MP_OBJ_FROM_PTR(&pin_PB_26)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_27), MP_OBJ_FROM_PTR(&pin_PB_27)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_28), MP_OBJ_FROM_PTR(&pin_PB_28)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_29), MP_OBJ_FROM_PTR(&pin_PB_29)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_30), MP_OBJ_FROM_PTR(&pin_PB_30)},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PB_31), MP_OBJ_FROM_PTR(&pin_PB_31)},
};
MP_DEFINE_CONST_DICT(pin_board_pins_locals_dict, pin_board_pins_locals_dict_table);
