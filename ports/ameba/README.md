MicroPython port to Ameba
==========================

This MicroPython port is for AmebaD platform, details of the platform can be found here  https://www.amebaiot.com/en/amebad/

Supported Boards are: AMB21/AMB22, AMB23, BW16/BW16-TypeC, Sparkfun Thing Plus AW-CU488 and AMB25.

Build Firmware
---------------

If you are using Windows, it is recommended to use [Windows Subsystem for Linux](https://msdn.microsoft.com/en-au/commandline/wsl/install_guide)
to build the project.

Take note that this build has 2 dependencies,
1. ARM generic toolchain to be installed seperately and added into your `PATH`
2. The old `amb_micropython` repository who serve as `BSP` under the `lib` directory 

To build board's firmware run:

```bash
$ make BOARD=AI_THINKER_BW16
```
PS: `BOARD` macro default to `AMEBA_AMB21` board thus no macro is required when compiling for this board.

Uploading the Firmware
-----------------------

First, check the Serial/COM port that Ameba board is connected to.

Secondly, press the RESET button while holding down the UART Download button to enter ```UART Download Mode```, then using the command:

```bash
$ make upload UPLOAD_PATH=<port>
```

For BW16-TypeC, Sparkfun ThingPlus AW-CU488 and AMB25, these boards support Auto Upload Mode, just use the command:

```bash
$ make upload UPLOAD_PATH=<port> AUTO_UPLOAD=Enable
```

Acessing the board
-------------------

Once the firmware is uploaded, access the MicroPython Repl via Serial/COM port, there are many ways to do this, one of it is via `mpremote`


```bash
$ mpremote connect <port>
```

Troubleshooting
----------------

During the building process, some user may encounter error that suspend the process, this is due to missing system environment setup and can be fixed as follows,

**Error related to Python**

By default, MicroPython use ```python3``` to run building scripts for the MicroPython kernals, if you encounter error related to Python, it may be because the path of the Python3
executable is not added into system environment variable.

However, if the enviroment variable is already added but the build couldn't be completed, you may try:
1) Restart the PC
2) type "python" on the terminal, if the version is python3, then please add

```bash
PYTHON = python
``` 
to the second line of the "Makefile" in "port/ameba" folder

**Error related to MPY-CROSS**

If building process stop when ```mpy-cross``` is shown as error, try the follwing steps:

1) Navigate to "mpy-cross" folder
2) Open terminal and run:

```bash
make
``` 
After building the MicroPython Cross Compiler is finished, this should fix the error 
