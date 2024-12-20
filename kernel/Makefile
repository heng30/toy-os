DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build
TARGET = ckernel
TARGET_OBJ = $(TARGET).o

BOOT_IMAGE = boot.img
DISK_IMAGE = disk.img
KERNEL_IMAGE = kernel.img

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
REVERT_ASM_TOOL = objconv

DEFINE_FLAGS = #-D __DEBUG__ __USE_CALLBAK_MOVING_FUNCTION__ __MULTI_TASK_DEBUG__  __RING_TEST__
DEBUG_FLAGS =
RELEASE_FLAGS =

C_FLAGS = $(DEFINE_FLAGS) $(RELEASE_FLAGS) $(DEBUG_FLAGS) -fno-pic -m32 -fno-asynchronous-unwind-tables -s -c -O0  -I$(DIR)/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) -fno-builtin -Wconversion -Wextra -Wsign-compare -Wfloat-equal -Warray-bounds -fno-delete-null-pointer-checks

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS) -m elf_i386
NASM_FLAG = -w-zeroing
REVERT_ASM_TOOL_FLAG = -v0

EXCLUDE_SRC_DIR := grep -v build | grep -v backup

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC))) # 将.c文件加入到vpath中

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

REVERT_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ:%.o=%.revert.asm)))
vpath %.revert.asm $(sort $(dir $(REVERT_ASM))) # 将.revert.asm文件加入到vpath中

FINAL_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(REVERT_ASM:%.revert.asm=%.asm)))
vpath %.asm $(sort $(dir $(FINAL_ASM))) # 将.asm文件加入到vpath中

.PHONY: all build clean

all: mk-dir build

# 从vpath中读取所有的.c文件，逐个编译成.o文件
$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) $(C_FLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET_OBJ): $(OBJ)
	$(LD) $(LD_FLAGS) -r $^ -o $@

# 从vpath中读取所有的.o文件，逐个反汇编成.revert.asm文件
$(BUILD_DIR)/%.revert.asm: %.o | $(DIR)
	$(REVERT_ASM_TOOL) $(REVERT_ASM_TOOL_FLAG) -fnasm $< $@

$(BUILD_DIR)/%.asm: %.revert.asm | $(DIR)
	sed -n '/SECTION .text/,/SECTION .note.gnu.property/p' $< | sed '/SECTION/d' > $@

boot-img:
	nasm -o $(BUILD_DIR)/$(BOOT_IMAGE) boot.asm

kernel-img:
	nasm $(NASM_FLAG) -o  $(BUILD_DIR)/$(KERNEL_IMAGE) kernel.asm

pack-build-files:
	tar -zcvf build/kernel.tar.gz build/*

build: $(OBJ) $(REVERT_ASM) $(FINAL_ASM) $(BUILD_DIR)/$(TARGET).revert.asm $(BUILD_DIR)/$(TARGET).asm pack-build-files
	# boot-img kernel-img

clean:
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)
