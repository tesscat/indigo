ARCH := x86_64
TARGET := kernel
BUILD_DIR := build
OUT_DIR := out
SRC_DIR := src
ARCH_DIR := $(SRC_DIR)/$(TARGET)/arch/$(ARCH)/

PREFIX := $(ARCH)-elf

AS := $(PREFIX)-as
LD := $(PREFIX)-ld
SILVERC := x
SILVERC_2 := clang
SILVERC_2_FLAGS := -ffreestanding -Wall -Werror -Wextra -fno-exceptions -fno-rtti

AS_FLAGS :=
FLAGS := -ffreestanding -nostdlib -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-avx -shared

AS_FLAGS_32 := --32
FLAGS_32 := -m elf_i386

default: $(OUT_DIR)/$(TARGET)

clean:
	rm -r $(OUT_DIR) $(BUILD_DIR)

include $(ARCH_DIR)/make.config

$(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.silver
	mkdir -p $(shell dirname $@)
	$(SILVERC) $< &> /dev/null
	mv $(SRC_DIR)/$(*).32.ll $(BUILD_DIR)/$(*).32.ll
	$(SILVERC_2) -S -target i386-unknown-none-elf $(BUILD_DIR)/$(*).32.ll -o $(BUILD_DIR)/$(*).32.S
	$(SILVERC_2) $(SILVERC_2_FLAGS) -target i386-unknown-none-elf -c $(BUILD_DIR)/$(*).32.S -o $@

$(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.s
	mkdir -p $(shell dirname $@)
	$(AS) $(AS_FLAGS) $(AS_FLAGS_32) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.silver
	mkdir -p $(shell dirname $@)
	$(SILVERC) $< &> /dev/null
	mv $(SRC_DIR)/$(*).ll $(BUILD_DIR)/$(*).ll
	$(SILVERC_2) -S $(BUILD_DIR)/$(*).ll -o $(BUILD_DIR)/$(*).S
	$(SILVERC_2) $(SILVERC_2_FLAGS) -c $(BUILD_DIR)/$(*).S -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	mkdir -p $(shell dirname $@)
	$(AS) $(AS_FLAGS) $< -o $@
