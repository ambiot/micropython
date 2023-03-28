print()
import machine, wireless ,time, modules, socket
import os
import sys
from wireless import WLAN
from machine import Pin, UART, Timer, RTC, PWM, I2C, SPI, WDT, ADC, FLASH
from socket import socket

print("[MP]: Imported all builtin libraries")
print()


print("[MP]: Connecting to Filesystem")


# Try to mount the filesystem, and format the flash if it doesn't exist.
bdev = machine.FLASH()

try: 
    vfs = os.VfsFat(bdev)
    os.mount(vfs, "/")
    print("[MP]: Mounted on FLASH '/'")
except:
    print("[MP]: Creating VFS over FLASH..")
    os.VfsFat.mkfs(bdev)
    vfs = os.VfsFat(bdev)
    print("[MP]: Created VFS over FLASH")
    os.mount(vfs, "/")
    
del bdev, vfs


