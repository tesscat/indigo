# TODO: unweird this
MOD_ESC := $(shell echo $(MODULES_OUT_DIR)/ | sed 's/\//\\\//g')
INITRD_DEPS := $(shell echo $(INITRD_MODULES) | sed 's/[^ ]* */$(MOD_ESC)&/g')


$(OUT_DIR)/initrd: $(INITRD_DEPS) # TODO: better initrd dep handling
	mkdir -p $(shell dirname $@)
	cd $(MODULES_OUT_DIR); mksquashfs $(INITRD_MODULES) $(BASE_DIR)/$@ -no-strip -no-compression
	cd $(BASE_DIR)
