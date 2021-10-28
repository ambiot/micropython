#include "ff.h"
#include <fatfs_ext/inc/ff_driver.h>
#include <sdcard.h>
#include "flash_api.h"
#include <flash_fatfs.h>
#include "py/mpconfig.h"
#include "py/runtime.h"
#include "py/stream.h"

bool init_sd_fs(void);

extern const mp_obj_type_t sdfs_type;

typedef struct {
    mp_obj_base_t base;
    uint8_t       unit;
} sdfs_obj_t;