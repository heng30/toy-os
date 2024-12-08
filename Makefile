#!/bin/sh

DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build

TARGET = mk-disk

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc

DEFINE_FLAGS = -D __TEST__
DEBUG_FLAGS = -g
# RELEASE_FLAGS = -O3

C_FLAGS = -c $(DEFINE_FLAGS) $(RELEASE_FLAGS) $(DEBUG_FLAGS) -I$(DIR)/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS)

EXCLUDE_SRC_DIR := grep -v build | grep -v .git | grep -v kernel | grep -v backup

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC))) # 将.c文件加入到vpath中

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

.PHONY: all build-font build run clean

all: run

build: mk-dir build-kernel $(BUILD_DIR)/$(TARGET)

## 从vpath中读取所有的.c文件，逐个编译成.o文件
$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) $(C_FLAGS) -o $@ $<

# 将所有的.o文件链接成可执行文件
$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(LD) $(LD_FLAGS) $^ -o $@

build-kernel:
	cd kernel && make && cd ..

build-font: mk-dir $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET) --font

run: build
	$(BUILD_DIR)/$(TARGET)

test: build
	$(BUILD_DIR)/$(TARGET) --test

clean:
	cd kernel && make clean && cd ..
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)
