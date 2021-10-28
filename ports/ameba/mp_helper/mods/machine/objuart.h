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
#ifndef OBJUART_H_
#define OBJUART_H_

#include "objpin.h"

#include "serial_api.h"

#define UART_MIN_BAUDRATE          (9600)
#define UART_MAX_BAUDRATE          (115200)

#define UART_DEFAULT_BAUDRATE       (115200)
#define UART_DEFAULT_DATA_BITS      (8)
#define UART_DEFAULT_PARITY         (0)
#define UART_DEFAULT_STOP_BITS      (1)
#define UART_DEFAULT_TX_TIMEOUT     (10)
#define UART_DEFAULT_RX_TIMEOUT     (10)

extern const mp_obj_type_t uart_type;
extern const PinMap PinMap_UART_TX[];
extern const PinMap PinMap_UART_RX[];

typedef struct {
    uint32_t        baudrate;
    uint8_t         data_bits;
    uint8_t         parity;
    uint8_t         stop_bits;
} UartParams;

typedef struct {
    uint32_t    timeout_ms;
    pin_obj_t   *pin;
} UartLane;

typedef struct {
    mp_obj_base_t base;
    serial_t  obj;
    uint8_t   unit;
    UartParams params;
    UartLane   tx;
    UartLane   rx;
    bool      irq_enabled;
    mp_obj_t  irq_handler;
} uart_obj_t;

#endif  // OBJUART_H_
