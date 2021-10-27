/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George on behalf of Pycom Ltd
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

#include <stdio.h>

#include "py/mpconfig.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mpthread.h"
#include "py/mphal.h"


#if MICROPY_PY_THREAD

extern StackType_t mpTaskStack[];

// the mutex controls access to the linked list
STATIC mp_thread_mutex_t thread_mutex;
STATIC mp_thread_t thread_entry0;
STATIC mp_thread_t *thread;        // root pointer, handled bp mp_thread_gc_others

void mp_thread_init(void) {
    mp_thread_mutex_init(&thread_mutex);

    // create first entry in linked list of all threads
    thread              = &thread_entry0;
    thread->id          = xTaskGetCurrentTaskHandle();
    thread->ready       = 1;
    thread->arg         = NULL;
    thread->stack       = mpTaskStack;
    thread->stack_len   = MICROPY_TASK_STACK_DEPTH;
    thread->next        = NULL;

    mp_thread_set_state(&mp_state_ctx.thread);
}

void mp_thread_gc_others(void) {
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        gc_collect_root((void**)&th, 1);
        gc_collect_root(&th->arg, 1); // probably not needed
        if (th->id == xTaskGetCurrentTaskHandle()) {
            continue;
        }
        if (!th->ready) {
            continue;
        }
        gc_collect_root(th->stack, th->stack_len); // probably not needed
    }
    mp_thread_mutex_unlock(&thread_mutex);
}

// TODO(Chester) Recursive find may cause lantency
mp_state_thread_t *mp_thread_get_state(void) {
    mp_state_thread_t *state;
    //mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            state = th->state;
            break;
        }
    }
    //mp_thread_mutex_unlock(&thread_mutex);
    return state;
}

// TODO(Chester) Recursive find may cause lantency
void mp_thread_set_state(void *state) {
    //mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->state = state;
            break;
        }
    }
    //mp_thread_mutex_unlock(&thread_mutex);
}

void mp_thread_start(void) {
    mp_thread_mutex_lock(&thread_mutex, 1);
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->ready = 1;
            break;
        }
    }
    mp_thread_mutex_unlock(&thread_mutex);
}

STATIC void *(*ext_thread_entry)(void*) = NULL;

STATIC void freertos_entry(void *arg) {
    if (ext_thread_entry) {
        ext_thread_entry(arg);
    }
    vTaskDelete(NULL);
    for (;;) {
    }
}

void mp_thread_create(void *(*entry)(void*), void *arg, size_t *stack_size) {
    // store thread entry function into a global variable so we can access it
    ext_thread_entry = entry;

    if (*stack_size == 0) {
        *stack_size = 4096; // default stack size
    } else if (*stack_size < 2048) {
        *stack_size = 2048; // minimum stack size
    }

    // allocate stack and linked-list node (must be outside thread_mutex lock)
    /*
     * pvPortMalloc from ucHeap here, and Idle task will handle the vPortFree for us
     * So don't worry about the memory free problem in stack malloc
     */
    StackType_t *stack = (StackType_t *)pvPortMalloc(*stack_size);

    mp_thread_t *th = m_new_obj(mp_thread_t);

    mp_thread_mutex_lock(&thread_mutex, 1);

    // create thread
    #if 0
    TaskHandle_t id = xTaskGenericCreate(freertos_entry,
            "Thread",
            *stack_size / sizeof(StackType_t),  // Task depth
            arg,            // Parameters
            2,              // Priority for spawn tasks
            NULL,
            stack,
            NULL);
    #endif
    TaskHandle_t id = xTaskCreate(
            freertos_entry,
            "Thread",
            *stack_size / sizeof(StackType_t),  // Task depth
            arg,            // Parameters
            2,              // Priority for spawn tasks
            NULL); // xxm


    if (id == NULL) {
        mp_thread_mutex_unlock(&thread_mutex);
        mp_raise_msg(&mp_type_OSError, "can't create thread");
    }

    // add thread to linked list of all threads
    th->id          = id;
    th->ready       = 0;
    th->arg         = arg;
    th->stack       = stack;
    th->stack_len   = *stack_size / sizeof(StackType_t);
    th->next        = thread;
    thread          = th;

    mp_thread_mutex_unlock(&thread_mutex);

    // adjust stack_size to provide room to recover from hitting the limit
    // *stack_size -= 512;
}

void mp_thread_finish(void) {
    mp_thread_mutex_lock(&thread_mutex, 1);
    // TODO unlink from list
    for (mp_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->ready = 0;
            break;
        }
    }
    mp_thread_mutex_unlock(&thread_mutex);
}

void mp_thread_mutex_init(mp_thread_mutex_t *mutex) {
    mutex->handle = xSemaphoreCreateMutex();
}

// To allow hard interrupts to work with threading we only take/give the semaphore
// if we are not within an interrupt context and interrupts are enabled.

int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait) {
    int ret = xSemaphoreTake(mutex->handle, wait ? portMAX_DELAY : 0);
    return ret == pdTRUE;
}

void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex) {
    xSemaphoreGive(mutex->handle);
    // TODO check return value
}

#endif // MICROPY_PY_THREAD
