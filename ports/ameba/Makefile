include ../../py/mkenv.mk


# declare before py.mk
FROZEN_MANIFEST ?= manifest.py

MICROPY_VFS_FAT ?= 0  # need to strip fatfs libs out of lib_micropython.a first 
MICROPY_VFS_LFS2 ?= 1

include $(TOP)/py/py.mk

OS := $(shell uname)
CHIP := AMEBAD
BOARD := RTL8722DM
DEBUG := 1
VENDOR := ../../lib/ameba_sdk/MicroPython_RTL8722/ports/rtl8722/amebad_vendor/sdk
TOOL := amebad_tool/tools



$(info ## This build now supports Cygwin, Linux and MacOS)
$(info ## Please specify which board you are building for, currently supported boards:)
$(info    )
$(info    * RTL8722DM)
$(info    * RTL8722DM_MINI)
$(info    )
$(info ## Now building for: $(BOARD))


# Declaring tools depending on OS
# Windows Cygwin
ifeq ($(findstring CYGWIN, $(OS)), CYGWIN) 
TC_PATH := 
CROSS_COMPILE := arm-none-eabi-
AR = $(TC_PATH)$(CROSS_COMPILE)ar.exe
CC = $(TC_PATH)$(CROSS_COMPILE)gcc.exe
AS = $(TC_PATH)$(CROSS_COMPILE)as.exe
NM = $(TC_PATH)$(CROSS_COMPILE)nm.exe
LD = $(TC_PATH)$(CROSS_COMPILE)gcc.exe
GDB = $(TC_PATH)$(CROSS_COMPILE)gdb.exe
SIZE = $(TC_PATH)$(CROSS_COMPILE)size.exe
OBJCOPY = $(TC_PATH)$(CROSS_COMPILE)objcopy.exe
OBJDUMP = $(TC_PATH)$(CROSS_COMPILE)objdump.exe
STRIP = $(TC_PATH)$(CROSS_COMPILE)strip.exe
POSTBUILD = postbuild_img2_arduino_windows.exe
IMAGETOOL = amebad_image_tool.exe
PICK = pick.exe
PAD  = pad.exe
BUILDTOOL_PATH := tools/windows
UPLOAD_PATH ?= COM8
POSTBUILDTOOL_PATH := amebad_tool/tools/windows
MPY_CROSS = $(TOP)/mpy-cross/mpy-cross.exe
LINKER_SCRIPT = 
POSTBUILD_TOOLCHAIN_PATH = cygwin
else ifeq ($(findstring Linux, $(OS)), Linux)
# Linux
TC_PATH := 
CROSS_COMPILE := arm-none-eabi-
AR = $(TC_PATH)$(CROSS_COMPILE)ar
CC = $(TC_PATH)$(CROSS_COMPILE)gcc
AS = $(TC_PATH)$(CROSS_COMPILE)as
NM = $(TC_PATH)$(CROSS_COMPILE)nm
LD = $(TC_PATH)$(CROSS_COMPILE)gcc
GDB = $(TC_PATH)$(CROSS_COMPILE)gdb
SIZE = $(TC_PATH)$(CROSS_COMPILE)size
OBJCOPY = $(TC_PATH)$(CROSS_COMPILE)objcopy
OBJDUMP = $(TC_PATH)$(CROSS_COMPILE)objdump
STRIP = $(TC_PATH)$(CROSS_COMPILE)strip
POSTBUILD = postbuild_img2_arduino_linux
IMAGETOOL = amebad_image_tool
PICK = pick
PAD  = pad
BUILDTOOL_PATH := tools/linux
UPLOAD_PATH ?= /dev/ttyUSB0
POSTBUILDTOOL_PATH := amebad_tool/tools/linux
MPY_CROSS = $(TOP)/mpy-cross/mpy-cross
LINKER_SCRIPT = 
POSTBUILD_TOOLCHAIN_PATH = linux
else
# MacOS
TC_PATH := 
CROSS_COMPILE := arm-none-eabi-
AR = $(TC_PATH)$(CROSS_COMPILE)ar
CC = $(TC_PATH)$(CROSS_COMPILE)gcc
AS = $(TC_PATH)$(CROSS_COMPILE)as
NM = $(TC_PATH)$(CROSS_COMPILE)nm
LD = $(TC_PATH)$(CROSS_COMPILE)gcc
GDB = $(TC_PATH)$(CROSS_COMPILE)gdb
SIZE = $(TC_PATH)$(CROSS_COMPILE)size
OBJCOPY = $(TC_PATH)$(CROSS_COMPILE)objcopy
OBJDUMP = $(TC_PATH)$(CROSS_COMPILE)objdump
STRIP = $(TC_PATH)$(CROSS_COMPILE)strip
POSTBUILD = postbuild_img2_arduino_macos
IMAGETOOL = amebad_image_tool
PICK = pick
PAD  = pad
BUILDTOOL_PATH := tools/macos
UPLOAD_PATH ?= /dev/ttyUSB0
POSTBUILDTOOL_PATH := amebad_tool/tools/macos
MPY_CROSS = $(TOP)/mpy-cross/mpy-cross
LINKER_SCRIPT = 
POSTBUILD_TOOLCHAIN_PATH = macos
endif

include amebad.mk

include $(TOP)/py/mkrules.mk
