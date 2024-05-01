PROJECT := indigo

# TODO: redo link-specific flags
ARCH := x86_64
BUILD_DIR := build
OUT_DIR := out
SRC_DIR := src
COMPONENTS := loader kernel trampoline

LIBS_DIR := $(SRC_DIR)/libs

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
	rm -r $(OUT_DIR) $(BUILD_DIR) | true

COMPONENT_FILES := $(foreach component, $(COMPONENTS), $(SRC_DIR)/$(component)/arch/$(ARCH)/$(component).mk)

include $(COMPONENT_FILES)

LIB_FILES := $(foreach lib, $(shell find $(LIBS_DIR)/* -maxdepth 0 -type d | sed "s|$(LIBS_DIR)||g"), $(LIBS_DIR)/$(lib)/$(lib).mk)

include $(LIB_FILES)
