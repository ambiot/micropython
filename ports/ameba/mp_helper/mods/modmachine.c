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
// micropython headers

// local object headers
//
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "machine/objuart.h"
#include "machine/objpin.h"
#include "machine/objtimer.h"
#include "machine/objrtc.h"
#include "machine/objpwm.h"
#include "machine/obji2c.h" 
#include "machine/objspi.h" 
#include "machine/objadc.h"
#include "machine/objsdcard.h"
#include "machine/objflash.h"
#include "machine/objwdt.h"
#include "us_ticker_api.h"

#define TICK_READ_FROM_CPU 0
/*
#include "machine/objcrypto.h"
*/

// mbed lib headers
#include "sys_api.h"
//#include "sleep_ex_api.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/
extern TaskHandle_t mp_main_task_handle;


void modmachine_init(void) {
    rtc_init0();
    //crypto_init0();
}

STATIC mp_obj_t machine_reset(void) {
    sys_reset();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(machine_reset_obj, machine_reset);


STATIC mp_obj_t machine_info(void) {
    // FreeRTOS info
    printf("---------------------------------------------\n");
    printf("FreeRTOS Memory Info\n");
    printf("---------------------------------------------\n");
    printf("Total heap: %d\n", configTOTAL_HEAP_SIZE);
    
    char ptrTaskList[250];
    vTaskList(ptrTaskList);
    printf("*******************************************\n");
    printf("Task          State   Prio  Stack(avai) Num\n"); 
    printf("*******************************************\n");
    printf("%s", ptrTaskList);
    printf("*******************************************\n");
    printf("The avail heap from FreeRTOS: %d bytes\n", xPortGetFreeHeapSize());
    printf("*******************************************\n");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(machine_info_obj, machine_info);



#if 0
STATIC mp_obj_t machine_deepsleep(mp_obj_t duration_in) {
    uint32_t duration = mp_obj_get_int(duration_in);
    deepsleep_ex(DSLEEP_WAKEUP_BY_TIMER, duration);
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(machine_deepsleep_obj, machine_deepsleep);
#endif // end of checking for 'machine_deepsleep'

MP_WEAK mp_uint_t machine_time_pulse_us(mp_obj_t self_in, int pulse_level, mp_uint_t timeout_us) {
    mp_uint_t start =  (mp_uint_t)us_ticker_read();
    pin_obj_t *self = self_in;
    // printf("%d\n", gpio_read((gpio_t *)&(self->obj)) );
    while (gpio_read((gpio_t *)&(self->obj)) != pulse_level) {
        if ((mp_uint_t)((mp_uint_t)us_ticker_read() - start) >= timeout_us) {
            return (mp_uint_t)-2;
        }
    }
    start = (mp_uint_t)us_ticker_read();
    while (gpio_read((gpio_t *)&(self->obj)) == pulse_level) {
        if ((mp_uint_t)((mp_uint_t)us_ticker_read() - start) >= timeout_us) {
            return (mp_uint_t)-1;
        }
    }
    return (mp_uint_t)((mp_uint_t)us_ticker_read() - start);
}

STATIC mp_obj_t machine_time_pulse_us_(size_t n_args, const mp_obj_t *args) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(args[0]);
    int level = 0;
    if (mp_obj_is_true(args[1])) {
        level = 1;
    }
    mp_uint_t timeout_us = 1000000;
    if (n_args > 2) {
        timeout_us = mp_obj_get_int(args[2]);
    }
    mp_uint_t us = machine_time_pulse_us(pin, level, timeout_us);
    // May return -1 or -2 in case of timeout
    return mp_obj_new_int(us);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_time_pulse_us_obj, 2, 3, machine_time_pulse_us_);

STATIC const mp_map_elem_t machine_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__),      MP_OBJ_NEW_QSTR(MP_QSTR_umachine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reboot),        MP_OBJ_FROM_PTR(&machine_reset_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_memInfo),       MP_OBJ_FROM_PTR(&machine_info_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_UART),          MP_OBJ_FROM_PTR(&uart_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Pin),           MP_OBJ_FROM_PTR(&pin_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Timer),         MP_OBJ_FROM_PTR(&timer_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RTC),           MP_OBJ_FROM_PTR(&rtc_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PWM),           MP_OBJ_FROM_PTR(&machine_pwm_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_I2C),           MP_OBJ_FROM_PTR(&machine_hw_i2c_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SPI),           MP_OBJ_FROM_PTR(&machine_spi_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ADC),           MP_OBJ_FROM_PTR(&adc_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WDT),           MP_OBJ_FROM_PTR(&wdt_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SDCard),        MP_OBJ_FROM_PTR(&sdcard_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FLASH),         MP_OBJ_FROM_PTR(&flash_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_time_pulse_us), MP_OBJ_FROM_PTR(&machine_time_pulse_us_obj) },

    #if 0
    { MP_OBJ_NEW_QSTR(MP_QSTR_CRYPTO),        MP_OBJ_FROM_PTR(&crypto_type) },
    #endif
};
STATIC MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t mp_module_machine = {
    .base    = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&machine_module_globals,
};
