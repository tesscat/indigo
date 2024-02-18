TRAMPOLINE_NAME := trampoline
TRAMPOLINE_ARCH_DIR := $(SRC_DIR)/$(TRAMPOLINE_NAME)/arch/$(ARCH)/

TRAMPOLINE_AS_SRC_FILES := $(shell find $(TRAMPOLINE_ARCH_DIR) -name "*.s")
_TRAMPOLINE_AS_OBJ := $(TRAMPOLINE_AS_SRC_FILES:.s=.o)
TRAMPOLINE_AS_OBJ := $(shell echo " $(_TRAMPOLINE_AS_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

TRAMPOLINE_SILVER_SRC_FILES := $(shell find $(TRAMPOLINE_ARCH_DIR) -name "*.silver")
_TRAMPOLINE_SILVER_OBJ := $(TRAMPOLINE_SILVER_SRC_FILES:.silver=.o)
TRAMPOLINE_SILVER_OBJ := $(shell echo " $(_TRAMPOLINE_SILVER_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

TRAMPOLINE_CPP_SRC_FILES := $(shell find $(TRAMPOLINE_ARCH_DIR) -name "*.cpp")
_TRAMPOLINE_CPP_OBJ := $(TRAMPOLINE_CPP_SRC_FILES:.cpp=.o)
TRAMPOLINE_CPP_OBJ := $(shell echo " $(_TRAMPOLINE_CPP_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")


INC_FLAGS := $(INC_FLAGS) -I$(TRAMPOLINE_ARCH_DIR)/headers -I$(SRC_DIR)/headers/common 

# TRAMPOLINE_DEPS := $(OUT_DIR)/libstd.a $(OUT_DIR)/libmem.a

TRAMPOLINE_OBJS := $(TRAMPOLINE_SILVER_OBJ) $(TRAMPOLINE_AS_OBJ) $(TRAMPOLINE_CPP_OBJ) $(TRAMPOLINE_DEPS)

.PHONY: project run listobjs

listobjs:
	echo $(TRAMPOLINE_OBJS)

$(BUILD_DIR)/$(TRAMPOLINE_NAME)/%.o: $(SRC_DIR)/$(TRAMPOLINE_NAME)/%.cpp
	mkdir -p $(shell dirname $@)
	$(CPPC) $(BASE_FLAGS) -fno-stack-protector -fno-stack-check $(INC_FLAGS) -c $< -o $@

$(OUT_DIR)/$(TRAMPOLINE_NAME): $(TRAMPOLINE_OBJS)
	mkdir -p $(shell dirname $@)
	$(LD) $(LD_FLAGS) -z max-page-size=0x1000 -T $(TRAMPOLINE_ARCH_DIR)/trampoline.ld $^ -o $@

$(BUILD_DIR)/$(LOADER_SYSROOT_NAME)/trampoline: $(OUT_DIR)/$(TRAMPOLINE_NAME)
	mkdir -p $(shell dirname $@)
	cp $< $@

$(BUILD_DIR)/$(TRAMPOLINE_NAME)/%.psfu: $(SRC_DIR)/$(TRAMPOLINE_NAME)/%.psfu.gz
	mkdir -p $(shell dirname $@)
	cat $< | gunzip - > $@

$(BUILD_DIR)/$(TRAMPOLINE_NAME)/%/font.o: $(BUILD_DIR)/$(TRAMPOLINE_NAME)/%/font.psfu
	mkdir -p $(shell dirname $@)
	$(let prevdir, $(shell pwd), \
        mkdir -p $(shell dirname $@); \
		cd $(shell dirname $@); \
		objcopy -O elf64-x86-64 -B i386 -I binary font.psfu font.o; \
		cd $(prevdir); \
	)
