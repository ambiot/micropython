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

#include "obji2c.h"


i2c_t i2cwire0;
//i2c_t i2cwire1;


STATIC i2c_obj_t i2c_obj[2] = {
    {.base.type = &i2c_type, .unit = 0, .freq = I2C_DEFAULT_BAUD_RATE_HZ },
    {.base.type = &i2c_type, .unit = 1, .freq = I2C_DEFAULT_BAUD_RATE_HZ },
};

int _i2c_read(mp_obj_base_t *self_in, uint8_t *dest, size_t len, bool nack) {
  i2c_obj_t *self = (i2c_obj_t*)self_in;
  mp_uint_t i = 0;
  for (i = 0; i < len; i++) {
    dest[i] = i2c_byte_read(&i2cwire0, nack && (i == len));
  }
  return 0;
}

int _i2c_write(mp_obj_base_t *self_in, const uint8_t *src, size_t len) {
  i2c_obj_t *self = (i2c_obj_t*)self_in;
  int num_acks = 0;
  while (len--) {
    int ret = i2c_byte_write(&i2cwire0, *src++);
    if (ret != true)
      return 0;
    ++num_acks;
  }
  return num_acks;
}

// return value:
//    0 - success
//   <0 - error, with errno being the negative of the return value
int _i2c_readfrom(mp_obj_base_t *self_in, uint16_t addr, uint8_t *dest, size_t len, bool stop) {
  i2c_obj_t *self = (i2c_obj_t*)self_in;
  int ret = i2c_read(&i2cwire0, addr, dest, len, stop);
  if (ret != len)
    return -1;
  return 0; // success
}

// return value:
//  >=0 - number of acks received
//   <0 - error, with errno being the negative of the return value
int _i2c_writeto(mp_obj_base_t *self_in, uint16_t addr, const uint8_t *src, size_t len, bool stop) {
  i2c_obj_t *self = (i2c_obj_t*)self_in;
  int ret = i2c_write(&i2cwire0, addr, src, len, stop);
  return ret;
}

///////////////////////  XXM  ///////////////////////
// Replaced all i2c_p->readfrom with _i2c_readfrom //
// Replaced all i2c_p->writeto with _i2c_writeto   //
// due to snytax error                             //
/////////////////////////////////////////////////////

STATIC const mp_machine_i2c_p_t machine_hard_i2c_p = {
    .read = _i2c_read,
    .write = _i2c_write,
    //.readfrom = _i2c_readfrom,  // xxm
    //.writeto = _i2c_writeto,  // xxm
};

STATIC mp_obj_t i2c_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    enum {ARG_unit, ARG_sda, ARG_scl, ARG_freq};
    const mp_arg_t i2c_init_args[] = {
        { MP_QSTR_unit, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_sda,  MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_scl,  MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = I2C_DEFAULT_BAUD_RATE_HZ} },
    };

    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(i2c_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), i2c_init_args, args);

    pin_obj_t *sda = pin_find(args[ARG_sda].u_obj);
    pin_obj_t *scl = pin_find(args[ARG_scl].u_obj);

    // Use Realtek's api to validate pins
    PinName pn_sda = (PinName)pinmap_peripheral(sda->id, PinMap_I2C_SDA);
    PinName pn_scl = (PinName)pinmap_peripheral(scl->id, PinMap_I2C_SCL);

    if (pn_sda == NC)
        mp_raise_ValueError("I2C SDA pin not match");

    if (pn_scl == NC)
        mp_raise_ValueError("I2C SCL pin not match");
    
    i2c_obj_t *self = &i2c_obj[args[ARG_unit].u_int];
    self->freq = MIN(MAX(args[ARG_freq].u_int, I2C_MIN_BAUD_RATE_HZ), I2C_MAX_BAUD_RATE_HZ);
    self->scl  = scl;
    self->sda  = sda;
    
    i2c_init(&i2cwire0, self->sda->id, self->scl->id);

    i2c_frequency(&i2cwire0, self->freq);

    return (mp_obj_t)self;
}

STATIC void i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    i2c_obj_t *self = self_in;
    mp_printf(print, "I2C(%d, freq=%u, SCL=%q, SDA=%q)", self->unit, self->freq, self->scl->name, self->sda->name);
}

