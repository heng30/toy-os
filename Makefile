#!/bin/sh

DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build

TARGET = mk-disk
BOOT_IMAGE = boot.img
DISK_IMAGE = disk.img

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc

DEBUG_FLAGS = -g
# RELEASE_FLAGS = -O3

C_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS) -I$(DIR)/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS)

EXCLUDE_SRC_DIR := grep -v build | grep -v .git

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC))) # 将.c文件加入到vpath中

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

.PHONY: all build run clean

all: build run

build: mk-dir boot-img $(BUILD_DIR)/$(TARGET)

## 从vpath中读取所有的.c文件，逐个编译成.o文件
$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) -c $(C_FLAGS) -o $@ $<

# 将所有的.o文件链接成可执行文件
$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(LD) $(LD_FLAGS) $^ -o $@

boot-img: mk-dir
	nasm -o $(BUILD_DIR)/$(BOOT_IMAGE) boot.asm

run:
	$(BUILD_DIR)/$(TARGET)

clean:
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)
