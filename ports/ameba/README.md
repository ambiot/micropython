# MicroPython Ameba Documentation

> This port is currently only for Realtek's RTL8722DM microcontroller!

Realtek's RTL8722DM is a ARM Cortex-M33 based, dual-band WiFi (2.4G Hz and 5G Hz) and BLE 5.0 capable microntroller that is ideal for many IoT applications.

This MicroPython port is for Ameba RTL8722DM platform, details of the platform can be found here  https://www.amebaiot.com/en/amebad/


## 1. How to build firmware?

This port uses GNU ```make``` to build the firmware, make sure you have it installed and added to your PATH before proceed.

After that, you can build firmware by using the following command in the current directory.

```bash
$ make
```


Take note that this build has 2 dependencies,
1. ARM generic toolchain to be installed seperately and added into your `PATH`
2. The old `amb_micropython` repository who serve as `BSP` under the `lib` directory 

Also Note that, there are currently 2 `BOARD` supported in this SDK, they are.
1. RTL8722DM
2. RTL8722DM_MINI

Thus, in order to build firmware specific to a board, the compile time macro must be added when invoking `make` command. For example, to build for RTL8722DM_MINI, the following command shuold be used instead,

```bash
$ make BOARD=RTL8722DM_MINI
```

PS: `BOARD` macro default to `RTL8722DM` board thus no macro is required when compiling for this board.


### 1.1 FAQ
During the building process, some user may encounter error that suspend the process, this is due to missing system enevironment setup and can be fixed as follows,

#### Error related to python
By default, MicroPython use ```python3``` to run building scripts for the MicroPython kernals, if you encounter error related to python, it may be because the path of the Python3 executable is not added to system environment variable.

However, if environment variable is already added but the build couldn't be completed, you may try,
1) Re-start your PC
2) type "python" on your terminal, if the python shown is python3, then please add 
```bash
PYTHON = python
``` 
to the second line of the "Makefile" in our "port/ameba" folder

#### Error related to MPY-CROSS
If building process stop when ```mpy-cross``` shown as error, there is a step to be done as follows,
1) navigate to "mpy-cross" folder
2) Open your terminal and just type 
```bash
$ make
```
Wait for make finish building the MicroPython Cross Compiler, then this should fix the error

## 2. How to upload?

First, check your ameba Serial/COM port, make sure Ameba D's port name is correctly updated in the ```UPLOAD_PATH``` variable in the "Makefile";

Second, press RESET button while holding down UART Download button to enter ```UART Download Mode```,
Then, type following command,

```bash
$ make upload
```


## 3. How to use MicroPython ameba Port?

### 3.1 Brief introduction to MicroPython ameba port
MicroPython distinguishes itself from other compilation-based platforms (Arduino etc.) with its powerful method of real-time interaction to Microcontroller through a built-in feature --  ```REPL```. 

REPL stands for Read-Evaluation-Print-Loop, it's an interactive prompt that you can use to access and control your microcontroller.

REPL has been equipped with other powerful features such as tab completion, line editing, auto-indentation, input history and more. It basically functions like the classic Python IDLE but running on microcontroller.

To use REPL, simply open any serial terminal software (most common ones are teraterm, putty etc.) on your PC and connect to your microcontroller's serial port, then set baudrate to ```115200``` before manually reset the board, then you will see ```>>>``` MicroPython prompt appear on the console. Now you may type in any Python script on REPL as long as it's support by MicroPython and your microcontroller's MicroPython port.

Most importantly, try to abuse "help()" function as much as possible to gain more information. For example, upon microcontroller power up and REPL shown, just type

```Python
help()
```
You will see a help page giving you more details about this port; also if you type

```Python
help(modules)
```
it will list out all available builtin modules that are at your disposal

Furthermore, if you want to learn more about a module, such as its API and CONSTANT available, simply type the following code and details of that module will be returned to you,
```Python
help(the module of your interest)
```

Let's take Pin module (GPIO) as an example:
```Python
>>> help(Pin)
object <class 'Pin'> is of type type
  id -- <function>
  init -- <function>
  value -- <function>
  off -- <function>
  on -- <function>
  toggle -- <function>
  board -- <class 'board'>
  IN -- 0
  OUT -- 1
  PULL_NONE -- 0
  PULL_UP -- 1
  PULL_DOWN -- 2
```

### 3.2 REPL Hotkeys
1. Ctrl + d : Soft reboot
	MicroPython will perform software reboot, this is useful when your microcontroller is behaving abnormally. This will also run scripts in 'boot.py' once again.

	Note that this will only reset the MicroPython interpretor not the hardware, all your previously configured hardware will stay the way it is until you manually hard-reset the board.

