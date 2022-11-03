include ../../py/mkenv.mk


# declare before py.mk
FROZEN_MANIFEST ?= manifest.py

MICROPY_VFS_FAT ?= 1
MICROPY_VFS_LFS2 ?= 1

include $(TOP)/py/py.mk

OS := $(shell uname)
CHIP := AMEBAD
BOARD := RTL8722DM_MINI
DEBUG := 1
VENDOR := ../../lib/ameba_sdk/sdk
TOOL := amebad_tool/tools



$(info ## This build now supports Cygwin, Linux and MacOS)
$(info ## Please specify which board you are building for, currently supported boards:)
$(info    )
$(info    * RTL8722DM)
$(info    * RTL8722DM_MINI)
$(info    * BW16)
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
UPLOAD_PATH ?= /dev/ttyS5
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



################################
#        Include path          # 
################################
INC += -I.
INC += -I$(VENDOR)/project/realtek_amebaD_va0_example/inc/inc_hp
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(HEADER_BUILD)
INC += -I$(VENDOR)/component/os/freertos
INC += -I$(VENDOR)/component/os/freertos/freertos_v10.2.0/Source/include
INC += -I$(VENDOR)/component/os/freertos/freertos_v10.2.0/Source/portable/GCC/ARM_CM33/non_secure
INC += -I$(VENDOR)/component/os/os_dep/include
INC += -I$(VENDOR)/component/soc/realtek/amebad/misc
INC += -I$(VENDOR)/component/common/api/network/include
INC += -I$(VENDOR)/component/common/api
INC += -I$(VENDOR)/component/common/api/at_cmd
INC += -I$(VENDOR)/component/common/api/platform
INC += -I$(VENDOR)/component/common/api/wifi
INC += -I$(VENDOR)/component/common/api/wifi/rtw_wpa_supplicant/src
INC += -I$(VENDOR)/component/common/api/wifi/rtw_wpa_supplicant/src/crypto
INC += -I$(VENDOR)/component/common/application
INC += -I$(VENDOR)/component/common/media/framework
INC += -I$(VENDOR)/component/common/example
INC += -I$(VENDOR)/component/common/example/wlan_fast_connect
INC += -I$(VENDOR)/component/common/mbed/api
INC += -I$(VENDOR)/component/common/mbed/hal
INC += -I$(VENDOR)/component/common/mbed/hal_ext
INC += -I$(VENDOR)/component/common/mbed/targets/hal/rtl8721d
INC += -I$(VENDOR)/component/common/network
INC += -I$(VENDOR)/component/common/network/lwip/lwip_v2.0.2/port/realtek/freertos
INC += -I$(VENDOR)/component/common/network/lwip/lwip_v2.0.2/src/include
INC += -I$(VENDOR)/component/common/network/lwip/lwip_v2.0.2/src/include/lwip
INC += -I$(VENDOR)/component/common/network/lwip/lwip_v2.0.2/port/realtek
INC += -I$(VENDOR)/component/common/test
INC += -I$(VENDOR)/component/soc/realtek/amebad/cmsis
INC += -I$(VENDOR)/component/soc/realtek/amebad/fwlib
INC += -I$(VENDOR)/component/soc/realtek/amebad/misc
INC += -I$(VENDOR)/component/common/drivers/wlan/realtek/include
INC += -I$(VENDOR)/component/common/drivers/wlan/realtek/src/osdep
INC += -I$(VENDOR)/component/common/drivers/wlan/realtek/src/hci
INC += -I$(VENDOR)/component/common/network/ssl/polarssl-1.3.8/include
INC += -I$(VENDOR)/component/common/network/ssl/ssl_ram_map/rom
INC += -I$(VENDOR)/component/common/utilities
INC += -I$(VENDOR)/component/common/video/v4l2/inc
INC += -I$(VENDOR)/component/common/media/rtp_codec
INC += -I$(VENDOR)/component/common/file_system/fatfs
INC += -I$(VENDOR)/component/common/file_system/fatfs/disk_if/inc
INC += -I$(VENDOR)/component/common/file_system/fatfs/r0.10c/include
INC += -I$(VENDOR)/component/common/file_system/ftl
INC += -I$(VENDOR)/component/common/drivers/sdio/realtek/sdio_host/inc
INC += -I$(VENDOR)/component/common/audio
INC += -I$(VENDOR)/component/common/drivers/i2s
INC += -I$(VENDOR)/component/common/application/xmodem
INC += -I$(VENDOR)/component/common/network/mDNS
INC += -I$(VENDOR)/component/soc/realtek/amebad/fwlib/include
INC += -I$(VENDOR)/component/soc/realtek/amebad/swlib/string
INC += -I$(VENDOR)/component/soc/realtek/amebad/app/monitor/include
INC += -I$(VENDOR)/component/soc/realtek/amebad/app/xmodem
INC += -I$(VENDOR)/component/common/network/ssl/mbedtls-2.4.0/include
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/board/amebad/lib
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/board/amebad/src
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/board/amebad/src/vendor_cmd
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/board/common/inc
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/example/ble_scatternet
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/app
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/gap
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/profile
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/profile/client
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/bluetooth/profile/server
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/os
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/platform
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/inc/stack
INC += -I$(VENDOR)/component/common/bluetooth/realtek/sdk/src/mcu/module/data_uart_cmd
INC += -I$(VENDOR)/component/common/drivers/ir/protocol
INC += -I$(VENDOR)/component/common/drivers/wlan/realtek/src/core/option
INC += -I$(VENDOR)/component/common/drivers/wlan/realtek/src/hal
INC += -I$(VENDOR)/component/common/drivers/wlan/realtek/src/hal/phydm

INC += -I$(TOP)/extmod
#INC += -I$(TOP)/lib/utils
INC += -I$(TOP)/shared/timeutils
INC += -I$(TOP)/shared/readline
INC += -I$(TOP)/shared/netutils
INC += -I$(TOP)/shared/runtime
#INC += -I$(TOP)/lib/lwip/src/include/lwip

INC += -Imp_helper
INC += -Imp_helper/mods
INC += -Imp_helper/mods/network
INC += -Imp_helper/mods/machine


# Micropython Port Source file list
# -------------------------------------------------------------------
UPY_C += pins.c
UPY_C += mphalport.c
UPY_C += mp_helper/diskio.c
# UPY_C += mp_helper/flashbdev.c
UPY_C += mp_helper/exception.c
UPY_C += mp_helper/help.c
UPY_C += mp_helper/input.c
UPY_C += mp_helper/bufhelper.c
#UPY_C += mp_helper/mpthreadport.c
UPY_C += mp_helper/gccollect.c
UPY_C += mp_helper/mods/modameba.c
UPY_C += mp_helper/mods/modmachine.c
UPY_C += mp_helper/mods/wireless/objwlan.c
UPY_C += mp_helper/mods/moduwireless.c
UPY_C += mp_helper/mods/modsocket.c
#UPY_C += mp_helper/mods/modnetwork.c
UPY_C += mp_helper/mods/modutime.c
#UPY_C += mp_helper/mods/modterm.c
#UPY_C += mp_helper/mods/modussl.c
#UPY_C += mp_helper/mods/machine/objwdt.c
UPY_C += mp_helper/mods/machine/objflash.c
UPY_C += mp_helper/mods/machine/objrtc.c
UPY_C += mp_helper/mods/machine/objadc.c
##UPY_C += mp_helper/mods/machine/objdac.c
UPY_C += mp_helper/mods/machine/objpin.c
UPY_C += mp_helper/mods/machine/obji2c.c
UPY_C += mp_helper/mods/machine/objpwm.c
UPY_C += mp_helper/mods/machine/objtimer.c
UPY_C += mp_helper/mods/machine/objspi.c
UPY_C += mp_helper/mods/machine/objuart.c
UPY_C += shared/runtime/pyexec.c
UPY_C += shared/readline/readline.c
UPY_C += shared/runtime/interrupt_char.c
UPY_C += shared/timeutils/timeutils.c
UPY_C += shared/runtime/sys_stdio_mphal.c
UPY_C += shared/netutils/netutils.c

# File System
#UPY_C += lib/oofatfs/ff.c 
# UPY_C += $(VENDOR)/component/common/file_system/fatfs/fatfs_ext/src/ff_driver.c
# UPY_C += $(VENDOR)/component/common/file_system/fatfs/r0.10c/src/diskio.c
# UPY_C += $(VENDOR)/component/common/file_system/fatfs/r0.10c/src/ff.c
# UPY_C += $(VENDOR)/component/common/file_system/fatfs/disk_if/src/flash_fatfs.c
# UPY_C += $(VENDOR)/component/common/file_system/fatfs/disk_if/src/sdcard.c
UPY_C += mp_helper/mods/machine/objsdcard.c

# main
UPY_C += main.c


# Initialize target name and target object files
# -------------------------------------------------------------------

all: application manipulate_images

TARGET=application


# Generate obj list
# -------------------------------------------------------------------

UPY_O = $(addprefix $(BUILD)/, $(UPY_C:.c=.o))
UPY_O += $(addprefix $(BUILD)/, $(SRC_MOD:.c=.o))
UPY_O += $(addprefix $(BUILD)/, $(SRC_LIB:.c=.o))

OBJ = $(UPY_O) $(PY_O)
SRC_QSTR += $(UPY_C) $(SRC_LIB) $(SRC_MOD)
SRC_QSTR_AUTO_DEPS +=

################################
# 			CFLAGS 			   #
################################

# Optimize level
CFLAGS = -O2

# CPU arch
CFLAGS += -march=armv8-m.main+dsp
CFLAGS += -mthumb 
CFLAGS += -D$(CHIP)
CFLAGS += -D$(BOARD)
CFLAGS += -DMICROPYTHON_RTL8721D
CFLAGS += -DCONFIG_PLATFORM_8721D
# source code macro
CFLAGS += -ffunction-sections -mcmse -mfloat-abi=hard -mfpu=fpv5-sp-d16 -g -gdwarf-3 -Wno-unused-parameter
CFLAGS += -nostartfiles -nodefaultlibs -nostdlib -O2 -D__FPU_PRESENT -gdwarf-3 -fstack-usage 
CFLAGS += -fdata-sections -nostartfiles -nostdlib -Wall -Wpointer-arith -Wstrict-prototypes 
#CFLAGS += -Wundef -Wno-write-strings -Wno-maybe-uninitialized -c -MMD -Wextra 
CFLAGS += -Wno-write-strings -Wno-maybe-uninitialized -c -MMD -Wextra 
CFLAGS += -Wl,--start-group
CFLAGS += $(INC)
CFLAGS += -Wl,--end-group
CFLAGS += $(CFLAGS_MOD)


###########################
#         LDFLAGS         #
###########################
LFLAGS =
LFLAGS += -O2 -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=hard -mfpu=fpv5-sp-d16 
LFLAGS += -nostartfiles -specs nosys.specs -Wl,--gc-sections

LIBFLAGS = -Wl,--no-enum-size-warning -Wl,--warn-common -Wl,--print-memory-usage


###############################
#         ARCHIVE LIST        #
###############################
LIBAR += -Wl,--start-group
LIBAR += -L$(VENDOR)/../lib/ -l_micropython -l_wlan -l_wps 
LIBAR += -l_websocket -l_user -l_usbh -l_usbd -l_tftp -l_mdns -l_m4a_self 
LIBAR += -l_httpd -l_httpc -l_eap -l_dct -l_coap -l_cmsis_dsp -l_arduino_bt -l_arduino_mbedtls240
LIBAR += -Wl,--end-group


###########################
#         MAKE RULES      #
###########################
application: prerequirement $(OBJ)
	$(Q)echo '==========================================================='
	$(Q)echo 'Linking object code'
	$(Q)echo '==========================================================='
	$(LD) -L$(LINKER_SCRIPT) -L$(TC_PATH)../lib -T$(LINKER_SCRIPT)rlx8721d_img2_is_micropython.ld $(LFLAGS) -Wl,-Map=$(BUILD)/Preprocessed_image2.map $(LIBFLAGS) -o $(BUILD)/$(TARGET).axf $(OBJ) $(LIBAR) -lm
	$(Q)$(OBJDUMP) -d $(BUILD)/$(TARGET).axf > $(BUILD)/Preprocessed_image2.asm


.PHONY: prerequirement
prerequirement: check_toolchain check_postbuildtools submodules
	$(Q)echo '==========================================================='
	$(Q)echo 'Prepare tools and images'
	$(Q)echo '==========================================================='
	$(Q)$(MKDIR) -p $(BUILD)/bsp/image
	$(Q)cp -f $(TOOL)/image/km0_boot_all.bin $(BUILD)/bsp/image/km0_boot_all.bin
	$(Q)chmod +rw $(BUILD)/bsp/image/km0_boot_all.bin
	$(Q)cp -f $(TOOL)/image/km0_image2_all.bin $(BUILD)/bsp/image/km0_image2_all.bin
	$(Q)chmod +rw $(BUILD)/bsp/image/km0_image2_all.bin
	$(Q)cp -f $(TOOL)/image/km4_boot_all.bin $(BUILD)/bsp/image/km4_boot_all.bin
	$(Q)chmod +rw $(BUILD)/bsp/image/km4_boot_all.bin
	$(Q)mkdir -p $(BUILD)/$(BUILDTOOL_PATH)
	$(Q)cp -f $(POSTBUILDTOOL_PATH)/$(PICK) $(BUILD)/$(BUILDTOOL_PATH)/$(PICK)
	$(Q)cp -f $(POSTBUILDTOOL_PATH)/$(PAD) $(BUILD)/$(BUILDTOOL_PATH)/$(PAD)
	#$(Q)echo "Delete diskio from the static micropython library."
	#ar dv $(VENDOR)/../lib/lib_micropython.a diskio.o 2> /dev/null


.PHONY: check_toolchain
check_toolchain:
	@if command -v arm-none-eabi-gcc; \
		then echo "--ToolChain Exists--"; \
		else echo "--Toolchain Not Installed/added to PATH--"; \
			exit 1; fi

.PHONY: check_postbuildtools
check_postbuildtools:
	@if [ -d $(POSTBUILDTOOL_PATH) ]; \
		then echo "--Postbuildtool Exists--"; \
		else echo "--Extracting tools..."; \
			make -C $(TOOL) all; fi


.PHONY: manipulate_images
manipulate_images: $(POSTBUILD)
	$(Q)echo '==========================================================='
	$(Q)echo 'Image manipulating'
	$(Q)echo '==========================================================='
	./$(BUILD)/$(POSTBUILD) $(BUILD) $(TARGET).axf " " 0
	$(Q)echo '==========================='
	$(Q)echo 'End of Image manipulating'
	$(Q)echo '==========================='


.PHONY: upload
upload: $(IMAGETOOL)
	$(Q)echo '## Make sure Ameba serial port name has been correctly updated in the Makefile!'
	./$(BUILD)/$(IMAGETOOL) $(UPLOAD_PATH)


.PHONY: purge
purge:
	make cleanpwd
	make clean
	clear


.PHONY: cleanpwd
cleanpwd:
	rm -f ./*.d
	rm -f ./*.bin


.PHONY: com
com:
	@if [ $(findstring CYGWIN, $(OS)) = CYGWIN ]; \
		then ttermpro /C=8 /BAUD=115200; \
		else mpremote connect $(UPLOAD_PATH); fi


.PHONY: release
release:
	cp -f $(BUILD)/km0_km4_image2.bin $(TOP)/../Release/Windows
	cp -f $(BUILD)/km0_km4_image2.bin $(TOP)/../Release/Linux
	cp -f $(BUILD)/km0_km4_image2.bin $(TOP)/../Release/MacOS


.PHONY: checkmem
checkmem:
	$(Q)echo
	$(Q)echo 'Summary of memory'
	$(SIZE) -A -d build/application.axf
	$(Q)echo 'Top 30 memory blocks'
	$(NM) build/application.axf --print-size --size-sort --print-armap | tail -30



######################
#     Submodule      #
######################

GIT_SUBMODULES += lib/ameba_sdk

.PHONY: submodules
submodules:
	$(MAKE) -f ../../py/mkrules.mk GIT_SUBMODULES="$(GIT_SUBMODULES)" submodules


######################
#    Compilation     #
######################
$(UPY_O): $(BUILD)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@


##############
#    Tools   #
##############
$(POSTBUILD):
	$(Q)cp -f $(POSTBUILDTOOL_PATH)/$(POSTBUILD) $(BUILD)/$(POSTBUILD)
	$(Q)rm -f ./*.d

$(IMAGETOOL):
	$(Q)cp -f $(POSTBUILDTOOL_PATH)/$(IMAGETOOL) $(BUILD)/$(IMAGETOOL)
	$(Q)cp -f $(POSTBUILDTOOL_PATH)/imgtool_flashloader_amebad.bin ./
	$(Q)cp -f $(TOOL)/image/km0_boot_all.bin ./
	$(Q)cp -f $(TOOL)/image/km0_image2_all.bin ./
	$(Q)cp -f $(TOOL)/image/km4_boot_all.bin ./
	$(Q)cp -f $(BUILD)/km0_km4_image2.bin ./


include $(TOP)/py/mkrules.mk