STATIC mp_obj_t mp_i2c_reset(mp_obj_t self_in) {
    i2c_obj_t *self = self_in;
    i2c_reset(&i2cwire0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(i2c_reset_obj, mp_i2c_reset);

STATIC mp_obj_t machine_i2c_scan(mp_obj_t self_in) {
    mp_obj_base_t *self = MP_OBJ_TO_PTR(self_in);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    mp_obj_t list = mp_obj_new_list(0, NULL);
    // 7-bit addresses 0b0000xxx and 0b1111xxx are reserved
    for (int addr = 0x08; addr < 0x78; ++addr) {
        int ret = _i2c_writeto(self, addr, NULL, 0, true);
        if (ret == 0) {
            mp_obj_list_append(list, MP_OBJ_NEW_SMALL_INT(addr));
        }
    }
    return list;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_scan_obj, machine_i2c_scan);

STATIC mp_obj_t machine_i2c_readinto(size_t n_args, const mp_obj_t *args) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    if (i2c_p->read == NULL) {
        mp_raise_msg(&mp_type_OSError, "I2C operation not supported");
    }

    // get the buffer to read into
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);

    // work out if we want to send a nack at the end
    bool nack = (n_args == 2) ? true : mp_obj_is_true(args[2]);

    // do the read
    int ret = i2c_p->read(self, bufinfo.buf, bufinfo.len, nack);
    if (ret != 0) {
        mp_raise_OSError(-ret);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readinto_obj, 2, 3, machine_i2c_readinto);

STATIC mp_obj_t machine_i2c_write(mp_obj_t self_in, mp_obj_t buf_in) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(self_in);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    if (i2c_p->write == NULL) {
        mp_raise_msg(&mp_type_OSError, "I2C operation not supported");
    }

    // get the buffer to write from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);

    // do the write
    int ret = i2c_p->write(self, bufinfo.buf, bufinfo.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    // return number of acks received
    return MP_OBJ_NEW_SMALL_INT(ret);
}
MP_DEFINE_CONST_FUN_OBJ_2(machine_i2c_write_obj, machine_i2c_write);

STATIC mp_obj_t machine_i2c_readfrom(size_t n_args, const mp_obj_t *args) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    mp_int_t addr = mp_obj_get_int(args[1]);
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[2]));
    bool stop = (n_args == 3) ? true : mp_obj_is_true(args[3]);
    int ret = _i2c_readfrom(self, addr, (uint8_t*)vstr.buf, vstr.len, stop);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readfrom_obj, 3, 4, machine_i2c_readfrom);

STATIC mp_obj_t machine_i2c_readfrom_into(size_t n_args, const mp_obj_t *args) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);
    bool stop = (n_args == 3) ? true : mp_obj_is_true(args[3]);
    int ret = _i2c_readfrom(self, addr, bufinfo.buf, bufinfo.len, stop);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_readfrom_into_obj, 3, 4, machine_i2c_readfrom_into);

STATIC mp_obj_t machine_i2c_writeto(size_t n_args, const mp_obj_t *args) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(args[0]);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    
    mp_int_t addr = mp_obj_get_int(args[1]);
    
    //mp_buffer_info_t bufinfo;
    //mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
    
    int int_in = mp_obj_get_int(args[2]);

    bool stop = (n_args == 3) ? true : mp_obj_is_true(args[3]);

    //int ret = _i2c_writeto(self, addr,  bufinfo.buf, bufinfo.len, stop); 
    int ret = _i2c_writeto(self, addr, (const uint8_t *)&int_in, 1, stop); 

    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    // return number of acks received
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_writeto_obj, 3, 4, machine_i2c_writeto);

STATIC int read_mem(mp_obj_t self_in, uint16_t addr, uint32_t memaddr, uint8_t addrsize, uint8_t *buf, size_t len) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(self_in);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;
    uint8_t memaddr_buf[4];
    size_t memaddr_len = 0;
    for (int16_t i = addrsize - 8; i >= 0; i -= 8) {
        memaddr_buf[memaddr_len++] = memaddr >> i;
    }
    int ret = _i2c_writeto(self, addr, memaddr_buf, memaddr_len, false);
    if (ret != memaddr_len) {
        // must generate STOP
        _i2c_writeto(self, addr, NULL, 0, true);
        return ret;
    }
    return _i2c_readfrom(self, addr, buf, len, true);
}

