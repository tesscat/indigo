LIB_NAME := libstd
CFLAGS := $(BASE_FLAGS)
CPPFLAGS := $(BASE_FLAGS)

KERNEL_NAME := kernel
KERNEL_ARCH_DIR := $(SRC_DIR)/$(KERNEL_NAME)/arch/$(ARCH)/

INC_FLAGS := $(INC_FLAGS) -I$(KERNEL_ARCH_DIR)/headers -I$(SRC_DIR)/headers/common

$(BUILD_DIR)/libs/$(LIB_NAME)/%.o: $(SRC_DIR)/libs/$(LIB_NAME)/%.cpp
	mkdir -p $(shell dirname $@)
	$(CPPC) $(BASE_FLAGS) -fno-stack-protector -fno-stack-check $(INC_FLAGS) -c $< -o $@

include $(LIBS_DIR)/lib.mk