2. Ctrl + e : Paste mode
	Paste mode allow you to perform pasting a large trunk of code into REPL at once without executing code line by line. This is useful when you have found a MicroPython library and wish to test it out immediately by copy and paste

3. Ctrl + b : Normal mode
	This hotkey will set REPL back to normal mode. This is useful if you are stuck in certain mode and can not get out.

4. Ctrl + c : Quick cancel
	This hotkey help you to cancel any input and return a new line


## 4. Peripheral Control -- machine module
MicroPython Ameba D port supports rich peripheral features through the use of ```machine``` module

### GPIO
To control GPIO, import ```Pin``` module through ```machine```. Here pin PB_18 is used as an example to output logic level 0 and 1 and blink 3 times 

```Python
from machine import Pin
a = Pin("PB_18", Pin.OUT)
a.value(1)
time.sleep_ms(500)
a.value(0)
time.sleep_ms(500)
a.on()
time.sleep_ms(500)
a.off()
time.sleep_ms(500)
a.toggle()
time.sleep_ms(500)
a.toggle()
```
Copy and paste the above code into the REPL to try it out.

#### For Your Information
The above example start with creating an object of class ```Pin```, and it has the following format
```Python
Pin("pin_name"[required], direction[required], pull_mode[optional], initial_value[optional] )
#Thus the following are all valid
a = Pin("PB_7", Pin.OUT, Pin.PULL_DOWN, value = 1)
#or this
a = Pin("PA_1", Pin.OUT, Pin.PULL_UP, value = 0)
#or this
a = Pin("PB_13", Pin.IN)
#or this
a = Pin("PA_10", Pin.OUT) # by default, pull_mode = PULL_NONE, value = 0
```
Use ```help(Pin)``` to learn more about CONSTANT and API available under this class.

PS: type the object name you just created to check its configurations, for example
```Python
>>> a = Pin("PA_26", Pin.OUT,Pin.PULL_UP, value =1)
>>> a
Pin('PA_26', mode=Pin.OUT, pull=Pin.PULL_UP) # this line is returned by MicroPython
```

### PWM
To use PWM (Pulse Width Modulation), import ```PWM``` module through ```machine```. Here pin PB_7 is used as an example to make an LED to slowly brighten up

```Python
from machine import Pin, PWM
p = PWM(pin = "PB_7")
# 0 duty cycle thus output 0
p.duty_u16 (0)
# 10% duty cycle
p.duty_u16 (6553)
# 50% duty cycle
p.duty_u16(32768)
# 100% duty cycle
p.duty_u16 (65535)
# 20 Hz at 50% duty cycle, LED blinking
p.duty_u16 (32768)
p.freq(20)
```
Copy and execute each line one by one to see LED gradually bright up and blink

#### For Your Information
The above example start with creating an object of class ```PWM```, and it has the following format
```Python
PWM(pin_name[required], unit_id[optional])
```
Use help(PWM) to view more information about this class

Note: For RTL8722DM, PWM is only supported on the following pins,
PA_23, PA_24, PA_25, PA_26

For MINI board,
PB_4, PB_5, PB_7, PA_12, PA_13, PA_23, PA_24, PA_28, PA_30



### ADC
To use ADC (Analog to Digital Convertor), import ```ADC``` module through ```machine```. Here we connect pin PB_4 to a Potentiometer to read its analogue value  

```Python
from machine import ADC
a = ADC(0)
a.read_u16()
```

#### For Your Information
The above example start with creating an object of class ```ADC```, and it has the following format
```Python
ADC( unit_id[required])
```
Use help(ADC) to view more information about this class

Note: There are 7 units of ADC pins, they are
|unit |  Pin  
|:----|:-----:
| 0   |  PB_4 
| 1   |  PB_5 
| 2   |  PB_6 
| 3   |  PB_7 
| 4   |  PB_1 
| 5   |  PB_2 
| 6   |  PB_3 



### Delay and Timing
Use the ```time``` module to take advantage of time and delays

```Python
import time
time.sleep(1)           # sleep for 1 second
time.sleep_ms(500)      # sleep for 500 milliseconds
time.sleep_us(10)       # sleep for 10 microseconds
start = time.ticks_ms() # get millisecond counter
```
Use help(time) to view more information about this class


### Timer
Use the ```Timer``` module through ```machine``` module

