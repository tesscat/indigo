KERNEL_NAME := kernel
KERNEL_ARCH_DIR := $(SRC_DIR)/$(KERNEL_NAME)/arch/$(ARCH)/

KERNEL_AS_SRC_FILES := $(shell find $(KERNEL_ARCH_DIR) -name "*.s")
_KERNEL_AS_OBJ := $(KERNEL_AS_SRC_FILES:.s=.o)
KERNEL_AS_OBJ := $(shell echo " $(_KERNEL_AS_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

KERNEL_SILVER_SRC_FILES := $(shell find $(KERNEL_ARCH_DIR) -name "*.silver")
_KERNEL_SILVER_OBJ := $(KERNEL_SILVER_SRC_FILES:.silver=.o)
KERNEL_SILVER_OBJ := $(shell echo " $(_KERNEL_SILVER_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

KERNEL_CPP_SRC_FILES := $(shell find $(KERNEL_ARCH_DIR) -name "*.cpp")
_KERNEL_CPP_OBJ := $(KERNEL_CPP_SRC_FILES:.cpp=.o)
KERNEL_CPP_OBJ := $(shell echo " $(_KERNEL_CPP_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")



INC_FLAGS := $(INC_FLAGS) -I$(KERNEL_ARCH_DIR)/headers -I$(SRC_DIR)/$(KERNEL_NAME)/common/headers

MKISO := grub-mkrescue

KERNEL_OBJS := $(KERNEL_SILVER_OBJ) $(KERNEL_AS_OBJ) $(KERNEL_CPP_OBJ)

LINK_SCRIPT := $(KERNEL_ARCH_DIR)link.ld

.PHONY: project run listobjs

listobjs:
	echo $(KERNEL_OBJS)

# run: $(OUT_DIR)/$(PROJECT).iso 
# 	qemu-system-$(ARCH) -accel kvm -serial stdio -cdrom $(OUT_DIR)/$(PROJECT).iso


$(OUT_DIR)/$(KERNEL_NAME): $(KERNEL_OBJS)
	mkdir -p $(shell dirname $@)
	echo $(AS_SRC_FILES) $(KERNEL_OBJS)
	$(LD) -T $(LINK_SCRIPT) $(FLAGS) $(FLAGS_32) $^ -o $@

$(OUT_DIR)/$(PROJECT).iso: $(OUT_DIR)/$(KERNEL_NAME)
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(KERNEL_ARCH_DIR)/grub.cfg $(BUILD_DIR)/isodir/boot/grub/
	cp $(OUT_DIR)/$(KERNEL_NAME) $(BUILD_DIR)/isodir/boot/
	$(MKISO) $(BUILD_DIR)/isodir -o $@
