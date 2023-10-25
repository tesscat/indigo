LIB_NAME := ion_efi
CFLAGS := $(EFI_C_FLAGS)
CPPFLAGS := $(EFI_CPP_FLAGS) -I$(POSIX_EFI_PATH)

include $(LIBS_DIR)/lib.mk
