MODULE_NAME := squashfs
# CPPFLAGS := $(BASE_FLAGS)


# $(BUILD_DIR)/modules/$(MODULE_NAME)/%.o: $(MODULES_DIR)/$(MODULE_NAME)/%.c
# 	mkdir -p $(shell dirname $@)
# 	clang $(moduleMK_INCL_STMT) $(CFLAGS) -c $< -o $@
tt:
	echo $(BUILD_DIR) $(MODULE_NAME) $(MODULES_DIR) $(KERNEL_NAME) -I$(SRC_DIR)/$(KERNEL_NAME)/arch/$(ARCH)/headers


$(BUILD_DIR)/modules/$(MODULE_NAME)/%.o: $(MODULES_DIR)/$(MODULE_NAME)/%.cpp
	mkdir -p $(shell dirname $@)
	clang++ $(moduleMK_INCL_STMT) $(CPPFLAGS) -c $< -o $@

CPP_SRC_FILES := $(shell find $(MODULES_DIR)/$(MODULE_NAME) -name "*.cpp")
_CPP_OBJ := $(CPP_SRC_FILES:.cpp=.o)
CPP_OBJ := $(shell echo " $(_CPP_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

C_SRC_FILES := $(shell find $(MODULES_DIR)/$(MODULE_NAME) -name "*.c")
_C_OBJ := $(C_SRC_FILES:.c=.o)
C_OBJ := $(shell echo " $(_C_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

IFLAGS := -I$(MODULES_DIR)/$(MODULE_NAME)/headers -I$(SRC_DIR)/$(KERNEL_NAME)/arch/$(ARCH)/headers

$(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/squashfs.iko: $(C_SRC_FILES) $(CPP_SRC_FILES)
	mkdir -p $(shell dirname $@)
	# TODO: figure out what __stack_chk_fail is and how to link it in
	$(CPPC) $(BASE_FLAGS) -fno-PIC -fno-PIE -fno-stack-check -fno-stack-protector $(IFLAGS) -c $^ -o $@
