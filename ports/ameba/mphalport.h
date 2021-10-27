// empty file
#define mp_hal_delay_us_fast(us) mp_hal_delay_us(us)

/* FIXME: We need to implement mp_hal_stdio_poll at some stage */
#define mp_hal_stdio_poll(flags) (0)
