#include <stdint.h>

// xm debug flag, defualt to 0 to remove certain sections of code
// set to 1 to restore to orginal code
#define xmdebug                                 (0)

#define MP_HAL_CLEAN_DCACHE

// options to control how Micro Python is built
#define MICROPY_QSTR_BYTES_IN_HASH              (1)
#define MICROPY_ALLOC_PATH_MAX                  (128)
#define MICROPY_PY_THREAD                       (0)
#define MICROPY_EMIT_THUMB                      (1)
#define MICROPY_EMIT_INLINE_THUMB               (1)
#define MICROPY_PERSISTENT_CODE_LOAD            (1)
#define MICROPY_COMP_MODULE_CONST               (1)
#define MICROPY_COMP_CONST                      (1)
#define MICROPY_REPL_EVENT_DRIVEN               (0)
#define MICROPY_PY_BUILTINS_HELP                (1)
#define MICROPY_PY_BUILTINS_HELP_TEXT           ameba_mp_help_text
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN        (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN        (0)
#define MICROPY_CPYTHON_COMPAT                  (1)
#define MICROPY_ENABLE_GC                       (1)
#define MICROPY_ENABLE_SOURCE_LINE              (1)
#define MICROPY_STREAMS_NON_BLOCK               (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO         (MICROPY_ENABLE_GC)
#define MICROPY_ENABLE_COMPILER                 (1)
#define MICROPY_ENABLE_FINALISER                (1)
#define MICROPY_HELPER_REPL                     (1)
#define MICROPY_LONGINT_IMPL                    (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL                      (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_ENABLE_DOC_STRING               (1)
#define MICROPY_ERROR_REPORTING                 (MICROPY_ERROR_REPORTING_DETAILED)
#define MICROPY_REPL_AUTO_INDENT                (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF  (1)
#define MICROPY_MODULE_WEAK_LINKS               (1)
#define MICROPY_PY_BUILTINS_HELP_MODULES        (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY           (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW          (1)
#define MICROPY_PY_BUILTINS_ENUMERATE           (1)
#define MICROPY_PY_BUILTINS_FROZENSET           (1)
#define MICROPY_PY_BUILTINS_REVERSED            (1)
#define MICROPY_PY_BUILTINS_EXECFILE            (1)
#define MICROPY_PY_BUILTINS_SET                 (1)
#define MICROPY_PY_BUILTINS_SLICE               (1)
#define MICROPY_PY_BUILTINS_PROPERTY            (1)
#define MICROPY_PY_BUILTINS_TIMEOUTERROR        (1)
#define MICROPY_PY___FILE__                     (1)
#define MICROPY_ENABLE_SCHEDULER                (1)
#define MICROPY_PY_GC                           (1)
#define MICROPY_PY_ARRAY                        (1)
#define MICROPY_PY_ATTRTUPLE                    (1)
#define MICROPY_PY_COLLECTIONS                  (1)
#define MICROPY_PY_WEBSOCKET                    (0)
#define MICROPY_PY_WEBREPL_DELAY                (20)
#define MICROPY_PY_MATH                         (1)
#define MICROPY_PY_IO                           (1)
#define MICROPY_PY_IO_FILEIO                    (0)
#define MICROPY_PY_UCTYPES                      (1)
#define MICROPY_PY_UHEAPQ                       (1)
#define MICROPY_PY_UJSON                        (0)
#define MICROPY_PY_UZLIB                        (1)
#define MICROPY_PY_UBINASCII                    (1)
#define MICROPY_PY_URE                          (1)
//#define MICROPY_PY_USSL_FINALISER               (1)
#define MICROPY_PY_STRUCT                       (1)
#define MICROPY_PY_SYS                          (1)
#define MICROPY_PY_SYS_MODULES                  (0)
#define MICROPY_PY_SYS_STDFILES                 (1)
// extmod machine modules
#define MICROPY_PY_MACHINE                      (1)
#define MICROPY_PY_MACHINE_SPI                  (1)
#define MICROPY_PY_MACHINE_SPI_MSB              (0) // const only
#define MICROPY_PY_MACHINE_SPI_LSB              (1) // const only
#define MICROPY_PY_MACHINE_I2C                  (1)
#define MICROPY_PY_MACHINE_PWM                  (1)
#define MICROPY_PY_MACHINE_PWM_DUTY_U16_NS      (1)
#define MICROPY_PY_MACHINE_PWM_INCLUDEFILE      "ports/ameba/mp_helper/mods/machine/objpwm.c"

#define MICROPY_PY_UERRNO                       (1)
#define MICROPY_PY_SYS_EXIT                     (1)
#define MICROPY_PY_USELECT                      (1)
#define MICROPY_PY_UTIMEQ                       (1)
#define MICROPY_PY_URANDOM                      (1)
#define MICROPY_PY_FRAMEBUF                     (1)
#define MICROPY_PY_BUILTINS_FLOAT               (1)
#define MICROPY_PY_UTIME_MP_HAL                 (1)
#define MICROPY_MODULE_FROZEN_STR               (1)
#define MICROPY_MODULE_FROZEN_MPY               (1)
#define MICROPY_QSTR_EXTRA_POOL                 mp_qstr_frozen_const_pool
#define MICROPY_USE_INTERNAL_PRINTF				(0) // xmdebug // disable upy internal printf
#define MICROPY_KBD_EXCEPTION                   (1)
#define MICROPY_PY_TERM_NUM                     (3)
#define MICROPY_PY_WEBREPL                      (0)
#define GENERIC_ASM_API                         (0)
#define MICROPY_NLR_SETJMP                      (0)
#define MICROPY_PY_LWIP_SLIP                    (0)
#define MICROPY_PY_OS_DUPTERM                   (0)
#define MICROPY_PY_UOS_DUPTERM_BUILTIN_STREAM   (0)
#define MICROPY_NLR_X64                         (0)

#define MICROPY_READER_VFS                      (MICROPY_VFS)
#define MICROPY_VFS                             (1)
#define MICROPY_VFS_FAT                         (0)
#define MICROPY_READER_FATFS                    (MICROPY_VFS)
#define MICROPY_FATFS_ENABLE_LFN                (1)
#define MICROPY_FATFS_LFN_CODE_PAGE             (437) /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_FATFS_VOLUMES                   (1)
#define MICROPY_FATFS_RPATH                     (2)
#define MICROPY_FATFS_MAX_SS                    (512)
#define MICROPY_FATFS_USE_LABEL                 (1)

/*
// File System
#define CONFIG_FATFS_EN 1
#if CONFIG_FATFS_EN
// fatfs version
#define FATFS_R_10C
// fatfs disk interface
#define FATFS_DISK_USB  0
#define FATFS_DISK_SD   1
#define FATFS_DISK_FLASH    0
#endif
*/

#include "rtl8721d.h"

// use vfs's functions for import stat and builtin open
#define mp_import_stat mp_vfs_import_stat
#define mp_builtin_open mp_vfs_open
#define mp_builtin_open_obj mp_vfs_open_obj

#define MICROPY_EVENT_POLL_HOOK \
    do { \
        /* extern void mp_handle_pending(void); */ \
        mp_handle_pending(true); \
    } while(0); \


// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_input), MP_OBJ_FROM_PTR(&mp_builtin_input_obj) }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR_open),  MP_OBJ_FROM_PTR(&mp_builtin_open_obj) },  \

extern const struct _mp_obj_module_t mp_module_modules;
extern const struct _mp_obj_module_t mp_module_umachine;
extern const struct _mp_obj_module_t mp_module_uos;
extern const struct _mp_obj_module_t mp_module_utime;
extern const struct _mp_obj_module_t mp_module_uwireless;
extern const struct _mp_obj_module_t mp_module_usocket;
#if 0
extern const struct _mp_obj_module_t mp_module_uterminal;
extern const struct _mp_obj_module_t mp_module_ussl;
extern const struct _mp_obj_module_t mp_network_module;
#endif


#define MICROPY_PORT_BUILTIN_MODULES \
    { MP_OBJ_NEW_QSTR(MP_QSTR_modules),      MP_OBJ_FROM_PTR(&mp_module_modules) },    \
    { MP_OBJ_NEW_QSTR(MP_QSTR_umachine),     MP_OBJ_FROM_PTR(&mp_module_umachine) },   \
    { MP_OBJ_NEW_QSTR(MP_QSTR_uos),          MP_OBJ_FROM_PTR(&mp_module_uos) },        \
    { MP_OBJ_NEW_QSTR(MP_QSTR_utime),        MP_OBJ_FROM_PTR(&mp_module_utime) },      \
    { MP_OBJ_NEW_QSTR(MP_QSTR_uwireless),    MP_OBJ_FROM_PTR(&mp_module_uwireless) },  \
    { MP_OBJ_NEW_QSTR(MP_QSTR_usocket),      MP_OBJ_FROM_PTR(&mp_module_usocket) },    \
/*    { MP_OBJ_NEW_QSTR(MP_QSTR_ussl),         MP_OBJ_FROM_PTR(&mp_module_ussl) },    \
    { MP_OBJ_NEW_QSTR(MP_QSTR_network),      MP_OBJ_FROM_PTR(&mp_network_module) },    \
    { MP_OBJ_NEW_QSTR(MP_QSTR_uterminal),    MP_OBJ_FROM_PTR(&mp_module_uterminal) },   \
*/

#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_module),      MP_OBJ_FROM_PTR(&mp_module_modules) },  \
    { MP_OBJ_NEW_QSTR(MP_QSTR_machine),     MP_OBJ_FROM_PTR(&mp_module_umachine) },  \
    { MP_OBJ_NEW_QSTR(MP_QSTR_os),          MP_OBJ_FROM_PTR(&mp_module_uos) },         \
    { MP_OBJ_NEW_QSTR(MP_QSTR_time),        MP_OBJ_FROM_PTR(&mp_module_utime) },       \
    { MP_OBJ_NEW_QSTR(MP_QSTR_wireless),    MP_OBJ_FROM_PTR(&mp_module_uwireless) },  \
    { MP_OBJ_NEW_QSTR(MP_QSTR_socket),    MP_OBJ_FROM_PTR(&mp_module_usocket) },  \
/*    { MP_OBJ_NEW_QSTR(MP_QSTR_json),        MP_OBJ_FROM_PTR(&mp_module_ujson) },    \
    { MP_OBJ_NEW_QSTR(MP_QSTR_errno),       MP_OBJ_FROM_PTR(&mp_module_uerrno) },   \
    { MP_OBJ_NEW_QSTR(MP_QSTR_select),      MP_OBJ_FROM_PTR(&mp_module_uselect) },  \
*/

#define MICROPY_PY_SYS_PLATFORM             "Realtek Ameba"

#define MICROPY_HW_PORT_VERSION             "1.0.2"

#define MICROPY_HW_BOARD_NAME               MICROPY_PY_SYS_PLATFORM

#if defined(RTL8722DM)
#define MICROPY_HW_MCU_NAME                 "RTL8722DM"
#elif defined(RTL8722DM_MINI)
#define MICROPY_HW_MCU_NAME                 "RTL8722DM_MINI"
#else
#error "Please specify the correct board name before re-try"
#endif 

#define MICROPY_WLAN_AP_DEFAULT_SSID        "YourSSID"
#define MICROPY_WLAN_AP_DEFAULT_PASS        "YourPSWD"

// Heap size for Garbage Collector
#define MP_HEAP_SIZE                        (16 * 1024)
//#define MP_HEAP_SIZE                        (180 * 1024) // xxm
//#define MP_HEAP_SIZE                        (1124 * 1024)

#define MICROPY_TASK_NAME                   "MicroPython"
#define MICROPY_TASK_STACK_DEPTH            (((20 * 1024) + 512) / sizeof(StackType_t))
#define MICROPY_TASK_PRIORITY               (3) // 3 for Realtime, the highest priority

#define MICROPY_NETWORK_CORE_STACK_NAME     "TCPIP"
#define MICROPY_NETWORK_CORE_STACK_DEPTH    (10 * 1024) + 0
#define MICROPY_NETWORK_CORE_STACK_PRIORITY (configMAX_PRIORITIES - 1)

#define MICROPY_TERM_RX_STACK_NAME          "TERMRX"
#define MICROPY_TERM_RX_STACK_DEPTH         (((10 * 1024) + 0) / sizeof(StackType_t))
#define MICROPY_TERM_RX_STACK_PRIORITY      (MICROPY_TASK_PRIORITY)

// These root pointers prevent GC from wiping the buffers pointed by these pointer while we still need them 
#define MICROPY_PORT_ROOT_POINTERS          \
    const char *readline_hist[8];           \
    vstr_t *repl_line;                      \
    mp_obj_list_t  term_list_obj;           \
    mp_obj_t dupterm_arr_obj;               \
    mp_obj_t log_uart_rx_chr_obj;           \
    mp_map_t mp_terminal_map;


#define mp_type_fileio mp_type_vfs_fat_fileio
#define mp_type_textio mp_type_vfs_fat_textio // xxm

// type definitions for the specific machine

#define BYTES_PER_WORD (4)

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define UINT_FMT "%lu"
#define INT_FMT "%ld"

#define MP_HAL_PIN_FMT "%u"

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef void *machine_ptr_t; // must be of pointer size
typedef const void *machine_const_ptr_t; // must be of pointer size
typedef long mp_off_t;

#define MP_STATE_PORT MP_STATE_VM
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

#ifdef UINT
#undef UINT
#endif
