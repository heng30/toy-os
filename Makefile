#!/bin/sh

DIR=${shell pwd}
BUILD_DIR=$(DIR)/build

DEBUG_FLAG=-g
# RELEASE_FLAG=-O3
BUILD_FLAG=$(RELEASE_FLAG) $(DEBUG_FLAG)
LD_FLAG=

all: disk-img

disk-img: boot-img
	gcc $(BUILD_FLAG) $(LD_FLAG) -o $(BUILD_DIR)/mk-disk mk-disk.c && $(BUILD_DIR)/mk-disk

boot-img: build-dir
	nasm -o $(BUILD_DIR)/boot.img boot.asm

floppy: floppy.c
	gcc $(BUILD_FLAG) $(LD_FLAG) -c -o $(BUILD_DIR)/floppy.o $^

clean:
	- rm boot.img disk.img mk-disk
	- rm -rf $(BUILD_DIR)

build-dir:
	- mkdir -p $(BUILD_DIR)

