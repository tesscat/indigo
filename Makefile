PROJECT := indigo

 # TODO: redo link-specific flags
ARCH := x86_64
BUILD_DIR := build
OUT_DIR := out
SRC_DIR := src
# ARCH_DIR := $(SRC_DIR)/$(TARGET)/arch/$(ARCH)/
COMPONENTS := loader kernel

LIBS_DIR := $(SRC_DIR)/libs

INC_FLAGS := -I$(SRC_DIR)/headers/libs

PREFIX := $(ARCH)-elf

# ARCH_32 := i686
# PREFIX_32 := $(ARCH_32)-elf

# AS := $(PREFIX)-g++
LD := ld.lld
CPPC := clang++
# AS_32 := $(PREFIX_32)-g++
# LD_32 := $(PREFIX_32)-g++
# CPPC_32 := $(PREFIX_32)-g++

NASM_32 := nasm
# AS := clang++
# LD := clang++
# CPPC := clang++
SILVERC := x
SILVERC_2 := clang
SILVERC_2_FLAGS := -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti

BASE_FLAGS := $(INC_FLAGS) -ffreestanding -nostdlib -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-avx -fno-pie -no-pie -O2 -fno-rtti -Wall -Wextra -fno-exceptions
AS_FLAGS := $(BASE_FLAGS)

LD_FLAGS := -nostdlib --no-pie

AS_FLAGS_32 :=
FLAGS_32 :=
NASM_FLAGS_32 := -felf32

# default: $(OUT_DIR)/$(TARGET)

clean:
	rm -r $(OUT_DIR) $(BUILD_DIR)

COMPONENT_FILES := $(foreach component, $(COMPONENTS), $(SRC_DIR)/$(component)/arch/$(ARCH)/$(component).mk)

include $(COMPONENT_FILES)

LIB_FILES := $(foreach lib, $(shell find $(LIBS_DIR)/* -maxdepth 0 -type d | sed "s|$(LIBS_DIR)||g"), $(LIBS_DIR)/$(lib)/$(lib).mk)

include $(LIB_FILES)


# $(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.cpp
# 	mkdir -p $(shell dirname $@)
# 	$(CPPC_32) $(FLAGS) $(FLAGS_32) -c $^ -o $@
#
# $(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.silver
# 	mkdir -p $(shell dirname $@)
# 	INT_WIDTH=32 $(SILVERC) $< &> /dev/null
# 	mv $(SRC_DIR)/$(*).32.ll $(BUILD_DIR)/$(*).32.ll
# 	$(SILVERC_2) -c $(SILVERC_2_FLAGS) $(FLAGS_32) $(BUILD_DIR)/$(*).32.ll -o $@
#
# $(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.s
# 	mkdir -p $(shell dirname $@)
# 	$(AS_32) $(AS_FLAGS) $(AS_FLAGS_32) -c $< -o $@
#
# $(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.S
# 	mkdir -p $(shell dirname $@)
# 	$(NASM_32) $(NASM_FLAGS) $(NASM_FLAGS_32) $< -o $@
#
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.silver
# 	mkdir -p $(shell dirname $@)
# 	$(SILVERC) $< &> /dev/null
# 	mv $(SRC_DIR)/$(*).ll $(BUILD_DIR)/$(*).ll
# 	$(SILVERC_2) -S $(BUILD_DIR)/$(*).ll -o $(BUILD_DIR)/$(*).S
# 	$(SILVERC_2) $(SILVERC_2_FLAGS) -c $(BUILD_DIR)/$(*).S -o $@
#
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
# 	mkdir -p $(shell dirname $@)
# 	$(AS) $(AS_FLAGS) -c $< -o $@
