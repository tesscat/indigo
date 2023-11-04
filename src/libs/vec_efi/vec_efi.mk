LIB_NAME := vec_efi
CFLAGS := $(EFI_C_FLAGS)
CPPFLAGS := $(EFI_CPP_FLAGS) -I$(POSIX_EFI_PATH)

$(BUILD_DIR)/libs/$(LIB_NAME)/%.o: $(LIBS_DIR)/$(LIB_NAME)/%.c
	mkdir -p $(shell dirname $@)
	clang $(LIBMK_INCL_STMT) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/libs/$(LIB_NAME)/%.o: $(LIBS_DIR)/$(LIB_NAME)/%.cpp
	mkdir -p $(shell dirname $@)
	clang++ $(LIBMK_INCL_STMT) $(CPPFLAGS) -c $< -o $@

include $(LIBS_DIR)/lib.mk
