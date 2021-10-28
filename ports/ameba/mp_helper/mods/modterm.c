/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Chester Tseng
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
// micropython headers
#include "py/mpconfig.h"
#include "py/misc.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/objarray.h"
#include "py/stream.h"
#include "py/objstr.h"
#include "py/ringbuf.h"
#include "interrupt_char.h"
#include "pyexec.h"
#include "modterm.h"

/*****************************************************************************
 *                              Internal variables
 * ***************************************************************************/
void *xRxQueue;

/*****************************************************************************
 *                              Internal variables
 * ***************************************************************************/
typedef struct _poll_obj_t {
    mp_obj_t obj;
    mp_uint_t (*ioctl)(mp_obj_t obj, mp_uint_t request, mp_uint_t arg, int *errcode);
    mp_uint_t (*read)(mp_obj_t obj, void *buf_in, mp_uint_t size, int *errcode);
    mp_uint_t (*write)(mp_obj_t obj, const void *buf_in, mp_uint_t size, int *errcode);
    mp_uint_t flags;
} poll_obj_t;

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/
STATIC void modterm_map_add(mp_map_t *poll_map, const mp_obj_t *obj, mp_uint_t obj_len, mp_uint_t flags) {
    for (mp_uint_t i = 0; i < obj_len; i++) {
        mp_map_elem_t *elem = mp_map_lookup(poll_map, mp_obj_id(obj[i]), MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
        if (elem->value == NULL) {
            // object not found; get its ioctl, write, read and add it to the poll list
            const mp_stream_p_t *stream_p = mp_get_stream_raise(obj[i], MP_STREAM_OP_IOCTL);
            poll_obj_t *poll_obj = m_new_obj(poll_obj_t);
            poll_obj->obj = obj[i];
            poll_obj->ioctl = stream_p->ioctl;
            poll_obj->read  = stream_p->read;
            poll_obj->write = stream_p->write;
            poll_obj->flags = flags;
            elem->value = poll_obj;
        } else {
            // object exists; override its' flag
            ((poll_obj_t*)elem->value)->flags = flags;
        }
    }
}

void mp_term_tx_strn(char *str, size_t len) {
    printf("inside--mp_term_tx_strn\n");
    for (mp_uint_t i = 0; i < MP_STATE_PORT(mp_terminal_map).alloc; ++i) {
        if (!MP_MAP_SLOT_IS_FILLED(&MP_STATE_PORT(mp_terminal_map), i)) {
            continue;
        }
        poll_obj_t *poll_obj = (poll_obj_t*)MP_STATE_PORT(mp_terminal_map).table[i].value;
        int errcode = 0;
        mp_int_t ret = poll_obj->ioctl(poll_obj->obj, MP_STREAM_POLL, poll_obj->flags, &errcode);
        if (ret & MP_STREAM_POLL_HUP) {
            mp_map_lookup(&MP_STATE_PORT(mp_terminal_map), mp_obj_id(poll_obj->obj),
                    MP_MAP_LOOKUP_REMOVE_IF_FOUND);
            mp_obj_t close_m[3];
            mp_load_method(poll_obj->obj, MP_QSTR_close, close_m);
            mp_call_method_n_kw(0, 0, close_m);
            continue;
        }
        if (ret & MP_STREAM_POLL_WR && !(ret & MP_STREAM_POLL_HUP)) {
            mp_int_t ret = poll_obj->write(poll_obj->obj, str, len, &errcode);
            if (ret == MP_STREAM_ERROR)
                continue;
        }
    }
}

int mp_term_rx_chr() {
	char chr = 0;
    printf("inside--mp_term_rx_chr\n");

    if (rtw_pop_from_xqueue(&xRxQueue, (void *)&chr, RTW_WAIT_FOREVER) < 0) {
		// Do something here...
    } else {
        return chr;
    }
  
}

void modterm_rx_thread(void const *arg) {
    char array[TERM_RX_CHUNK_SIZE];
    for(;;) {
        char chr = 0;
        for (mp_uint_t i = 0; i < MP_STATE_PORT(mp_terminal_map).alloc; ++i) {

            if (!MP_MAP_SLOT_IS_FILLED(&MP_STATE_PORT(mp_terminal_map), i)) {
                continue;
            }

            poll_obj_t *poll_obj = (poll_obj_t*)MP_STATE_PORT(mp_terminal_map).table[i].value;
            int errcode;

            mp_int_t ret = poll_obj->ioctl(poll_obj->obj, MP_STREAM_POLL, poll_obj->flags, &errcode);
            
            if (ret & MP_STREAM_POLL_HUP) {
                mp_map_lookup(&MP_STATE_PORT(mp_terminal_map), mp_obj_id(poll_obj->obj),
                        MP_MAP_LOOKUP_REMOVE_IF_FOUND);
                mp_obj_t close_m[3];
                mp_load_method(poll_obj->obj, MP_QSTR_close, close_m);
                mp_call_method_n_kw(0, 0, close_m);
                continue;
            }

            if (ret & MP_STREAM_POLL_RD && !(ret & MP_STREAM_POLL_HUP)) {
                memset(array, 0x00, TERM_RX_CHUNK_SIZE);
                mp_int_t ret = poll_obj->read(poll_obj->obj, array, TERM_RX_CHUNK_SIZE, &errcode);
                for (mp_uint_t j = 0; j < TERM_RX_CHUNK_SIZE; j++) {
                    // Do something with ret
                    chr = array[j];
                    if (chr == mp_interrupt_char)
                        mp_sched_keyboard_interrupt();
                    else if (chr != 0x00){
                        rtw_push_to_xqueue(&xRxQueue, (void *)&chr, RTW_WAIT_FOREVER);
                    } else {}
                }
            }
        }
        rtw_yield_os();
    }
    rtw_deinit_xqueue(&xRxQueue);
    rtw_thread_exit();;
}

void modterm_init (void) {
    mp_map_init(&MP_STATE_PORT(mp_terminal_map), 0);

    // Create terminal task
    struct task_struct stTermTask;
    BaseType_t xReturn = rtw_create_task(&stTermTask, MICROPY_TERM_RX_STACK_NAME,
            MICROPY_TERM_RX_STACK_DEPTH, MICROPY_TERM_RX_STACK_PRIORITY, modterm_rx_thread, NULL);
    rtw_init_xqueue(&xRxQueue, "RX_Q", sizeof(char), TERM_RX_QUEUE_MSG_LENGTH);
}

STATIC mp_obj_t term_register(mp_obj_t obj_in) {
    modterm_map_add(&MP_STATE_PORT(mp_terminal_map), &obj_in, 1, MP_STREAM_POLL_RD | MP_STREAM_POLL_WR | MP_STREAM_POLL_HUP);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(term_register_obj, term_register);

STATIC mp_obj_t term_unregister(mp_obj_t obj_in) {
    mp_map_lookup(&MP_STATE_PORT(mp_terminal_map), mp_obj_id(obj_in), MP_MAP_LOOKUP_REMOVE_IF_FOUND);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(term_unregister_obj, term_unregister);

STATIC const mp_map_elem_t uterminal_module_globals_table[] = {
    //{ MP_OBJ_NEW_QSTR(MP_QSTR___name__),   MP_OBJ_NEW_QSTR(MP_QSTR_uterm) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_register),   MP_OBJ_FROM_PTR(&term_register_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_unregister), MP_OBJ_FROM_PTR(&term_unregister_obj) },
};
STATIC MP_DEFINE_CONST_DICT(uterminal_module_globals, uterminal_module_globals_table);

const mp_obj_module_t mp_module_uterminal = {
    .base    = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&uterminal_module_globals,
};
