CPP_SRC_FILES := $(shell find $(LIBS_DIR)/$(LIB_NAME) -name "*.cpp")
_CPP_OBJ := $(CPP_SRC_FILES:.cpp=.o)
CPP_OBJ := $(shell echo " $(_CPP_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")

C_SRC_FILES := $(shell find $(LIBS_DIR)/$(LIB_NAME) -name "*.c")
_C_OBJ := $(C_SRC_FILES:.c=.o)
C_OBJ := $(shell echo " $(_C_OBJ)" | sed "s| $(SRC_DIR)| $(BUILD_DIR)|g")
#
# $(BUILD_DIR)/libs/$(LIB_NAME)/%.o: $(LIBS_DIR)/$(LIB_NAME)/%.c
# 	mkdir -p $(shell dirname $@)
# 	clang $(LIBMK_INCL_STMT) $(CFLAGS) -c $< -o $@
#
# $(BUILD_DIR)/libs/$(LIB_NAME)/%.o: $(LIBS_DIR)/$(LIB_NAME)/%.cpp
# 	mkdir -p $(shell dirname $@)
# 	clang++ $(LIBMK_INCL_STMT) $(CPPFLAGS) -c $< -o $@

$(OUT_DIR)/$(LIB_NAME).a: $(C_OBJ) $(CPP_OBJ)
	mkdir -p $(shell dirname $@)
	ar cr $@ $^
