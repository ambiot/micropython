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
#ifndef OBJPIN_H_
#define OBJPIN_H_

#include "py/obj.h"

#include "gpio_api.h"
#include "PinNames.h"
#include "pinmap.h"
#include "extmod/virtpin.h"

extern const mp_obj_type_t pin_type;
extern const mp_obj_dict_t pin_board_pins_locals_dict;

enum {
    PIN_FN_UART = 0,
    PIN_FN_SPI,
    PIN_FN_I2C,
    PIN_FN_I2S,
    PIN_FN_PCM,
    PIN_FN_ADC,
    PIN_FN_DAC,
    PIN_FN_PINIRQ,
};

enum {
    PIN_TYPE_UART_TX = 0,
    PIN_TYPE_UART_RX,
    PIN_TYPE_UART_RTS,
    PIN_TYPE_UART_CTS,
};

enum {
    PIN_TYPE_SPI_CLK = 0,
    PIN_TYPE_SPI_MOSI,
    PIN_TYPE_SPI_MISO,
    PIN_TYPE_SPI_CS0,
    PIN_TYPE_SPI_CS1,
    PIN_TYPE_SPI_CS2,
};

enum {
    PIN_TYPE_I2C_SDA = 0,
    PIN_TYPE_I2C_SCL,
};

enum {
    PIN_TYPE_I2S_WS=0,
    PIN_TYPE_I2S_CLK,
    PIN_TYPE_I2S_SD_TX,
    PIN_TYPE_I2S_MCK,
    PIN_TYPE_I2S_SD_RX,
};

enum {
    PIN_TYPE_PCM_SYNC=0,
    PIN_TYPE_PCM_CLK,
    PIN_TYPE_PCM_TX,
    PIN_TYPE_PCM_RX,
};

typedef struct {
    const mp_obj_base_t base;
    const qstr          name;
    gpio_t              obj;
    uint8_t             id;
    uint16_t            pull;
    uint8_t             mode;         
    uint8_t             value;
    const PinMap        *af;
} pin_obj_t;

typedef struct {
    const char *name;
    const pin_obj_t *pin;
} pin_named_pin_t;

typedef struct {
    mp_obj_base_t base;
    qstr name;
    const pin_named_pin_t *named_pins;
} pin_named_pins_obj_t;

#endif  // OBJPIN_H_
