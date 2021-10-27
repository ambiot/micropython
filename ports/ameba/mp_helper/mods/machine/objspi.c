/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Chester Tseng
 * Copyright (c) 2020 Simon XI
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

#include "objspi.h"

#define SPI_MAX 2       //max 2 sets of SPI supported on Ameba D
static uint8_t id = 0; // default SPI idx id is 0 

spi_t mp_spi_obj[SPI_MAX];  // MBED obj 

STATIC spi_obj_t spi_obj[2] = {
    {.base.type = &spi_type, .unit = 0, .bits = 8, .baudrate = SPI_DEFAULT_BAUD_RATE, .pol = SCPOL_INACTIVE_IS_LOW, .pha = SCPH_TOGGLES_IN_MIDDLE },
    {.base.type = &spi_type, .unit = 1, .bits = 8, .baudrate = SPI_DEFAULT_BAUD_RATE, .pol = SCPOL_INACTIVE_IS_LOW, .pha = SCPH_TOGGLES_IN_MIDDLE },
};


STATIC void spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    spi_obj_t *self = self_in;
    if (id == 0) {
        mp_printf(print, "SPI(%d, baudrate=%u, bits=%d, MOSI=PB_18, MISO=PB_19, SCLK=PB_20, CS=PB_21 )", 
            self->unit, self->baudrate, self->bits);
    } else {
        mp_printf(print, "SPI(%d, baudrate=%u, bits=%d, MOSI=PB_4, MISO=PB_5, SCLK=PB_6, CS=PB_7 )", 
            self->unit, self->baudrate, self->bits);
    }
}


STATIC mp_obj_t spi_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_unit, ARG_baudrate, ARG_pol, ARG_pha, ARG_bits, ARG_firstbit, ARG_miso, ARG_mosi, ARG_sck, ARG_mode };
    const mp_arg_t spi_init_args[] = {
        { MP_QSTR_unit,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_baudrate, MP_ARG_INT,                  {.u_int = SPI_DEFAULT_BAUD_RATE} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = SCPOL_INACTIVE_IS_LOW} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = SCPH_TOGGLES_IN_MIDDLE} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = MICROPY_PY_MACHINE_SPI_MSB} },
        { MP_QSTR_miso,     MP_ARG_KW_ONLY | MP_ARG_OBJ },
        { MP_QSTR_mosi,     MP_ARG_KW_ONLY | MP_ARG_OBJ },
        { MP_QSTR_sck,      MP_ARG_KW_ONLY | MP_ARG_OBJ },
        { MP_QSTR_mode,     MP_ARG_INT                 , {.u_int = SPI_MASTER}},
    };
    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(spi_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), spi_init_args, args);

    if (args[ARG_unit].u_int > 1)
        mp_raise_ValueError("Invalid SPI unit");

    spi_obj_t *self  = &spi_obj[args[ARG_unit].u_int];
    id = args[ARG_unit].u_int;
    self->mode = args[ARG_mode].u_int;
    self->baudrate = args[ARG_baudrate].u_int;
    self->bits     = args[ARG_bits].u_int;
    
    if (args[ARG_unit].u_int == 0) {
        mp_spi_obj[id].spi_idx = MBED_SPI0;  // only MBED_SPI0 can be used as slave
        spi_init(&mp_spi_obj[id], SPI_0_MOSI, SPI_0_MISO, SPI_0_SCLK, SPI_0_CS);
    } else {
        mp_spi_obj[id].spi_idx = MBED_SPI1;
        spi_init(&mp_spi_obj[id], SPI_1_MOSI, SPI_1_MISO, SPI_1_SCLK, SPI_1_CS);
    }
    
    if (self->mode == SPI_MASTER) {
        spi_format(&mp_spi_obj[id], 8, 0, SPI_MASTER);       // 8 bits, mode 0[polarity=0,phase=0], and master-role
        spi_frequency(&mp_spi_obj[id], self->baudrate);      // default 2M baud rate
    } else {
        if (mp_spi_obj[id].spi_idx == MBED_SPI0) {
            spi_format(&mp_spi_obj[id], 8, 0, SPI_SLAVE);        // 8 bits, mode 0[polarity=0,phase=0], and slave-role
        } else {
            mp_raise_ValueError("Error: Only SPI 0 can be set as slave");
        }
    }

    return (mp_obj_t)self;
}


STATIC mp_obj_t spi_stop(mp_obj_t *self_in) {
    spi_free(&mp_spi_obj[id]);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(spi_stop_obj, spi_stop);


STATIC mp_obj_t spi_read(mp_obj_t *self_in) {
    spi_obj_t *self = self_in;
    int data = 0;

    if (self->mode == SPI_MASTER) {
        data = spi_master_write(&mp_spi_obj[id], NULL);
        return mp_obj_new_int(data);
    } else {
        data = spi_slave_read(&mp_spi_obj[id]);
        return mp_obj_new_int(data);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(spi_read_obj, spi_read);


STATIC void spi_write(mp_obj_t *self_in, mp_obj_t value_in) {
    spi_obj_t *self = self_in;
    mp_int_t value = mp_obj_get_int(value_in);

    if (self->mode == SPI_MASTER) {
        spi_master_write(&mp_spi_obj[id], value);
    } else {
        spi_slave_write(&mp_spi_obj[id], value);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(spi_write_obj, spi_write);


STATIC const mp_map_elem_t spi_locals_dict_table[] = {
    // basic SPI operations
    //{ MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&spi_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&spi_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&spi_write_obj) },
    // class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_MASTER),    MP_OBJ_NEW_SMALL_INT(SPI_MASTER) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SLAVE),     MP_OBJ_NEW_SMALL_INT(SPI_SLAVE) },
};
STATIC MP_DEFINE_CONST_DICT(spi_locals_dict, spi_locals_dict_table);


const mp_obj_type_t spi_type = {
    { &mp_type_type },
    .name        = MP_QSTR_SPI,
    .print       = spi_print,
    .make_new    = spi_make_new,
    .locals_dict = (mp_obj_dict_t *)&spi_locals_dict,
};
