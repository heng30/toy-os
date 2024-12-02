#!/bin/sh

DIR = ${shell pwd}
BUILD_DIR = $(DIR)/build
TARGET = mk-disk

CC = gcc
DEBUG_FLAGS = -g
# RELEASE_FLAGS = -O3

C_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS) -I$(DIR)/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS)

EXCLUDE_SRC_DIR := grep -v build | grep -v .git

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC)))

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

.PHONY: all run clean

all: mk-dir boot-img build

$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) -c $(C_FLAGS) -o $@ $<

build: $(OBJ)
	$(CC) $(LD_FLAGS) $^ -o $(BUILD_DIR)/$(TARGET)

boot-img: mk-dir
	nasm -o $(BUILD_DIR)/boot.img boot.asm

run:
	$(BUILD_DIR)/$(TARGET)

clean:
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)


