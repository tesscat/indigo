PROJECT := indigo
BASE_DIR := $(shell pwd)
# TODO: redo link-specific flags
ARCH := x86_64
BUILD_DIR := build
OUT_DIR := out
MODULES_OUT_DIR := $(OUT_DIR)/modules
SRC_DIR := src
COMPONENTS := loader kernel trampoline

LIBS_DIR := $(SRC_DIR)/libs

MODULES_DIR := $(SRC_DIR)/modules

INITRD_DIR := $(SRC_DIR)/initrd

INC_FLAGS := -I$(SRC_DIR)/headers/libs

PREFIX := $(ARCH)-elf

LD := ld.lld
CPPC := clang++ -std=c++20 -g -O0

NASM := nasm
SILVERC := x
SILVERC_2 := clang
SILVERC_2_FLAGS := -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti

BASE_FLAGS := $(INC_FLAGS) -ffreestanding -nostdlib -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-avx -fno-rtti -Wall -Wextra -fno-exceptions -D_STDINT_H=1
AS_FLAGS := $(BASE_FLAGS)

LD_FLAGS := -nostdlib --no-pie

NASM_FLAGS := -felf64 -g

clean:
	rm -rf $(OUT_DIR) $(BUILD_DIR)

COMPONENT_FILES := $(foreach component, $(COMPONENTS), $(SRC_DIR)/$(component)/arch/$(ARCH)/$(component).mk)

include $(COMPONENT_FILES)

LIB_FILES := $(foreach lib, $(shell find $(LIBS_DIR)/* -maxdepth 0 -type d | sed "s|$(LIBS_DIR)||g"), $(LIBS_DIR)/$(lib)/$(lib).mk)

include $(LIB_FILES)

MODULE_FILES := $(foreach mod, $(shell find $(MODULES_DIR)/* -maxdepth 0 -type d | sed "s|$(MODULES_DIR)||g"), $(MODULES_DIR)/$(mod)/$(mod).mk)

INITRD_MODULES :=

include $(MODULE_FILES)

# need to include initrd after modules

include $(INITRD_DIR)/initrd.mk
