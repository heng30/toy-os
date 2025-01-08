#!/bin/sh

DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build
FS_DATA_DIR = $(DIR)/../res/fs-data
SYSTEM_CALL_LIB_DIR = $(DIR)/lib
ENTRY_FILE = entry.asm

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
REVERT_ASM_TOOL = objconv

DEFINE_FLAGS =
DEBUG_FLAGS =
RELEASE_FLAGS =

C_FLAGS = $(DEFINE_FLAGS) $(RELEASE_FLAGS) $(DEBUG_FLAGS) -fno-stack-protector -fno-pic -m32 -fno-asynchronous-unwind-tables -s -c -O0  -I$(DIR)/lib/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) -fno-builtin -Wconversion -Wextra -Wsign-compare -Wfloat-equal -Warray-bounds -fno-delete-null-pointer-checks

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS) -m elf_i386
NASM_FLAG = -w-zeroing -Ox
REVERT_ASM_TOOL_FLAG = -v0

EXCLUDE_SRC_DIR := grep -v build | grep -v backup | grep -v lib

ASM_SRC = $(shell find . -name '*.asm' | $(EXCLUDE_SRC_DIR))
vpath %.asm $(sort $(dir $(ASM_SRC))) # 将.c文件加入到vpath中

BIN_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SRC:%.asm=%.exe)))
vpath %.exe $(sort $(dir $(BIN_ASM)))

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC))) # 将.c文件加入到vpath中

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

REVERT_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ:%.o=%.revert.asm)))
vpath %.revert.asm $(sort $(dir $(REVERT_ASM))) # 将.revert.asm文件加入到vpath中

FINAL_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(REVERT_ASM:%.revert.asm=%.asm)))
vpath %.asm $(sort $(dir $(FINAL_ASM))) # 将.asm文件加入到vpath中

BIN_C = $(addprefix $(BUILD_DIR)/,$(notdir $(FINAL_ASM:%.asm=%.exe)))
vpath %.exe $(sort $(dir $(BIN_C)))

.PHONY: all build clean

all: mk-dir copy-entry-file build

# 从vpath中读取所有的.c文件，逐个编译成.o文件
$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) $(C_FLAGS) -o $@ $<

# 从vpath中读取所有的.o文件，逐个反汇编成.revert.asm文件
$(BUILD_DIR)/%.revert.asm: %.o | $(DIR)
	$(REVERT_ASM_TOOL) $(REVERT_ASM_TOOL_FLAG) -fnasm $< $@

$(BUILD_DIR)/%.asm: %.revert.asm | $(DIR)
	sed -n '/SECTION .text/,/SECTION .note.gnu.property/p' $< | sed '/SECTION/d' > $@
	sed -i '1r $(BUILD_DIR)/$(ENTRY_FILE)' $@

# 从vpath中读取所有的.asm文件，逐个编译成二进制文件
$(BUILD_DIR)/%.exe: %.asm | $(DIR)
	nasm $(NASM_FLAG) -o $@ $<
	- cp -f $@ $(FS_DATA_DIR)

build: $(OBJ) $(REVERT_ASM) $(FINAL_ASM) $(BIN_ASM) $(BIN_C)

clean:
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)

copy-entry-file:
	- cp -f $(SYSTEM_CALL_LIB_DIR)/$(ENTRY_FILE) $(BUILD_DIR)
	@sed -i 's/ppm.inc/lib\/ppm.inc/g' $(BUILD_DIR)/$(ENTRY_FILE)
	@sed -i 's/api.asm/lib\/api.asm/g' $(BUILD_DIR)/$(ENTRY_FILE)

move-map-symbols-files:
	mv -f $(DIR)/*.map $(BUILD_DIR)
