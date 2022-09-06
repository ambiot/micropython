print()
import machine, wireless ,time, modules, socket
import os
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
    vfs = os.VfsFat(sd)
    #os.mount(vfs, "/sd")
    os.mount(vfs, "/")
    print("[MP]: Success connecting to SD card")
    print()
except:
    print("[MP]: Creating VFS over SD card..")
    os.VfsFat.mkfs(sd)
    vfs = os.VfsFat(sd)
    print("[MP]: Success creating VFS over SD card..")
    #os.mount(vfs, "/sd")
    os.mount(vfs, "/")
    sys.print_exception()

del sd, vfs
