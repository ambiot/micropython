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

/*****************************************************************************
 *                              Header includes
 * ***************************************************************************/
#include "objcrypto.h"

#define CRYPTO_ENCRYPT      (0)
#define CRYPTO_DECRYPT      (1)

STATIC crypto_obj_t crypto_obj = {
    .base.type = &crypto_type,
};

void crypto_init0(void) {
    const char *error_str = "Crypto engine init failed\r\n";
    if (rtl_cryptoEngine_init() != 0)
        mp_printf(&mp_plat_print, "Crypto engine init failed\r\n");
}

STATIC void crypto_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    crypto_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "CRYPTO()");
}

STATIC mp_obj_t crypto_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return constant object
    return MP_OBJ_FROM_PTR(&crypto_obj);
}

STATIC mp_obj_t crypto_md5(mp_obj_t self_in, mp_obj_t array_in) {

    int8_t retVal;

    mp_buffer_info_t bufinfo_read;
    mp_get_buffer_raise(array_in, &bufinfo_read, MP_BUFFER_READ);

    uint8_t *dest = m_new(uint8_t, CRYPTO_MD5_DIGEST_LENGTH);

    if (retVal = rtl_crypto_md5(bufinfo_read.buf,
                    bufinfo_read.len, dest) < 0) {
        m_del(uint8_t, dest, CRYPTO_MD5_DIGEST_LENGTH);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception,
                    "CRYPTO engine (md5) process failed [%u]", retVal));
    }
    return mp_obj_new_bytearray_by_ref(CRYPTO_MD5_DIGEST_LENGTH, dest);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(crypto_md5_object, crypto_md5);

STATIC mp_obj_t crypto_sha1(mp_obj_t self_in, mp_obj_t array_in) {

    int8_t retVal;

    mp_buffer_info_t bufinfo_read;
    mp_get_buffer_raise(array_in, &bufinfo_read, MP_BUFFER_READ);

    uint8_t *dest = m_new(uint8_t, CRYPTO_SHA1_DIGEST_LENGTH);

    if (retVal = rtl_crypto_sha1(bufinfo_read.buf,
                    bufinfo_read.len, dest) < 0) {
        m_del(uint8_t, dest, CRYPTO_SHA1_DIGEST_LENGTH);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception,
                    "CRYPTO engine (sha1) process failed [%u]", retVal));
    }
    return mp_obj_new_bytearray_by_ref(CRYPTO_SHA1_DIGEST_LENGTH, dest);
}
STATIC  MP_DEFINE_CONST_FUN_OBJ_2(crypto_sha1_object, crypto_sha1);

STATIC mp_obj_t crypto_sha2(mp_obj_t self_in, mp_obj_t type_in, mp_obj_t array_in) {

    int8_t retVal;
    uint8_t sha2_type;

    sha2_type = mp_obj_get_int(type_in);
    
    if ((sha2_type != SHA2_NONE) &&
        (sha2_type != SHA2_224) &&
        (sha2_type != SHA2_256) &&
        (sha2_type != SHA2_384) &&
        (sha2_type != SHA2_512)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError,
                    "Invalid SHA2 type"));
    }

    mp_buffer_info_t bufinfo_read;
    mp_get_buffer_raise(array_in, &bufinfo_read, MP_BUFFER_READ);

    uint8_t *dest = m_new(uint8_t, CRYPTO_SHA2_DIGEST_LENGTH);

    if (retVal = rtl_crypto_sha2(sha2_type, bufinfo_read.buf,
                    bufinfo_read.len, dest) < 0) {
        m_del(uint8_t, dest, CRYPTO_SHA2_DIGEST_LENGTH);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception,
                    "CRYPTO engine (sha2) process failed [%u]", retVal));
    }
    return mp_obj_new_bytearray_by_ref(CRYPTO_SHA2_DIGEST_LENGTH, dest);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(crypto_sha2_object, crypto_sha2);