There are 3 sets of 32KHz General Timers available to user, Timer 1/2/3

```Python
from machine import Timer
t = Timer(1)  # Use Timer 1/2/3 only
t.init(t.PERIODIC, 2000)  # Set GTimer fired periodically at duration of 2 seconds, printing text on the terminal

# To stop the periodical timer, type 
t.deinit()
```

#### For Your Information
The above example start with creating an object of class ```Timer```, and it has the following format
```Python
Timer( unit_id[optinal] )
# if leave unit_id blank, it will take default value 1, meaning unit 1
```
Use help(Timer) to view more information about this class

Note: There are only 3 units of general timer, they are
| unit |  Timer  |  Freq
|:-----|:-------:|:-------: 
|  1   |  TIMER1 |  32 KHz 
|  2   |  TIMER2 |  32 KHz 
|  3   |  TIMER3 |  32 KHz 


### RTC
Use the ```RTC``` (Real Time Clock) module through ```machine``` module

```Python
from machine import RTC
rtc = RTC()
rtc.datetime() # get date and time 
rtc.datetime((2020, 12, 31, 4, 23, 58, 59, 0)) # set a specific date and time (year, month, day, weekday(0 for Monday), hour, minute, second, total seconds)
rtc.datetime() # check the updated date and time
```
Use help(RTC) to view more information about this class


### UART
Use the ```UART``` module through ```machine``` module

```Python
from machine import UART
uart = UART(tx="PA_21", rx= "PA_22")
uart.init()
uart.write('hello')
uart.read(5) # read up to 5 bytes
```
#### For Your Information
The above example start with creating an object of class ```UART```, and it has the following format
```Python
UART( unit_id[optional], baudrate[optional], databits[optional], paritybit[optional], stopbit[optional], tx_pin[required], rx_pin[required], timeout[optional],flowcontrol[optional] )
```
PS: Leaving all parameters except tx and rx blank will set the uart to default values which are

| Parameter | Default value
| :---------|:-------------:
| unit_id   |      0
| baudrate  |    115200
| databits  |      8
| stopbit   |      1
| paritybit |      0
| timeout   |     10 ms
| flowCtrl  |     -1

Note: There are 2 sets of uart, they are
| unit |   TX   |   RX
|:-----|:------:|:-------: 
|  0   |  PA_21 |  PA_22
|  3   |  PA_26 |  PA_25

For MINI board, they are
| unit |   TX   |   RX
|:-----|:------:|:-------: 
|  0   |  PA_21 |  PA_22
|  1   |  PB_1  |  PB_2


 Use help(UART) to view more information about this class



### I2C
Use the ```I2C``` (Inter-Integrated Circuit) module through ```machine``` module

Note: I2C only works in ```master``` mode.

```Python
from machine import Pin, I2C
i2c = I2C(scl = "PA_25", sda = "PA_26", freq=100000) # configure I2C with pins and freq. of 100KHz
i2c.scan()
i2c.writeto(8,bytes([123])) # send 1 byte to slave with address 8 with integer '123'
i2c.writeto(8,str("asf"))  # send a 3 bytes string to slave
i2c.readfrom(8, 6) # receive 6 bytes from slave
```


#### Arduino Test Code
```C
#include <Wire.h>

void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop() {
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println((char)x);         // print as char
  Serial.println(x);               // print as int
}
```

#### For Your Information
The above example start with creating an object of class ```I2C```, and it has the following format
```Python
I2C( unit_id[optional], scl_pin[required], sda_pin[required], frequency[optional])
```
PS: Leaving optional parameters blank will will assume taking default values which are

| Parameter | Default value
|:----------|:-------------: 
| unit_id   |      0
| frequency |   100000 Hz

There is currently 1 set of I2C, it is
| unit |   sda  |   scl
|:-----|:------:|:------: 
|   0  |  PA_26 |  PA_25

For MINI board,
| unit |   sda  |   scl
|:-----|:------:|:------: 
|   0  |  PB_0 |  PA_31

Use help(I2C) to view more information about this class


### SPI
Use the ```SPI``` (Serial Peripheral Interface) module through ```machine``` module


```Python
from machine import SPI
spi = SPI(1)		  # Only support 2 sets of SPI -- 0 and 1 
spi 				      # type instance name to check for details of the SPI set 
spi.write(bytes([123]))		# Write number 123 
spi.read()
```


