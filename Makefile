PROJECT := indigo

// TODO: redo link-specific flags
ARCH := x86_64
TARGET := kernel
BUILD_DIR := build
OUT_DIR := out
SRC_DIR := src
ARCH_DIR := $(SRC_DIR)/$(TARGET)/arch/$(ARCH)/

PREFIX := $(ARCH)-elf

AS := clang
LD := clang
SILVERC := x
SILVERC_2 := clang
SILVERC_2_FLAGS := -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti

FLAGS := -ffreestanding -nostdlib -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -mno-avx
AS_FLAGS := $(FLAGS)

AS_FLAGS_32 := -target i686-elf
FLAGS_32 := -target i686-elf

default: $(OUT_DIR)/$(TARGET)

clean:
	rm -r $(OUT_DIR) $(BUILD_DIR)

include $(ARCH_DIR)/make.config

$(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.silver
	mkdir -p $(shell dirname $@)
	$(SILVERC) $< 1> /dev/null
	mv $(SRC_DIR)/$(*).32.ll $(BUILD_DIR)/$(*).32.ll
	$(SILVERC_2) -c $(SILVERC_2_FLAGS) $(FLAGS_32) $(BUILD_DIR)/$(*).32.ll -o $@
	# $(SILVERC_2) -S -m32 -no-integrated-as $(BUILD_DIR)/$(*).32.ll -o $(BUILD_DIR)/$(*).32.S
	# $(AS) $(AS_FLAGS) $(AS_FLAGS_32) $(BUILD_DIR)/$(*).32.S -o $@
	# $(SILVERC_2) $(SILVERC_2_FLAGS) -target i386-unknown-none-elf -c $(BUILD_DIR)/$(*).32.S -o $@

$(BUILD_DIR)/%.32.o: $(SRC_DIR)/%.32.s
	mkdir -p $(shell dirname $@)
	$(AS) $(AS_FLAGS) $(AS_FLAGS_32) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.silver
	mkdir -p $(shell dirname $@)
	$(SILVERC) $< 1> /dev/null
	mv $(SRC_DIR)/$(*).ll $(BUILD_DIR)/$(*).ll
	$(SILVERC_2) -S $(BUILD_DIR)/$(*).ll -o $(BUILD_DIR)/$(*).S
	$(SILVERC_2) $(SILVERC_2_FLAGS) -c $(BUILD_DIR)/$(*).S -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	mkdir -p $(shell dirname $@)
	$(AS) $(AS_FLAGS) -c $< -o $@
