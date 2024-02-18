LOADER_NAME := loader
LOADER_ARCH_DIR := $(SRC_DIR)/$(LOADER_NAME)/arch/$(ARCH)
LOADER_SYSROOT_NAME := sysroot

POSIX_EFI_PATH := vendor/posix-uefi/uefi

LOADER_INCL_DIRS := $(LOADER_ARCH_DIR)/headers $(POSIX_EFI_PATH) $(SRC_DIR)/headers/common
LOADER_INCL_STMT := $(foreach path, $(LOADER_INCL_DIRS), -I$(path)) $(INC_FLAGS)

EFI_C_FLAGS := -fshort-wchar -mno-red-zone -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check -D__x86_64__ -DHAVE_USE_MS_ABI -mno-red-zone --target=x86_64-pc-win32-coff -Wno-builtin-requires-header -Wno-incompatible-library-redeclaration -Wno-long-long $(LOADER_INCL_STMT)
EFI_CPP_FLAGS := -fno-exceptions -fno-rtti $(EFI_C_FLAGS)
EFI_LD_FLAGS := -flavor link -subsystem:efi_application -nodefaultlib -dll -entry:uefi_init 

LOADER_CPP_SRC_FILES := $(shell find $(LOADER_ARCH_DIR) -name "*.cpp")
_LOADER_CPP_OBJ := $(LOADER_CPP_SRC_FILES:.cpp=.o)
LOADER_CPP_OBJ := $(shell echo " $(_LOADER_CPP_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

LOADER_C_SRC_FILES := $(shell find $(LOADER_ARCH_DIR) -name "*.c")
_LOADER_C_OBJ := $(LOADER_C_SRC_FILES:.c=.o)
LOADER_C_OBJ := $(shell echo " $(_LOADER_C_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

LOADER_DEPS := $(OUT_DIR)/ion_efi.a

LOADER_OBJ := $(LOADER_CPP_OBJ) $(LOADER_C_OBJ) $(LOADER_DEPS)

.PHONY: run posix_efi

posix_efi:
	make -C $(POSIX_EFI_PATH)

$(BUILD_DIR)/%.efi.o: $(SRC_DIR)/%.efi.cpp
	mkdir -p $(shell dirname $@)
	clang++ $(EFI_CPP_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.efi.o: $(SRC_DIR)/%.efi.c
	mkdir -p $(shell dirname $@)
	clang $(EFI_C_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(LOADER_NAME).efi: $(LOADER_OBJ)
	mkdir -p $(shell dirname $@)
	lld $(EFI_LD_FLAGS) $(POSIX_EFI_PATH)/*.o $^ -out:$@

$(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/: $(OUT_DIR)/$(LOADER_NAME).efi
	mkdir -p $@
	cp -r $(LOADER_ARCH_DIR)/$(LOADER_SYSROOT_NAME)/* $@
	mkdir -p $@/EFI/BOOT
	cp -r $< $@/EFI/BOOT/BOOTX64.EFI

$(OUT_DIR)/$(LOADER_SYSROOT_NAME)/: $(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/ $(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/kernel $(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/trampoline
	cp -r $< $@

$(OUT_DIR)/$(LOADER_NAME).efi: $(BUILD_DIR)/$(LOADER_NAME).efi
	mkdir -p $(shell dirname $@)
	cp $< $@

$(BUILD_DIR)/$(LOADER_NAME).img: $(OUT_DIR)/$(LOADER_SYSROOT_NAME)/
	# TODO: make these numbers better
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mcopy -s -i $@ $</* ::/

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(LOADER_NAME).img
	mkgpt -o $@ --image-size 4096 --part $< --type system

$(OUT_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).bin
	mv $< $@

# ifdef $(DEBUG)
run: $(OUT_DIR)/$(PROJECT).bin
	qemu-system-$(ARCH) -m 2G -s -S -bios /usr/share/edk2-ovmf/x64/OVMF.fd -net none -hda $^ &
	sleep 0.5
	lldb $(OUT_DIR)/trampoline $(OUT_DIR)/$(LOADER_NAME).efi $(OUT_DIR)/kernel -o 'gdb-remote localhost:1234'
# else
# run: $(OUT_DIR)/$(PROJECT).bin
	# qemu-system-$(ARCH) -bios /usr/share/edk2-ovmf/x64/OVMF.fd -net none -hda $^
# endif
