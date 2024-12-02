#!/bin/sh

DIR=${shell pwd}
BUILD_DIR=$(DIR)/build

all: disk-img

disk-img: boot-img
	gcc -o $(BUILD_DIR)/mk-disk mk-disk.c && $(BUILD_DIR)/mk-disk

boot-img: build-dir
	nasm -o $(BUILD_DIR)/boot.img boot.asm

clean:
	- rm boot.img disk.img mk-disk
	- rm -rf $(BUILD_DIR)

build-dir:
	- mkdir -p $(BUILD_DIR)

