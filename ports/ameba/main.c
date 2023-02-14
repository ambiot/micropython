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

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/compile.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "readline.h"
#include "pyexec.h"
#include "gccollect.h"
#include "exception.h"
#include "section_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device.h"
#include "serial_api.h"
#include "main.h"
#include "interrupt_char.h"
#include "mphalport.h"
#include "sys_api.c"



/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/

static void* app_mbedtls_calloc_func(size_t nelements, size_t elementSize)
{
    size_t size;
    void *ptr = NULL;

    size = nelements * elementSize;
    ptr = pvPortMalloc(size);

    if (ptr) {
        _memset(ptr, 0, size);
    }

    return ptr;
}

void app_mbedtls_rom_init(void)
{
    mbedtls_platform_set_calloc_free(app_mbedtls_calloc_func, vPortFree);
    //rom_ssl_ram_map.use_hw_crypto_func = 1;
    rtl_cryptoEngine_init();
}

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/


//osThreadId main_tid = 0;
TaskHandle_t mp_main_task_handle;

uint8_t mpHeap[MP_HEAP_SIZE];

void micropython_task(void const *arg) {

soft_reset:
    uart_repl_init();
    mp_stack_ctrl_init();

#if MICROPY_ENABLE_GC
    gc_init(mpHeap, mpHeap + sizeof(mpHeap));
#endif

    // Init MP runtime
    mp_init();
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));

    // Initialise sub-systems.
    readline_init0();
    modmachine_init();

    // Execute _boot.py to set up the filesystem
    pyexec_frozen_module("_boot.py");
//    #if MICROPY_VFS_FAT &&  
//    pyexec_frozen_module("_boot_fat.py");
//    #else
//    pyexec_frozen_module("_boot.py");
//    #endif

    // Execute user scripts.
    int ret = pyexec_file_if_exists("boot.py");
    if (ret & PYEXEC_FORCED_EXIT) {
        goto soft_reset_exit;
    }
    if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
        ret = pyexec_file_if_exists("main.py");
        if (ret & PYEXEC_FORCED_EXIT) {
            goto soft_reset_exit;
        }
    }

    for ( ; ; ) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0)
                break;
        } else {
            if (pyexec_friendly_repl() != 0) 
                break;
        }
        //osThreadYield();
    }

soft_reset_exit:

    gc_sweep_all();
    mp_hal_stdout_tx_str("MPY: soft reboot\r\n");
    mp_deinit();
    goto soft_reset;

}


int main (void) {

#ifdef CONFIG_MBED_TLS_ENABLED
    app_mbedtls_rom_init();
#endif

    //For all amebad boards, Analog pin needs to pull none. GPIO_PuPd_NOPULL/GPIO_PuPd_DOWN/GPIO_PuPd_UP
    PAD_PullCtrl(_PB_1, GPIO_PuPd_NOPULL);
    PAD_PullCtrl(_PB_2, GPIO_PuPd_NOPULL);
    PAD_PullCtrl(_PB_3, GPIO_PuPd_NOPULL);
    sys_jtag_off();

//    osThreadDef(micropython_task, MICROPY_TASK_PRIORITY, 1, MICROPY_TASK_STACK_DEPTH);
//    main_tid = osThreadCreate(osThread(micropython_task), NULL);

    if(xTaskCreate(micropython_task, \
                    ((const char*)"micropython_main_task"), \
                    MICROPY_TASK_STACK_DEPTH, \
                    NULL, \
                    tskIDLE_PRIORITY + 1, \
                    mp_main_task_handle) != pdPASS) {
                        printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
                    }

    //osKernelStart();
    vTaskStartScheduler();

    for(;;);
    
    return 0;
}


void nlr_jump_fail(void *val) {
    mp_printf(&mp_plat_print, "FATAL: uncaught exception %p\r\n", val);
    while(1);
}