#define MAX_MEMADDR_SIZE (4)
#define BUF_STACK_SIZE (12)

STATIC int write_mem(mp_obj_t self_in, uint16_t addr, uint32_t memaddr, uint8_t addrsize, const uint8_t *buf, size_t len) {
    mp_obj_base_t *self = (mp_obj_base_t*)MP_OBJ_TO_PTR(self_in);
    mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t*)self->type->protocol;

    // need some memory to create the buffer to send; try to use stack if possible
    uint8_t buf2_stack[MAX_MEMADDR_SIZE + BUF_STACK_SIZE];
    uint8_t *buf2;
    size_t buf2_alloc = 0;
    if (len <= BUF_STACK_SIZE) {
        buf2 = buf2_stack;
    } else {
        buf2_alloc = MAX_MEMADDR_SIZE + len;
        buf2 = m_new(uint8_t, buf2_alloc);
    }

    // create the buffer to send
    size_t memaddr_len = 0;
    for (int16_t i = addrsize - 8; i >= 0; i -= 8) {
        buf2[memaddr_len++] = memaddr >> i;
    }
    memcpy(buf2 + memaddr_len, buf, len);

    int ret = _i2c_writeto(self, addr, buf2, memaddr_len + len, true);
    if (buf2_alloc != 0) {
        m_del(uint8_t, buf2, buf2_alloc);
    }
    return ret;
}

STATIC const mp_arg_t machine_i2c_mem_allowed_args[] = {
    { MP_QSTR_addr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_memaddr, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_arg,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_addrsize, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
};

STATIC mp_obj_t machine_i2c_readfrom_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_n, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);

    // create the buffer to store data into
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[ARG_n].u_obj));

    // do the transfer
    int ret = read_mem(pos_args[0], args[ARG_addr].u_int, args[ARG_memaddr].u_int,
        args[ARG_addrsize].u_int, (uint8_t*)vstr.buf, vstr.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_readfrom_mem_obj, 1, machine_i2c_readfrom_mem);


STATIC mp_obj_t machine_i2c_readfrom_mem_into(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_buf, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);

    // get the buffer to store data into
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_WRITE);

    // do the transfer
    int ret = read_mem(pos_args[0], args[ARG_addr].u_int, args[ARG_memaddr].u_int,
        args[ARG_addrsize].u_int, bufinfo.buf, bufinfo.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_readfrom_mem_into_obj, 1, machine_i2c_readfrom_mem_into);

STATIC mp_obj_t machine_i2c_writeto_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_buf, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);

    // get the buffer to write the data from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_READ);

    // do the transfer
    int ret = write_mem(pos_args[0], args[ARG_addr].u_int, args[ARG_memaddr].u_int,
        args[ARG_addrsize].u_int, bufinfo.buf, bufinfo.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_writeto_mem_obj, 1, machine_i2c_writeto_mem);

STATIC const mp_map_elem_t i2c_locals_dict_table[] = {
    //{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&i2c_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&machine_i2c_scan_obj) },

    // primitive I2C operations
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&machine_i2c_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_i2c_write_obj) },

    // standard bus operations
    { MP_ROM_QSTR(MP_QSTR_readfrom), MP_ROM_PTR(&machine_i2c_readfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_into), MP_ROM_PTR(&machine_i2c_readfrom_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto), MP_ROM_PTR(&machine_i2c_writeto_obj) },

    // memory operations
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem), MP_ROM_PTR(&machine_i2c_readfrom_mem_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem_into), MP_ROM_PTR(&machine_i2c_readfrom_mem_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto_mem), MP_ROM_PTR(&machine_i2c_writeto_mem_obj) },
};
STATIC MP_DEFINE_CONST_DICT(i2c_locals_dict, i2c_locals_dict_table);



const mp_obj_type_t i2c_type = {
    { &mp_type_type },
    .name        = MP_QSTR_I2C,
    .print       = i2c_print,
    .make_new    = i2c_make_new,
    .protocol    = &machine_hard_i2c_p,
    .locals_dict = (mp_obj_dict_t *)&i2c_locals_dict,
};
