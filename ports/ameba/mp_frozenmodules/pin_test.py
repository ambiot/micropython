import time
from machine import Pin

print("Start of the Pin test")
print("This test only meant for AMB23(RTL8722DM-MINI)")

p = Pin("PA_10", Pin.OUT)

for i in range(100):
    time.sleep_ms(250)
    p.toggle()

print("End of the Pin test")

