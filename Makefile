#!/bin/sh

DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build

TARGET = mk-disk
BOOT_IMAGE = boot.img
DISK_IMAGE = disk.img
KERNEL_IMAGE = kernel.img

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc

DEFINE_FLAGS = -D __TEST__
DEBUG_FLAGS = -g
# RELEASE_FLAGS = -O3

C_FLAGS = -c $(DEFINE_FLAGS) $(RELEASE_FLAGS) $(DEBUG_FLAGS) -I$(DIR)/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS)

EXCLUDE_SRC_DIR := grep -v build | grep -v .git | grep -v kernel

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC))) # 将.c文件加入到vpath中

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

.PHONY: all build run clean

all: build run

build: mk-dir boot-img kernel-img $(BUILD_DIR)/$(TARGET)

## 从vpath中读取所有的.c文件，逐个编译成.o文件
$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) $(C_FLAGS) -o $@ $<

# 将所有的.o文件链接成可执行文件
$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(LD) $(LD_FLAGS) $^ -o $@

boot-img: mk-dir
	nasm -o $(BUILD_DIR)/$(BOOT_IMAGE) boot.asm

kernel-img: mk-dir build-kernel
	nasm -o $(BUILD_DIR)/$(KERNEL_IMAGE) kernel.asm

build-kernel:
	cd kernel && make && cd ..

run: build
	$(BUILD_DIR)/$(TARGET)

test: build
	$(BUILD_DIR)/$(TARGET) --test

clean:
	cd kernel && make clean && cd ..
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)