#### Arduino Test Code
```C
#include <SPI.h>

void setup (void) {
   Serial.begin (115200);
   pinMode(MISO, OUTPUT); // have to send on master in so it set as output
   SPCR |= _BV(SPE); // turn on SPI in slave mode
   SPI.attachInterrupt(); // turn on interrupt
}
ISR (SPI_STC_vect){ // SPI interrupt routine  
   byte c = SPDR; // read byte from SPI Data Register, value range from 0 - 255
   Serial.println(c); // print as it is 
   Serial.println((char)c); // print the ascii char
}

void loop (void) {
   // do nothing
}
```


#### For Your Information
The above example start with creating an object of class ```SPI```, and it has the following format
```Python
SPI( unit_id[required], baudrate[optional], polarity[optional], phase[optional], databits[optional], firstbit[optional], miso[optional], mosi[optional], sck[optional], mode[optional])
```
PS: Leaving optional parameters blank will assume taking default values which are

| Parameter | Default value
|:----------|:-------------: 
| baudrate  |  200000 Hz
| polarity  | inactive_low
| phase     | toggle_middle
| databits  |      8
| firstbit  |     MSB
| miso      |     N.A.
| mosi      |     N.A.
| sck       |     N.A.
| mode      |    Master

There is currently 2 set of SPI, they are,
| unit |   mosi |   miso  |  SCK  |  CS
|:-----|:------:|:-------:|:-----:|:-----: 
|   0  |  PB_18 |  PB_19  | PB_20 | PB_21
|   1  |  PB_4  |  PB_5   | PB_6  | PB_7

For MINI board,
| unit |   mosi |   miso  |  SCK  |  CS
|:-----|:------:|:-------:|:-----:|:-----: 
|   1  |  PA_12 |  PA_13  | PA_14 | PA_15

Note: both unit support master mode, but ```only unit 0``` support slave mode

Use help(SPI) to view more information about this class


### FLASH
Use the ```FLASH``` (External FLASH 2MB) module through ```machine``` module

```Python
from machine import FLASH
f=FLASH() #create a FLASH object
f.read(5, 1048576) #read 5 bytes from the specific address (1048576 = 0X100000)
f.write("Hello", 1048576) #write buffer content to the specific address
f.update("Hey", 1048576) #erase the existing content and write new content to the specific address
f.read(5, 1048576)
```




## 5. Netoworking
RTL8722 MicroPython port support WiFi connection through ```WLAN``` module.

### WiFi
Ameba RTL8722 support dual-band WiFi , namely 2.4GHz and 5GHz, giving itself versatility as well as speed in transmitting data wirelessly. 

To use ```WiFi``` module, we have to import it from the ```wireless``` module, and create an object as follows,
```Python
from wireless import WLAN
wifi = WLAN(mode = WLAN.STA) # STA means Station Mode
```

#### For Your Information
The above code creates an object of class ```WLAN```, and it has the following format
```Python
WLAN( mode[required]) 
```


#### Connect to WiFi with WPA2 security type
WPA2 is the most common security type, if not sure what security type your WiFi router is configured as, use this one
```Python
from wireless import WLAN
wifi = WLAN(mode = WLAN.STA)
wifi.connect(ssid = "MPSSID", pswd = "upyameba")
```
#### For Your Information
```connect()``` API has the following format,
```Python
connect( ssid[required], pswd[optional], security[optional])
```

PS: Leaving optional parameters blank will will assume taking default values which are

| Parameter | Default value
|:----------|:-------------: 
|    pswd   |     NULL
|  security |  WPA2_AES_PSK

Note: Connecting to an ```OPEN``` network is also supported, just omit 'pswd' parameter and type in "security = WLAN.OPEN" 


#### Scan network
scanning nearby available networks can be done with script below
```Python
from wireless import WLAN
wifi = WLAN(mode = WLAN.STA)
wifi.scan()
```

#### <del>Access Point (AP) mode
<del>Set up Ameba as an access point is supported too, follow example below to start your own AP
Note: MCU will stay in AP mode to service connections thus will not return to REPL
```Python
from wireless import WLAN
wifi = WLAN(mode = WLAN.AP)
wifi.start_ap(ssid = "MPSSID", pswd = "upyameba")
```

### Socket
After WiFi is set up, the best way to access the network is to use socket. Socket is like an imaginary ethernet socket by which you use to connect your PC to some server on the internet like Google or Github. Application layer protocol like HTTP and etc. are built on top of socket. Once you are given an IP address and a port number, you can start to connect to the remote device and talk to it.

import ```socket``` to use socket module

