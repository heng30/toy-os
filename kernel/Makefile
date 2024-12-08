DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build
TARGET = kernel
TARGET_OBJ = $(TARGET).o

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
REVERT_ASM_TOOL = objconv

DEFINE_FLAGS =
DEBUG_FLAGS =
RELEASE_FLAGS =

C_FLAGS = $(DEFINE_FLAGS) $(RELEASE_FLAGS) $(DEBUG_FLAGS) -m32 -fno-asynchronous-unwind-tables -s -c -O3 -I$(DIR)/include -I$(DIR)/../include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS) -m elf_i386

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
$(BUILD_DIR)/%.revert.asm: %.o | $(BUILD_DIR)
	$(REVERT_ASM_TOOL) -fnasm $< $@

$(BUILD_DIR)/%.asm: %.revert.asm | $(BUILD_DIR)
	sed -n '/SECTION .text/,/SECTION .note.gnu.property/p' $< | sed '/SECTION/d' > $@

build: $(OBJ) $(REVERT_ASM) $(FINAL_ASM) $(BUILD_DIR)/$(TARGET).revert.asm $(BUILD_DIR)/$(TARGET).asm

clean:
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)
