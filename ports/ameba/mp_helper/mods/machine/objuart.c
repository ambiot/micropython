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
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/stream.h"

#include "py/mperrno.h"
#include "exception.h"

#include "bufhelper.h"
#include "objuart.h"

STATIC const char *_parity_name[] = {"None", "1", "0"};

serial_t mp_uart_obj;

STATIC uart_obj_t uart_obj[2] = {{
    .base.type      = &uart_type,
    .unit           = 0,
    .params = {
        .baudrate  = UART_DEFAULT_BAUDRATE,
        .data_bits = UART_DEFAULT_DATA_BITS,
        .parity    = UART_DEFAULT_PARITY,
        .stop_bits = UART_DEFAULT_STOP_BITS,
    },
    .tx = {
        .timeout_ms = UART_DEFAULT_TX_TIMEOUT,
    },
    .rx = {
        .timeout_ms = UART_DEFAULT_RX_TIMEOUT,
    },
    .irq_enabled = true,
    .irq_handler = mp_const_none,
}, {
    .base.type      = &uart_type,
    .unit           = 3,
    .params = {
        .baudrate  = UART_DEFAULT_BAUDRATE,
        .data_bits = UART_DEFAULT_DATA_BITS,
        .parity    = UART_DEFAULT_PARITY,
        .stop_bits = UART_DEFAULT_STOP_BITS,
    },
    .tx = {
        .timeout_ms = UART_DEFAULT_TX_TIMEOUT,
    },
    .rx = {
        .timeout_ms = UART_DEFAULT_RX_TIMEOUT,
    },
    .irq_enabled = true,
    .irq_handler = mp_const_none,
}
};

void mp_obj_uart_irq_handler(uart_obj_t *self, SerialIrq event) {
    /*
     * At least read one char from register to prevent from interrupt pending
     */
    if (event == RxIrq) {

        char chr = serial_getc(&mp_uart_obj);
       
        if (self->irq_handler != mp_const_none) {
        /*
        * Don't lock gc (gc_lock) here because we need to create a new queue for mallo, if locked
        * gc_alloc would be wrong
        */
            nlr_buf_t nlr;
            if (nlr_push(&nlr) == 0) {
                mp_call_function_2(self->irq_handler, MP_OBJ_FROM_PTR(self), 
                        mp_obj_new_bytes(&chr, 1));
                nlr_pop();
            } else {
                self->irq_handler = mp_const_none;
                mp_printf(&mp_plat_print, "Uncaught exception in callback handler");
                if (nlr.ret_val != MP_OBJ_NULL)
                    mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            }
        }
    }
}

STATIC void uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    uart_obj_t *self = self_in;
    mp_printf(print, "UART(baudrate=%u, bits=%u, parity=%s, stop=%u, tx_timeout=%u, rx_timeout=%u)",
        self->params.baudrate, self->params.data_bits,
        _parity_name[self->params.parity], self->params.stop_bits,
        self->tx.timeout_ms, self->rx.timeout_ms);
}

