#include "objsdcard.h"
#include "extmod/vfs.h"
#include "sd.h" // sd card driver with sdio interface

#define BLOCK_SIZE   (512)


STATIC sdcard_obj_t sdcard_obj[1] = {
    {
        .base.type = &sdcard_type, 
        .block_size = 0,
        .start = 0,
        .len = 0,
    }
};


STATIC int interpret_sd_status(SD_RESULT result){
	int ret = 0;
	if(result == SD_OK)
		ret = 0;
	else if(result == SD_NODISK)
		ret = -ENODEV;
	else if(result == SD_INSERT)
		ret = -EBUSY;
	else if(result == SD_INITERR)
		ret = -EIO;
	else if(result == SD_PROTECTED)
		ret = -EROFS;

	return ret;
}

STATIC int interpret_sd_result(SD_RESULT result){
	int ret = 0;
	if(result == SD_OK)
		ret = 0;
	else if(result == SD_PROTECTED)
		ret = -EROFS;
	else if(result == SD_ERROR)
		ret = -EIO;
	return ret;
}

mp_obj_t sdcard_readblocks(size_t n_args, const mp_obj_t *args) {
    sdcard_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t block_num = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);

    if (n_args == 4) {
        uint32_t offset = mp_obj_get_int(args[3]);
        if (offset) {
            mp_raise_ValueError(MP_ERROR_TEXT("offset addressing not supported"));
        }
    }

    byte *buf = bufinfo.buf;

    int res = interpret_sd_result(SD_ReadBlocks(block_num, buf, bufinfo.len / self->block_size));
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(sdcard_readblocks_obj, 3, 4, sdcard_readblocks);

mp_obj_t sdcard_writeblocks(size_t n_args, const mp_obj_t *args) {
    sdcard_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t block_num = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);

    if (n_args == 4) {
        uint32_t offset = mp_obj_get_int(args[3]);
        if (offset) {
            mp_raise_ValueError(MP_ERROR_TEXT("offset addressing not supported"));
        }
    }

    int res = interpret_sd_result(SD_WriteBlocks(block_num, (uint8_t *)bufinfo.buf, bufinfo.len / self->block_size));
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(sdcard_writeblocks_obj, 3, 4, sdcard_writeblocks);


mp_obj_t sdcard_ioctl(mp_obj_t self_in, mp_obj_t op_in, mp_obj_t arg_in) {
    sdcard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t op = mp_obj_get_int(op_in);
    int ret = 0;
    switch (op) {
        case MP_BLOCKDEV_IOCTL_INIT: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }

        case MP_BLOCKDEV_IOCTL_DEINIT: {
            ret = interpret_sd_status(SD_DeInit());
            return MP_OBJ_NEW_SMALL_INT(ret);
        }

        case MP_BLOCKDEV_IOCTL_SYNC: {
            // ret = interpret_sd_result(SD_WaitReady());
            return MP_OBJ_NEW_SMALL_INT(ret);
        }

        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT: {
            return MP_OBJ_NEW_SMALL_INT(self->len / self->block_size);
        }

        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE: {
            return MP_OBJ_NEW_SMALL_INT(self->block_size);
        }

        case MP_BLOCKDEV_IOCTL_BLOCK_ERASE: {
            // mp_int_t block_num = mp_obj_get_int(arg_in);
            // SD_disk_Driver.ioctl(CTRL_ERASE_SECTOR, block_num);
            return MP_OBJ_NEW_SMALL_INT(0);
        }

        default:
            return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(sdcard_ioctl_obj, sdcard_ioctl);

STATIC const mp_rom_map_elem_t sdcard_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&sdcard_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&sdcard_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&sdcard_ioctl_obj) },
};
STATIC MP_DEFINE_CONST_DICT(sdcard_locals_dict, sdcard_locals_dict_table);

STATIC void sdcard_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    sdcard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "SDCard(start=0x%08x, len=%u)", self->start, self->len);
}

STATIC mp_obj_t sdcard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // Parse arguments
    enum { ARG_start, ARG_len };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_start, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_len,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[ARG_start].u_int == -1 && args[ARG_len].u_int == -1) {
        return MP_OBJ_FROM_PTR(&sdcard_obj);
    }

    sdcard_obj_t *self = m_new_obj(sdcard_obj_t);
    self->base.type = &sdcard_type;

    int error = interpret_sd_status(SD_Init());
    if (error) {
        mp_raise_ValueError(MP_ERROR_TEXT("SD Init failed"));
    }

    mp_int_t start = args[ARG_start].u_int;
    mp_int_t len = args[ARG_len].u_int;

    self->block_size = BLOCK_SIZE;
    
    if (len == -1) {
        uint32_t sectors = 0;
        SD_GetCapacity((u32*) &sectors);
        if (!sectors) {
            mp_raise_ValueError(MP_ERROR_TEXT("Could not query SD card capacity"));
        } else {
            len = sectors * self->block_size;
        }
    }
    
    if (start % BLOCK_SIZE != 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("start address must be divisible by block size"));
    }
    if ((len == -1) || (len % self->block_size != 0)) {
        mp_raise_ValueError(MP_ERROR_TEXT("length must be divisible by block size"));
    }

    self->start = start;
    self->len = len;

    return MP_OBJ_FROM_PTR(self);
}


const mp_obj_type_t sdcard_type = {
    { &mp_type_type },
    .name = MP_QSTR_SDCard,
    .print = sdcard_print,
    .make_new = sdcard_make_new,
    .locals_dict = (mp_obj_t)&sdcard_locals_dict,
};