#### Client Socket
```Python
import socket
from wireless import WLAN
# first connect to WiFi
wifi = WLAN(mode = WLAN.STA)
wifi.connect(ssid = "MPSSID", pswd = "upyameba") # change the ssid and pswd to yours
# second start a socket object
s = socket.SOCK()
# third connect the client to server
s.connect("www.google.com", 80) # domain google (must be in string), port 80 (integer)
```

#### For Your Information
The above example start with creating an object of class ```SOCK```, and it has the following format
```Python
socket.SOCK( domain[optional], type[optional] )
```
PS: Leaving optional parameters blank will will assume taking default values which are

| Parameter |  Default value
|:----------|:---------------: 
|  domain   |  AF_INET(IPv4)
|   type    | SOCK_STREAM(TCP)

Use help(socket) and help(socket.SOCK) to view more information about this class



#### Server Socket
```Python
import socket
from wireless import WLAN
# first connect to WiFi
wifi = WLAN(mode = WLAN.STA)
wifi.connect(ssid = "MPSSID", pswd = "upyameba") # change the ssid and pswd to yours
# second start a socket object
s = socket.SOCK()
# third connect the client to server
port = 5000
s.bind(port) # bind the server to localhost with port 5000 (integer)
s.listen()
conn, addr = s.accept()
```

#### Echo Server and Client example
Here is an example of letting a server socket and a client socket to echo each other's msg, to use this example, you need 2 devices, one of them is ameba RTL8722, the other can be a PC or another wifi board that runs MicroPython.

##### Server Code
```Python
import socket
from wireless import WLAN
wifi = WLAN(mode = WLAN.STA)
wifi.connect(ssid = "MPSSID", pswd = "upyameba") # change the ssid and pswd to yours
s = socket.SOCK()
port = 5000
s.bind(port) 
s.listen()
conn, addr = s.accept()
while True:
data = conn.recv(1024)
conn.send(data+"from server")
```

##### Client Code
```Python
import socket
from wireless import WLAN
wifi = WLAN(mode = WLAN.STA)
wifi.connect(ssid = "MPSSID", pswd = "upyameba") # change the ssid and pswd to yours
c = socket.SOCK()
# make sure to check the server IP address and update in the next line of code
c.connect("your server IP address", 5000) 
c.send("hello world")
data = c.recv(1024)
print(data)
```


#### HTTP Website example
With socket created, we can visit an HTTP website and get whatever information from it

##### Get information from HTTP website
```Python
# use Ctrl+E to enter Paste mode, then copy and paste the following code in Paste mode
import socket
from wireless import WLAN
wifi = WLAN(mode = WLAN.STA)
wifi.connect(ssid = "MPSSID", pswd = "upyameba") # change the ssid and pswd to yours

def http_get(url):
	_, _, host, path = url.split('/', 3)
	c = socket.SOCK()
	# We are visiting MicroPython official website's test page
	c.connect(host, 80) 
	c.send(bytes('GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n' % (path, host), 'utf8'))
	while True:
		data = c.recv(100)
		if data:
			print(str(data,'utf8'), end='')
		else:
			break

http_get('http://micropython.org/ks/test.html')
```

## 6.SD Card File System
SD File System is supported on MicroPython RTL8722 port through importing the ```SDFS``` module from ```machine``` module. This module is a simplified file system with the aim to highlight SD card manipulation, thus it doesn't support virtual file system as well as virtual file object.

Note: No hardware connection is needed for RTL8722DM_mini dev. board as it comes with SD Card slot on the back of the development board, and hardware connection has been pre-configured. However, for RTL8722DM dev. board (the big one), special configuration and fresh build of firmware are needed, please consult developer for more information.


### SD Card File System Examples
Use API provided to freely navigate through the SD card and read/write files as you see fit

#### Listing and navigating through the file system
```Python
from machine import SDFS

s=SDFS()          # create an instance and mount on file system on SD card
s.listdir()       # listing the files and folders under current path
s.mkdir("test")   # create a folder named "test" under current path
s.chdir("test")   # change directory to test folder
s.pwd()           # print out present working directory(current path)
s.chdir("/")      # change directory bach to root directory
s.rm("test")      # delete the test folder
```

#### File Manipulation
```Python
from machine import SDFS

s=SDFS()                # create an instance and mount on file system on SD card
s.create("ameba.txt")   # create a file named "ameba.txt"
s.write("ameba.txt", "ameba supports sd card file system!") # write a string to the file just created
s.read("ameba.txt")     # read the content from the same file
s.rm("ameba.txt")       # delete the file
```

Note: No file open or close is needed, the API does that automatically for you.