STATIC mp_obj_t uart_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    // TODO: CTS/RTS pin and flow control needed!
    enum {ARG_unit, ARG_baudrate, ARG_bits, ARG_stop, ARG_parity, ARG_timeout, ARG_tx, ARG_rx};
    const mp_arg_t uart_init_args[] = {
        { MP_QSTR_unit,                          MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_baudrate,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_DEFAULT_BAUDRATE} },
        { MP_QSTR_bits,         MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_DEFAULT_DATA_BITS} },
        { MP_QSTR_stop,         MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_DEFAULT_STOP_BITS} },
        { MP_QSTR_parity,       MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_DEFAULT_PARITY} },
        { MP_QSTR_timeout,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_DEFAULT_RX_TIMEOUT} },
        { MP_QSTR_tx,           MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_rx,           MP_ARG_REQUIRED | MP_ARG_OBJ },
    };
    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(uart_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args), uart_init_args, args);

    pin_obj_t *tx = (pin_obj_t *)pin_find(args[ARG_tx].u_obj);
    pin_obj_t *rx = (pin_obj_t *)pin_find(args[ARG_rx].u_obj);

    PinName pn_tx = (PinName)pinmap_peripheral(tx->id, PinMap_UART_TX);
    PinName pn_rx = (PinName)pinmap_peripheral(rx->id, PinMap_UART_RX);

    if (pn_tx == NC)
        mp_raise_ValueError("UART TX pin not match");

    if (pn_rx == NC)
        mp_raise_ValueError("UART RX pin not match");

    uart_obj_t *self       = &uart_obj[args[ARG_unit].u_int];
    self->params.baudrate  = MIN(MAX(args[ARG_baudrate].u_int, UART_MIN_BAUDRATE), UART_MAX_BAUDRATE);
    self->params.data_bits = args[ARG_bits].u_int;
    self->params.stop_bits = args[ARG_stop].u_int;
    self->params.parity    = args[ARG_parity].u_int;
    self->tx.timeout_ms    = args[ARG_timeout].u_int;
    self->rx.timeout_ms    = args[ARG_timeout].u_int;
    self->tx.pin           = tx;
    self->rx.pin           = rx;

    serial_init(&mp_uart_obj, self->tx.pin->id, self->rx.pin->id);

    return (mp_obj_t)self;
}

STATIC mp_obj_t uart_init0(mp_obj_t self_in) {
    uart_obj_t *self = self_in;
    serial_baud(&mp_uart_obj, self->params.baudrate);
    serial_format(&mp_uart_obj, self->params.data_bits, self->params.parity, self->params.stop_bits);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_init_obj, uart_init0);

STATIC mp_obj_t uart_deinit0(mp_obj_t self_in) {
    uart_obj_t *self = self_in;
    serial_free(&mp_uart_obj);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uart_deinit_obj, uart_deinit0);

STATIC mp_obj_t uart_irq_enable(mp_uint_t n_args, const mp_obj_t *args) {
    uart_obj_t *self = args[0];
    if (n_args == 1) {
        // get the value
        return self->irq_enabled ? mp_const_true:mp_const_false;
    } else {
        // set the pin value
        if (mp_obj_is_true(args[1])) {
            self->irq_enabled = true;
        } else {
            self->irq_enabled = false;
        }
        serial_irq_set(&mp_uart_obj, RxIrq, self->irq_enabled);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(uart_irq_enable_obj, 1, 2, uart_irq_enable);

STATIC mp_obj_t uart_irq_handler0(mp_obj_t self_in, mp_obj_t func_in) {
    uart_obj_t *self = self_in;

    if (!MP_OBJ_IS_FUN(func_in) && (func_in != mp_const_none)) {
        mp_raise_ValueError("Error function type");
    }

    self->irq_handler = func_in;
    serial_irq_handler(&mp_uart_obj, mp_obj_uart_irq_handler, self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(uart_irq_handler_obj, uart_irq_handler0);

STATIC const mp_map_elem_t uart_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_init),        MP_OBJ_FROM_PTR(&uart_init_obj) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_deinit),      MP_OBJ_FROM_PTR(&uart_deinit_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),        MP_OBJ_FROM_PTR(&mp_stream_read_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readline),    MP_OBJ_FROM_PTR((&mp_stream_unbuffered_readline_obj)) },
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_readinto),    MP_OBJ_FROM_PTR((&mp_stream_readinto_obj)) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),       MP_OBJ_FROM_PTR(&mp_stream_write_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_irq_enable),  MP_OBJ_FROM_PTR((&uart_irq_enable_obj)) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_irq_handler), MP_OBJ_FROM_PTR((&uart_irq_handler_obj)) },

    // class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_ParityNone),    MP_OBJ_NEW_SMALL_INT(ParityNone) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ParityOdd),     MP_OBJ_NEW_SMALL_INT(ParityOdd) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ParityEven),    MP_OBJ_NEW_SMALL_INT(ParityEven) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ParityForced1), MP_OBJ_NEW_SMALL_INT(ParityForced1) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ParityForced0), MP_OBJ_NEW_SMALL_INT(ParityForced0) },