STATIC mp_obj_t crypto_hmac_md5(mp_obj_t self_in, mp_obj_t array_in, mp_obj_t key_in) {

    int8_t retVal;

    mp_buffer_info_t bufinfo_read;
    mp_buffer_info_t bufinfo_key;
    mp_get_buffer_raise(array_in, &bufinfo_read, MP_BUFFER_READ);
    mp_get_buffer_raise(key_in, &bufinfo_key, MP_BUFFER_READ);

    uint8_t *dest = m_new(uint8_t, CRYPTO_MD5_DIGEST_LENGTH);

    if (retVal = rtl_crypto_hmac_md5(bufinfo_read.buf, bufinfo_read.len,
                        bufinfo_key.buf, bufinfo_key.len,
                        dest) < 0) {
        m_del(uint8_t, dest, CRYPTO_MD5_DIGEST_LENGTH);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception,
                    "CRYPTO engine (hmac_md5) process failed [%u]", retVal));
    }
    return mp_obj_new_bytearray_by_ref(CRYPTO_MD5_DIGEST_LENGTH, dest);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(crypto_hmac_md5_object, crypto_hmac_md5);

STATIC mp_obj_t crypto_hmac_sha1(mp_obj_t self_in, mp_obj_t array_in, mp_obj_t key_in) {
    int8_t retVal;

    mp_buffer_info_t bufinfo_read;
    mp_buffer_info_t bufinfo_key;
    mp_get_buffer_raise(array_in, &bufinfo_read, MP_BUFFER_READ);
    mp_get_buffer_raise(key_in, &bufinfo_key, MP_BUFFER_READ);

    uint8_t *dest = m_new(uint8_t, CRYPTO_SHA1_DIGEST_LENGTH);

    if (retVal = rtl_crypto_hmac_sha1(bufinfo_read.buf, bufinfo_read.len,
                        bufinfo_key.buf, bufinfo_key.len,
                        dest) < 0) {
        m_del(uint8_t, dest, CRYPTO_SHA1_DIGEST_LENGTH);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception,
                    "CRYPTO engine (hmac_sha1) process failed [%u]", retVal));
    }
    return mp_obj_new_bytearray_by_ref(CRYPTO_SHA1_DIGEST_LENGTH, dest);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(crypto_hmac_sha1_object, crypto_hmac_sha1);

STATIC mp_obj_t crypto_hmac_sha2(mp_obj_t self_in, mp_obj_t type_in, mp_obj_t array_in, mp_obj_t key_in) {
    int8_t retVal;
    uint8_t sha2_type;

    sha2_type = mp_obj_get_int(type_in);
    
    if ((sha2_type != SHA2_NONE) &&
        (sha2_type != SHA2_224) &&
        (sha2_type != SHA2_256) &&
        (sha2_type != SHA2_384) &&
        (sha2_type != SHA2_512)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Invalid SHA2 type"));
    }

    mp_buffer_info_t bufinfo_read;
    mp_buffer_info_t bufinfo_key;
    mp_get_buffer_raise(array_in, &bufinfo_read, MP_BUFFER_READ);
    mp_get_buffer_raise(key_in, &bufinfo_key, MP_BUFFER_READ);

    uint8_t *dest = m_new(uint8_t, CRYPTO_SHA2_DIGEST_LENGTH);

    if (retVal = rtl_crypto_hmac_sha2(sha2_type,
                        bufinfo_read.buf, bufinfo_read.len,
                        bufinfo_key.buf, bufinfo_key.len,
                        dest) < 0) {
        m_del(uint8_t, dest, CRYPTO_SHA2_DIGEST_LENGTH);
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception,
                    "CRYPTO engine (hmac_sha1) process failed [%u]", retVal));
    }
    return mp_obj_new_bytearray_by_ref(CRYPTO_SHA2_DIGEST_LENGTH, dest);
}
STATIC  MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(crypto_hmac_sha2_object, 4, 4, crypto_hmac_sha2);

