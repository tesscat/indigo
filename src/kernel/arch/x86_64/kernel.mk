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


INC_FLAGS := $(INC_FLAGS) -I$(KERNEL_ARCH_DIR)/headers -I$(SRC_DIR)/headers/common 

MKISO := grub-mkrescue

KERNEL_DEPS := $(OUT_DIR)/libstd.a $(OUT_DIR)/libmem.a

KERNEL_OBJS := $(KERNEL_SILVER_OBJ) $(KERNEL_AS_OBJ) $(KERNEL_CPP_OBJ) $(BUILD_DIR)/$(KERNEL_NAME)/arch/$(ARCH)/font.o $(KERNEL_DEPS)

.PHONY: project run listobjs

listobjs:
	echo $(KERNEL_OBJS)

$(BUILD_DIR)/$(KERNEL_NAME)/%.o: $(SRC_DIR)/$(KERNEL_NAME)/%.cpp
	mkdir -p $(shell dirname $@)
	$(CPPC) $(BASE_FLAGS) -mcmodel=large -fno-stack-protector -fno-stack-check $(INC_FLAGS) -c $< -o $@

$(OUT_DIR)/$(KERNEL_NAME): $(KERNEL_OBJS)
	mkdir -p $(shell dirname $@)
	$(LD) $(LD_FLAGS) -z max-page-size=0x1000 -T $(KERNEL_ARCH_DIR)/kernel.ld $^ -o $@

$(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/kernel: $(OUT_DIR)/$(KERNEL_NAME)
	mkdir -p $(shell dirname $@)
	cp $< $@

$(BUILD_DIR)/$(KERNEL_NAME)/%.psfu: $(SRC_DIR)/$(KERNEL_NAME)/%.psfu.gz
	mkdir -p $(shell dirname $@)
	cat $< | gunzip - > $@

$(BUILD_DIR)/$(KERNEL_NAME)/%/font.o: $(BUILD_DIR)/$(KERNEL_NAME)/%/font.psfu
	mkdir -p $(shell dirname $@)
	$(let prevdir, $(shell pwd), \
        mkdir -p $(shell dirname $@); \
		cd $(shell dirname $@); \
		objcopy -O elf64-x86-64 -B i386 -I binary font.psfu font.o; \
		cd $(prevdir); \
	)