#if 0 // Not support yet
    { MP_OBJ_NEW_QSTR(MP_QSTR_FlowControlNone),   MP_OBJ_NEW_SMALL_INT(FlowControlNone) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FlowControlRTS),    MP_OBJ_NEW_SMALL_INT(FlowControlRTS) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FlowControlCTS),    MP_OBJ_NEW_SMALL_INT(FlowControlCTS) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FlowControlRTSCTS), MP_OBJ_NEW_SMALL_INT(FlowControlRTSCTS) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(uart_locals_dict, uart_locals_dict_table);

STATIC mp_obj_t uart_recv(mp_obj_t self_in, char *buf_in, mp_uint_t size, int *errcode) {
    uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    
    int32_t ret = 0;

    // Direct return 0 when size = 0, to save the time
    if (size == 0) 
        return 0;

    mp_uint_t start = mp_hal_ticks_ms();
    for (mp_uint_t i = 0; i < size; i++) {
        while (!serial_readable(&mp_uart_obj)) {
            if ((mp_hal_ticks_ms() - start) > self->rx.timeout_ms) {
                *errcode = MP_ETIMEDOUT;
                goto ret;
            }
        }
        ret += 1;
        buf_in[i] = (byte)serial_getc(&mp_uart_obj);
    }

ret:
    if (ret == 0) {
        return MP_STREAM_ERROR;
    } else {
        return ret;
    }
}

STATIC mp_obj_t uart_send(mp_obj_t self_in, const char *buf_in, mp_uint_t size, int *errcode) {
    uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    int32_t ret = 0;

    mp_uint_t start = mp_hal_ticks_ms();
    for (mp_uint_t i = 0; i < size; i++) {
        while (!serial_writable(&mp_uart_obj)) {
            if ((mp_hal_ticks_ms() - start) > self->tx.timeout_ms) {
                *errcode = MP_ETIMEDOUT;
                goto ret;
            }
        }
        ret += 1;
        serial_putc(&mp_uart_obj, buf_in[i]);

        // A workaround, it seems log uart's FIFO is not working ...
        mp_hal_delay_us(250);
    }

ret:
    if (ret == 0) {
        return MP_STREAM_ERROR;
    } else {
        return ret;
    }
}

STATIC mp_uint_t uart_ioctl(mp_obj_t self_in, mp_uint_t request, mp_uint_t arg, int *errcode) {
    uart_obj_t *self = self_in;
    mp_uint_t ret;
    if (request == MP_STREAM_POLL) {
        mp_uint_t flags = arg;
        ret = 0;
        mp_uint_t status = 0;
        // Only return none zero when RX FIFO is not empty
        status = serial_readable(&mp_uart_obj);
        if ((flags & MP_STREAM_POLL_RD) && (status & true)) {
            ret |= MP_STREAM_POLL_RD;
        }

        // Only return none zero when TX FIFO is not full
        status = serial_writable(&mp_uart_obj);
        if ((flags & MP_STREAM_POLL_WR) && (status & true)) {
            ret |= MP_STREAM_POLL_WR;
        }
    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }
    return ret;
}

STATIC const mp_stream_p_t uart_stream_p = {
    .read    = uart_recv,
    .write   = uart_send,
    .ioctl   = uart_ioctl,
    .is_text = false,
};

const mp_obj_type_t uart_type = {
    { &mp_type_type },
    .name        = MP_QSTR_UART,
    .print       = uart_print,
    .make_new    = uart_make_new,
    .getiter     = mp_identity_getiter,
    .iternext    = mp_stream_unbuffered_iter,
    .protocol    = &uart_stream_p,
    .locals_dict = (mp_obj_dict_t *)&uart_locals_dict,
};
