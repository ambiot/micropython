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
#include "objtimer.h"

/*****************************************************************************
 *                              External variables
 * ***************************************************************************/

//gtimer_t mp_timer_obj;
gtimer_t mp_timer_obj[GTIMER_MAX]; // max 4 timers
static int32_t mp_timer_id = 0;

/*****************************************************************************
 *                              Inernal variables
 * ***************************************************************************/
STATIC const timer_obj_t timer_obj[4] = {
    {.base.type = &timer_type, .id = TIMER0 },
    {.base.type = &timer_type, .id = TIMER1 },
    {.base.type = &timer_type, .id = TIMER2 },
    {.base.type = &timer_type, .id = TIMER3 }
};

/*****************************************************************************
 *                              Internal functions
 * ***************************************************************************/

void timer_init0(void) {
    // Do nothng here
}

void mp_obj_timer_irq_handler(timer_obj_t *self) {
    //printf("entered timer irq\n");
    //mp_printf(&mp_plat_print, "timer interrupt running");
    mp_sched_schedule(self->callback, self);
    mp_hal_wake_main_task_from_isr();
    #if 0
    if (self->callback != mp_const_none) {
        gc_lock();
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_call_function_0(self->callback);
            nlr_pop();
        } else {
            self->callback = mp_const_none;
            mp_printf(&mp_plat_print, "Uncaught exception in callback handler");
            if (nlr.ret_val != MP_OBJ_NULL)
                mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
        }
        gc_unlock();
    }
    #endif
}

#if 0
/// \method callback(fun)
/// Set the function to be called when the timer triggers.
/// `fun` is passed 1 argument, the timer object.
/// If `fun` is `None` then the callback will be disabled.
STATIC mp_obj_t mp_timer_callback(mp_obj_t self_in, mp_obj_t callback) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (callback == mp_const_none) {
        // stop interrupt (but not timer)
        __HAL_TIM_DISABLE_IT(&self->tim, TIM_IT_UPDATE);
        self->callback = mp_const_none;
    } else if (mp_obj_is_callable(callback)) {
        __HAL_TIM_DISABLE_IT(&self->tim, TIM_IT_UPDATE);
        self->callback = callback;
        // start timer, so that it interrupts on overflow, but clear any
        // pending interrupts which may have been set by initializing it.
        __HAL_TIM_CLEAR_FLAG(&self->tim, TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(&self->tim); // This will re-enable the IRQ
        HAL_NVIC_EnableIRQ(self->irqn);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    return mp_const_none;
}
#endif

STATIC void timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    timer_obj_t *self = self_in;
    mp_printf(print, "TIMER(%d)", mp_timer_id);
}

STATIC mp_obj_t timer_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_unit };
    const mp_arg_t timer_init_args[] = {
        { MP_QSTR_unit,    MP_ARG_INT, {.u_int = 0} },
    };

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    mp_arg_val_t args[MP_ARRAY_SIZE(timer_init_args)];
    mp_arg_parse_all(n_args, all_args, &kw_args, MP_ARRAY_SIZE(args),
            timer_init_args, args);
    
    timer_obj_t *self = &timer_obj[args[ARG_unit].u_int];

    mp_timer_id = self->id;

    gtimer_init(&mp_timer_obj[mp_timer_id], mp_timer_id);

    return self;
}

#if 0
STATIC mp_obj_t timer_init(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    mp_timer_id = self->id;
    gtimer_init(&mp_timer_obj[mp_timer_id], mp_timer_id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_init_obj, timer_init);
#endif

STATIC mp_obj_t timer_deinit(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    gtimer_deinit(&mp_timer_obj[mp_timer_id]);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_deinit_obj, timer_deinit);

STATIC mp_obj_t timer_read_tick(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    uint32_t tick = gtimer_read_tick(&mp_timer_obj[mp_timer_id]);
    return mp_obj_new_int(tick);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_read_tick_obj, timer_read_tick);

STATIC mp_obj_t timer_read_us(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    uint64_t us = gtimer_read_us(&mp_timer_obj[mp_timer_id]);
    return mp_obj_new_int_from_ull(us);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_read_us_obj, timer_read_us);

STATIC mp_obj_t timer_reload(mp_obj_t self_in, mp_obj_t duration_us_in) {
    timer_obj_t *self = self_in;
    uint32_t duration_us = mp_obj_get_int(duration_us_in);
    gtimer_reload(&mp_timer_obj[mp_timer_id], duration_us);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(timer_reload_obj, timer_reload);

STATIC mp_obj_t timer_stop(mp_obj_t self_in) {
    timer_obj_t *self = self_in;
    gtimer_stop(&mp_timer_obj[mp_timer_id]);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(timer_stop_obj, timer_stop);


void test_irq_handler() {
    printf("--timer triggered. to stop: type t.stop()--\n");
}


STATIC mp_obj_t timer_start(mp_uint_t n_args, const mp_obj_t *args) {
    //enum { ARG_self, ARG_duration, ARG_callback, ARG_type };
    enum { ARG_self, ARG_duration, ARG_type };
    timer_obj_t *self = args[ARG_self];
    uint32_t duration = mp_obj_get_int(args[ARG_duration]);
#if 0
    if (!MP_OBJ_IS_FUN(args[ARG_callback]) && (args[ARG_callback] != mp_const_none))
        mp_raise_ValueError("Error function type");
#endif
    int32_t type = mp_obj_get_int(args[ARG_type]);

    if (type == TIMER_PERIODICAL) {
        //printf("IF periodical timer started\n"); 
        gtimer_start_periodical(&mp_timer_obj[mp_timer_id], duration, test_irq_handler, NULL); 
        //printf("periodical timer ended\n");    
    } else if (type == TIMER_ONESHOT) {
        //printf("ELSE IF periodical timer started\n"); 
        gtimer_start_one_shout(&mp_timer_obj[mp_timer_id], duration, test_irq_handler, NULL); 
        //printf("one shot timer ended\n");
    } else { mp_raise_ValueError("Invalid TIMER type"); }
#if 0
    else {
        printf("ELSE timer started\n"); 
        gtimer_start_periodical(&mp_timer_obj[mp_timer_id], duration, (void *)test_irq_handler2, 0);
        printf("ELSE timer ended\n");
    }
    printf("test before passing callback\n");
    self->callback = args[ARG_callback];
    printf("test at the end of timer start\n");
#endif
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(timer_start_obj, 3, 3,  timer_start);

STATIC const mp_map_elem_t timer_locals_dict_table[] = {
    // instance methods
    //{ MP_OBJ_NEW_QSTR(MP_QSTR_init),    MP_OBJ_FROM_PTR(&timer_init_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),  MP_OBJ_FROM_PTR(&timer_deinit_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick),    MP_OBJ_FROM_PTR(&timer_read_tick_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_us),      MP_OBJ_FROM_PTR(&timer_read_us_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reload),  MP_OBJ_FROM_PTR(&timer_reload_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop),    MP_OBJ_FROM_PTR(&timer_stop_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start),   MP_OBJ_FROM_PTR(&timer_start_obj) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_PERIODICAL),  MP_OBJ_NEW_SMALL_INT(TIMER_PERIODICAL) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ONESHOT),     MP_OBJ_NEW_SMALL_INT(TIMER_ONESHOT) },
};
STATIC MP_DEFINE_CONST_DICT(timer_locals_dict, timer_locals_dict_table);

const mp_obj_type_t timer_type = {
    { &mp_type_type },
    .name        = MP_QSTR_Timer,
    .print       = timer_print,
    .make_new    = timer_make_new,
    .locals_dict = (mp_obj_t)&timer_locals_dict,
};
