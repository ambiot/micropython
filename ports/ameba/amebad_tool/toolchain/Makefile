
.PHONY: all

OS := $(shell uname)

all:
ifeq ($(findstring CYGWIN, $(OS)), CYGWIN)
	mkdir -p cygwin
	tar -jxvf ameba_d_toolchain_cygwin-1.0.0-p1.tar.bz2 -C cygwin/
	tar -jxvf ameba_d_toolchain_cygwin-1.0.0-p2.tar.bz2 -C cygwin/
else ifeq ($(findstring Linux, $(OS)), Linux)
	mkdir -p linux
	tar -jxvf ameba_d_toolchain_linux_64-1.0.1.tar.bz2 -C linux/
else
	mkdir -p macos
	tar -jxvf ameba_d_toolchain_macos-1.0.1.tar.bz2 -C macos/
endif
