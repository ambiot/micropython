print()
import uos, sys, machine, wireless ,time, modules, socket
from wireless import WLAN
from machine import Pin, UART, Timer, RTC, PWM, I2C, SPI, ADC, FLASH
from socket import SOCK

print("[MP]: Imported all builtin libraries")
print()
print("[MP]: Connecting to Filesystem")
sd = machine.SDCard()
vfs = uos.VfsFat(sd)
uos.mount(vfs, "/sd")
print("[MP]: Success connecting to SD card")
print()

del sd, vfs, uos
