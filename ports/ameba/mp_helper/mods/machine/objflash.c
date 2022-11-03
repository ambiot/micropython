/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Chester Tseng
 * Copyright (c) 2021 Huang Yue
 * Copyright (c) 2022 Simon Xi 
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

#include "objflash.h"
#include "integer.h"
#include "stdint.h"
#include "diskio.h"
#include "stdio.h"
#include "device_lock.h"
#include "platform_opts.h"
#include "extmod/vfs.h"


#define FLASH_BLOCK_SIZE    512     
#define FLASH_SECTOR_COUNT  128     // 128 * 4KB = 512KB
#define SECTOR_SIZE_FLASH   4096    // 4KB

#define FLASH_START_OFFSET  0x106000    // offset to SPI_FLASH_BASE (0x0800 0000), equals to 0x0810 6000(KM0 IMG2 OTA2)
#define FLASH_FS_LEN        SECTOR_SIZE_FLASH * FLASH_SECTOR_COUNT  // by default 512KB for Flash_FatFS (at most 999KB can be allocated to Flash FatFS)


/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/
static flash_t amb_flash;
const mp_obj_type_t flash_type;


STATIC flash_obj_t amb_flash_obj = {
        .base = { &flash_type },
        .block_size = SECTOR_SIZE_FLASH,
        .start = FLASH_START_OFFSET,
        .len = FLASH_FS_LEN,
};


STATIC void amb_flash_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    flash_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "FLASH(start=0x%08x, len=%u)", self->start, self->len);
}


int interpret_flash_result(int out){
    int res;
    if(out)
        res = RES_OK;
    else 
        res = RES_ERROR;
    return res;
}

/* Read block(s) --------------------------------------------*/ 
mp_obj_t amb_flash_readblocks(size_t n_args, const mp_obj_t *args) {  
    flash_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t block_num = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);
 
    if (n_args == 4) {
        uint32_t offset = mp_obj_get_int(args[3]);
        if (offset) {
            mp_raise_ValueError(MP_ERROR_TEXT("offset addressing not supported"));
        }
    }

    int res;
    char retry_cnt = 0, i = 0;

    int count = bufinfo.len / self->block_size;

    // Must enter critical section before manipulate flash
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    do{
        res = interpret_flash_result(flash_stream_read(&(self->obj), FLASH_START_OFFSET + block_num * SECTOR_SIZE_FLASH, count*SECTOR_SIZE_FLASH, (uint8_t *) bufinfo.buf));
        if(++retry_cnt>=3)
            break;
    } while (res != RES_OK);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(amb_flash_readblocks_obj, 3, 4, amb_flash_readblocks);




/* Write block(s) --------------------------------------------*/
mp_obj_t amb_flash_writeblocks(size_t n_args, const mp_obj_t *args) {
//    printf("n_args[writeblocks] = %d\r\n", n_args);
    flash_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t block_num = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);

    if (n_args == 4) {
        uint32_t offset = mp_obj_get_int(args[3]);
        if (offset) {
            mp_raise_ValueError(MP_ERROR_TEXT("offset addressing not supported"));
        }
    }

    int res;
    char retry_cnt = 0, i = 0;

    int count = bufinfo.len / self->block_size;

    // must enter critical section before manipulate flash
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    do{
        for(i=0;i<count;i++){            
            uint32_t erase_addr = FLASH_START_OFFSET + (block_num + i)*SECTOR_SIZE_FLASH;

            flash_erase_sector(&(self->obj), erase_addr);
            res = interpret_flash_result(flash_stream_write(&(self->obj), FLASH_START_OFFSET + (block_num + i)*SECTOR_SIZE_FLASH, count*SECTOR_SIZE_FLASH, (uint8_t *) bufinfo.buf));
        }
        if(++retry_cnt>=3)
            break;
        
    } while (res != RES_OK);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);

    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(amb_flash_writeblocks_obj, 3, 4, amb_flash_writeblocks);


/* IOCTL blocks(s) --------------------------------------------*/
STATIC mp_obj_t amb_flash_ioctl (mp_obj_t self_in, mp_obj_t cmd_in, mp_obj_t arg_in){
    flash_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    
    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        case MP_BLOCKDEV_IOCTL_DEINIT: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        case MP_BLOCKDEV_IOCTL_SYNC: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        //Get sector count 
        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT: {
            return MP_OBJ_NEW_SMALL_INT(FLASH_SECTOR_COUNT); // 128
        }
        //Get sector size
        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE: {
            return MP_OBJ_NEW_SMALL_INT(SECTOR_SIZE_FLASH); // 4096(4KB)
        }
        case MP_BLOCKDEV_IOCTL_BLOCK_ERASE: {
            uint32_t block_num = mp_obj_get_int(arg_in);
            for( int i = block_num ; i > 0; i--) {
                flash_erase_block(&(self->obj), self->start);
            }
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        default: {
            return mp_const_none;
        }
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(amb_flash_ioctl_obj, amb_flash_ioctl);


STATIC mp_obj_t amb_flash_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // Check args.
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    amb_flash_obj.obj = amb_flash;

    // Return singleton object.
    return MP_OBJ_FROM_PTR(&amb_flash_obj);
}


STATIC const mp_rom_map_elem_t amb_flash_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&amb_flash_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&amb_flash_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&amb_flash_ioctl_obj) },
};
STATIC MP_DEFINE_CONST_DICT(amb_flash_locals_dict, amb_flash_locals_dict_table);


const mp_obj_type_t flash_type = {
    { &mp_type_type },
    .name        = MP_QSTR_FLASH,
    .print       = amb_flash_print,
    .make_new    = amb_flash_make_new,
    .locals_dict = (mp_obj_dict_t *)&amb_flash_locals_dict,
};
