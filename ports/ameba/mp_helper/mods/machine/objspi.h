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
#ifndef OBJSPI_H_
#define OBJSPI_H_

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "objpin.h"

#include "PinNames.h"
#include "spi_api.h"
#include "spi_ex_api.h"

#define SPI_DEFAULT_BAUD_RATE           (200000)

#define SPI_MASTER                      0
#define SPI_SLAVE                       1

#ifndef MICROPY_PY_MACHINE_SPI_MSB
#define MICROPY_PY_MACHINE_SPI_MSB      (0)
#define MICROPY_PY_MACHINE_SPI_LSB      (1)
#endif

extern const mp_obj_type_t spi_type;

extern const PinMap PinMap_SPI_MOSI[];
extern const PinMap PinMap_SPI_MISO[];


#if defined(RTL8722DM)
// SPI0 (S1)
#define SPI_0_MOSI  PB_18
#define SPI_0_MISO  PB_19
#define SPI_0_SCLK  PB_20
#define SPI_0_CS    PB_21

// SPI1 (S1)
#define SPI_1_MOSI  PB_4
#define SPI_1_MISO  PB_5
#define SPI_1_SCLK  PB_6
#define SPI_1_CS    PB_7

#elif defined(RTL8722DM_MINI)
#define SPI_0_MOSI  PB_4
#define SPI_0_MISO  PB_5
#define SPI_0_SCLK  PB_6
#define SPI_0_CS    PB_7

#define SPI_1_MOSI  PA_12
#define SPI_1_MISO  PA_13
#define SPI_1_SCLK  PA_14
#define SPI_1_CS    PA_15



#else
#error "Please specify the correct board name before re-try"
#endif 

typedef struct {
    mp_obj_base_t base;
    uint8_t   unit;
    uint8_t   bits;
    uint32_t  baudrate;
    uint8_t   pol;
    uint8_t   pha;
    pin_obj_t *sck;
    pin_obj_t *miso;
    pin_obj_t *mosi;
    uint8_t   ssi_idx;
    uint8_t   ssi_pinmux;
    uint8_t   mode;
} spi_obj_t;

#endif  // OBJSPI_H_
