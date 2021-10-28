.PHONY: all

OS := $(shell uname)

all:
ifeq ($(findstring CYGWIN, $(OS)), CYGWIN)
	mkdir -p windows
	tar -xvf postbuildTool_windows.tar -C windows/
else ifeq ($(findstring Linux, $(OS)), Linux)
	mkdir -p linux
	tar -xvf postbuildTool_linux.tar -C linux/
else
	mkdir -p macos
	tar -xvf postbuildTool_macos-1v05.tar -C macos/
endif