STATIC const mp_map_elem_t crypto_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_md5),       MP_OBJ_FROM_PTR(&crypto_md5_object) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sha1),      MP_OBJ_FROM_PTR(&crypto_sha1_object) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sha2),      MP_OBJ_FROM_PTR(&crypto_sha2_object) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_hmac_md5),  MP_OBJ_FROM_PTR(&crypto_hmac_md5_object) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_hmac_sha1), MP_OBJ_FROM_PTR(&crypto_hmac_sha1_object) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_hmac_sha2), MP_OBJ_FROM_PTR(&crypto_hmac_sha2_object) },

#if 0

    { MP_OBJ_NEW_QSTR(MP_QSTR_aes_cbc),   MP_OBJ_FROM_PTR(&crypto_aes_cbc_object) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_aes_ecb),   (mp_obj_t)&crypto_aes_aes_object },
    { MP_OBJ_NEW_QSTR(MP_QSTR_aes_ctr),   (mp_obj_t)&crypto_aes_ctr_object },

    { MP_OBJ_NEW_QSTR(MP_QSTR_3des_cbc),  (mp_obj_t)&crypto_3des_cbc_object },
    { MP_OBJ_NEW_QSTR(MP_QSTR_3des_ecb),  (mp_obj_t)&crypto_3des_ecb_object },

    { MP_OBJ_NEW_QSTR(MP_QSTR_des_cbc),   (mp_obj_t)&crypto_des_cbc_object },
    { MP_OBJ_NEW_QSTR(MP_QSTR_des_ecb),   (mp_obj_t)&crypto_des_ecb_object },

#endif

    { MP_OBJ_NEW_QSTR(MP_QSTR_SHA2_NONE), MP_OBJ_NEW_SMALL_INT(SHA2_NONE) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SHA2_224),  MP_OBJ_NEW_SMALL_INT(SHA2_224) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SHA2_256),  MP_OBJ_NEW_SMALL_INT(SHA2_256) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SHA2_384),  MP_OBJ_NEW_SMALL_INT(SHA2_384) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SHA2_512),  MP_OBJ_NEW_SMALL_INT(SHA2_512) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_ENCRYPT),  MP_OBJ_NEW_SMALL_INT(CRYPTO_ENCRYPT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DECRYPT),  MP_OBJ_NEW_SMALL_INT(CRYPTO_DECRYPT) },
    
    // Errno
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_DESC_NUM_SET_OutRange),          MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_DESC_NUM_SET_OutRange) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_BURST_NUM_SET_OutRange),         MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_BURST_NUM_SET_OutRange) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_NULL_POINTER),                   MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_NULL_POINTER) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_ENGINE_NOT_INIT),                MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_ENGINE_NOT_INIT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_ADDR_NOT_4Byte_Aligned),         MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_ADDR_NOT_4Byte_Aligned) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_KEY_OutRange),                   MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_KEY_OutRange) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_MSG_OutRange),                   MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_MSG_OutRange) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_IV_OutRange),                    MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_IV_OutRange) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_AUTH_TYPE_NOT_MATCH),            MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_AUTH_TYPE_NOT_MATCH) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_CIPHER_TYPE_NOT_MATCH),          MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_CIPHER_TYPE_NOT_MATCH) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_KEY_IV_LEN_DIFF),                MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_KEY_IV_LEN_DIFF) },
#ifdef AMEBA1
    { MP_OBJ_NEW_QSTR(MP_QSTR_ERRNO_AES_MSGLEN_NOT_16Byte_Aligned),  MP_OBJ_NEW_SMALL_INT(_ERRNO_CRYPTO_AES_MSGLEN_NOT_16Byte_Aligned) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(crypto_locals_dict, crypto_locals_dict_table);

const mp_obj_type_t crypto_type = {
    { &mp_type_type },
    .name        = MP_QSTR_CRYPTO,
    .print       = crypto_print,
    .make_new    = crypto_make_new,
    .locals_dict = (mp_obj_t)&crypto_locals_dict,
};
