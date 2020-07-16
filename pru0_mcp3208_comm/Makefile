
# Default location of Code Generation Tools and PRU Software Support Package
PRU_CGT ?=/usr/share/ti/cgt-pru
PRU_SSP ?=/usr/share/ti/pru-software-support-package

# Target remoteproc sysfs dir
RPROC = /sys/class/remoteproc/remoteproc1

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
PROJ_NAME=$(CURRENT_DIR)
LINKER_COMMAND_FILE=./AM335x_PRU.cmd
LIBS=--library=$(PRU_SSP)/lib/rpmsg_lib.lib
INCLUDE=--include_path=$(SSP_CGT)/include --include_path=$(PRU_SSP)/include \
           --include_path=$(PRU_SSP)/include/am335x

STACK_SIZE=0x100
HEAP_SIZE=0x100
GEN_DIR=gen

CFLAGS=-v3 -O2 --display_error_number --endian=little --hardware_mac=on   \
           --obj_directory=$(GEN_DIR) --pp_directory=$(GEN_DIR) -ppd -ppa \
           --asm_directory=$(GEN_DIR) --list_directory=$(GEN_DIR)         \
           --c_src_interlist --optimizer_interlist 

LFLAGS=--reread_libs --warn_sections --stack_size=$(STACK_SIZE) \
           --heap_size=$(HEAP_SIZE)

TARGET=$(GEN_DIR)/$(PROJ_NAME).out
MAP=$(GEN_DIR)/$(PROJ_NAME).map
SOURCES=$(wildcard *.c)
#Using .object instead of .obj in order to not conflict with the CCS build
OBJECTS=$(patsubst %,$(GEN_DIR)/%,$(SOURCES:.c=.object))

all: $(TARGET)

install:
	cp $(TARGET) /lib/firmware/$(PROJ_NAME)

deploy: install
	-echo stop > $(RPROC)/state
	echo $(PROJ_NAME) > $(RPROC)/firmware
	echo start > $(RPROC)/state

# Invokes the linker (-z flag) to make the .out file
$(TARGET): $(OBJECTS) $(LINKER_COMMAND_FILE)
	$(PRU_CGT)/bin/clpru $(CFLAGS) -z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $(TARGET) $(OBJECTS) -m$(MAP) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)

# Invokes the compiler on all c files in the directory to create the object files
$(GEN_DIR)/%.object: %.c
	mkdir -p $(GEN_DIR)
	$(PRU_CGT)/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

.PHONY: all clean run install

clean:
	rm -rf $(GEN_DIR)

# Includes the dependencies that the compiler creates (-ppd and -ppa flags)
-include $(OBJECTS:%.object=%.pp)

