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
#include <stdio.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mpstate.h"
#include "py/stream.h"
#include "extmod/misc.h"
#include "pyexec.h"
#include "py/obj.h"
#include "py/gc.h"
#include "py/ringbuf.h"
#include "wait_api.h"
#include "serial_api.h"
#include <stdio.h>
#include "interrupt_char.h"
#include "cmsis_os.h"
#include "ameba_soc.h"
#include "osdep_service.h"

extern TaskHandle_t mp_main_task_handle;

#define USE_RINGBUF 1

serial_t uartobj;

STATIC uint8_t uart_ringbuf_array[256];
ringbuf_t stdin_ringbuf = {uart_ringbuf_array, sizeof(uart_ringbuf_array), 0, 0};

/* LOGUART pins: */
#define UART_TX PA_7
#define UART_RX PA_8


// volatile int repl_buf = 0;

void uart_repl_handler(uint32_t id, SerialIrq event)
{
    gc_lock();

    ringbuf_t *handlerRingBuf = (ringbuf_t *)id;

    if (event == RxIrq)
    {
        //        while(serial_readable(&uartobj)) {
        int repl_buf = serial_getc(&uartobj);
        if (repl_buf == mp_interrupt_char)
        {
            mp_sched_keyboard_interrupt();
        }
        else
        {
            ringbuf_put(handlerRingBuf, (uint8_t)repl_buf);
        }
        //        }
    }
    gc_unlock();
}

void uart_repl_init()
{
    // init repl on UART and enable uart rx interrupt for receiving incoming data and handle keyboard interrupt
    serial_init(&uartobj, UART_TX, UART_RX);
    serial_baud(&uartobj, 115200);
    serial_format(&uartobj, 8, ParityNone, 1);
    serial_irq_handler(&uartobj, uart_repl_handler, (uint32_t)&stdin_ringbuf);
    serial_irq_set(&uartobj, RxIrq, 1);
}

void uart_send_string(serial_t *uartobj, const char *pstr)
{
    unsigned int i = 0;

    while (*(pstr + i) != 0)
    {
        serial_putc(uartobj, *(pstr + i));
        i++;
    }
}

void uart_send_string_with_length(serial_t *uartobj, const char *pstr, size_t len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        serial_putc(uartobj, pstr[i]);
    }
}

/////////////////////////////////////
//       HAL STDIO TX & RX         //
/////////////////////////////////////
int mp_hal_stdin_rx_chr(void)
{
#if USE_RINGBUF
    for (;;)
    {
        int c = ringbuf_get(&stdin_ringbuf);
        if (c != -1)
        {
            return c;
        }
        MICROPY_EVENT_POLL_HOOK
    }
#else
    int c = serial_getc(&uartobj);
    if (c == mp_interrupt_char)
    {
        mp_sched_keyboard_interrupt();
    }
    else
    {
        return c;
    }
#endif
}

int mp_hal_stdin_rx_readable(void)
{
#if USE_RINGBUF
    return ringbuf_avail(&stdin_ringbuf);
#else
    return serial_readable(&uartobj);
#endif
}

void mp_hal_stdout_tx_strn(const char *str, size_t len)
{
    // printf("--mp_hal_stdout_tx_strn--\n");
    // mp_term_tx_strn(str, len);
    uart_send_string_with_length(&uartobj, str, len);
}

void mp_hal_stdout_tx_chr(char c)
{
    // printf("mp_hal_stdout_tx_chr\n");
    // mp_term_tx_strn(&c, 1);
    serial_putc(&uartobj, (int)c);
}

void mp_hal_stdout_tx_str(const char *str)
{
    // printf("mp_hal_stdout_tx_str\n");
    // mp_term_tx_strn(str, strlen(str));
    uart_send_string(&uartobj, str);
}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len)
{
    while (len--)
    {
        if (*str == '\n')
        {
            mp_hal_stdout_tx_chr('\r');
        }
        // printf("mp_hal_stdout_tx_strn_cooked\n");
        mp_hal_stdout_tx_chr(*str++);
    }
}

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags)
{
#if USE_RINGBUF
    uintptr_t ret = 0;
    if ((poll_flags & MP_STREAM_POLL_RD) && ringbuf_peek(&stdin_ringbuf) != -1)
    {
        ret |= MP_STREAM_POLL_RD;
    }
    return ret;
#else
    poll_flags = poll_flags;
    return MP_STREAM_POLL_RD;
#endif
}

///////////////////////////////
//       Delay & Time        //
///////////////////////////////
void mp_hal_delay_ms(uint32_t ms)
{
    // vTaskDelay(ms) // vTaskDelay takes in tick as parameter, each tick = 1 ms
    osDelay(ms); // RTOS delay
}

void mp_hal_delay_us(uint32_t us)
{
    wait_us(us); // asm NOP
}

uint32_t mp_hal_ticks_ms(void)
{
    return rtw_get_current_time();
}

uint32_t mp_hal_ticks_us(void)
{
    // function to get the ticks time in microseconds precision
    uint64_t ticks = SYSTIMER_TickGet();
    return ticks;
}

// Wake up the main task if it is sleeping
void mp_hal_wake_main_task_from_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(mp_main_task_handle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR(pdTRUE);
    }
}

uint64_t mp_hal_time_ns(void)
{
    uint64_t ticks = SYSTIMER_TickGet(); // resolution is 31us
    return ticks * 31000ULL;
}



