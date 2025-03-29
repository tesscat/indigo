LIB_NAME := libmem
CFLAGS := $(BASE_FLAGS)
CPPFLAGS := $(BASE_FLAGS)

$(BUILD_DIR)/libs/$(LIB_NAME)/%.o: $(SRC_DIR)/libs/$(LIB_NAME)/%.cpp
	mkdir -p $(shell dirname $@)
	$(CPPC) $(BASE_FLAGS) -fno-stack-protector -fno-stack-check $(INC_FLAGS) -c $< -o $@

include $(LIBS_DIR)/lib.mk
