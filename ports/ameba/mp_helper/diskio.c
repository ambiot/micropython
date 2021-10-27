#include "py/mpconfig.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "lib/oofatfs/ff.h"

#include "timeutils.h"
#include "rtc_api.h"

DWORD get_fattime (void)
{
    timeutils_struct_time_t tm;
    timeutils_seconds_since_2000_to_struct_time(rtc_read(), &tm);

    return (((DWORD)(tm.tm_year - 1980) << 25) | ((DWORD)tm.tm_mon << 21) | ((DWORD)tm.tm_mday << 16) |
           ((DWORD)tm.tm_hour << 11) | ((DWORD)tm.tm_min << 5) | ((DWORD)tm.tm_sec >> 1));
}
