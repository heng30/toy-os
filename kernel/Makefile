DIR = $(shell pwd)
BUILD_DIR = $(DIR)/build

CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
REVERT_ASM_TOOL = objconv

DEFINE_FLAGS =
DEBUG_FLAGS =
RELEASE_FLAGS =

C_FLAGS = $(DEFINE_FLAGS) $(RELEASE_FLAGS) $(DEBUG_FLAGS) -m32 -fno-asynchronous-unwind-tables -s -c -O3 -I$(DIR)/include -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))

LD_FLAGS = $(RELEASE_FLAGS) $(DEBUG_FLAGS)

EXCLUDE_SRC_DIR := grep -v build

C_SRC = $(shell find . -name '*.c' | $(EXCLUDE_SRC_DIR))
vpath %.c $(sort $(dir $(C_SRC))) # 将.c文件加入到vpath中

OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SRC:%.c=%.o)))
vpath %.o $(sort $(dir $(OBJ))) # 将.o文件加入到vpath中

REVERT_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ:%.o=%.revert_asm)))
vpath %.revert_asm $(sort $(dir $(REVERT_ASM))) # 将.revert_asm文件加入到vpath中

FINAL_ASM = $(addprefix $(BUILD_DIR)/,$(notdir $(REVERT_ASM:%.revert_asm=%.asm)))
vpath %.asm $(sort $(dir $(FINAL_ASM))) # 将.asm文件加入到vpath中

.PHONY: all build clean

all: mk-dir build

## 从vpath中读取所有的.c文件，逐个编译成.o文件
$(BUILD_DIR)/%.o: %.c | $(DIR)
	$(CC) $(C_FLAGS) -o $@ $<

## 从vpath中读取所有的.o文件，逐个反汇编成.revert_asm文件
$(BUILD_DIR)/%.revert_asm: $(OBJ)
	$(REVERT_ASM_TOOL) -fnasm $< $@

$(BUILD_DIR)/%.asm: $(REVERT_ASM)
	sed -n '/Function begin/,/End of function/p' $< > $@

build: $(FINAL_ASM)

clean:
	- rm -rf $(BUILD_DIR)

mk-dir:
	- mkdir -p $(BUILD_DIR)
