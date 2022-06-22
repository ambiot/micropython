print()
import machine, wireless ,time, modules, socket
import uos
import sys
from wireless import WLAN
from machine import Pin, UART, Timer, RTC, PWM, I2C, SPI, ADC, FLASH
from socket import SOCK
print("[MP]: Imported all builtin libraries")
print()


print("[MP]: Connecting to Filesystem")
# Try to mount the filesystem, and format the SD card if it doesn't exist.
sd = machine.SDCard()

try:
    vfs = uos.VfsFat(sd)
    uos.mount(vfs, "/sd")
    print("[MP]: Success connecting to SD card")
    print()
except:
    print("[MP]: Creating VFS over SD card..")
    uos.VfsFat.mkfs(sd)
    vfs = uos.VfsFat(sd)
    print("[MP]: Success creating VFS over SD card..")
    uos.mount(vfs, "/sd")
    sys.print_exception()

#del uos, sd, vfs
