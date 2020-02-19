# TODO (Khangaroo): Make this process a lot less hacky (no, export did not work)
# See MakefileNSO

.PHONY: all clean skyline send

CROSSVER ?= 600

PYTHON := python3
ifeq (, $(shell which python3))
	# if no python3 alias, fall back to `python` and hope it's py3
	PYTHON   := python
endif

NAME 			:= $(shell basename $(CURDIR))
NAME_LOWER		:= $(shell echo $(NAME) | tr A-Z a-z)
PATCH 			:= $(NAME_LOWER)_patch_$(CROSSVER)

PATCH_DIR 		:= patches
BUILD_DIR 		:= build$(CROSSVER)

CONFIGS 		:= $(PATCH_DIR)/configs
CROSS_CONFIG 	:= $(CONFIGS)/$(CROSSVER).config

MAPS 			:= $(PATCH_DIR)/maps
CROSS_MAPS 		:= $(MAPS)/$(CROSSVER)
NAME_MAP 		:= $(BUILD_DIR)/$(NAME)$(CROSSVER).map

all: skyline

skyline:
	$(MAKE) all -f MakefileNSO CROSSVER=$(CROSSVER)
#	$(MAKE) skyline_patch_$(CROSSVER)/*.ips

$(PATCH)/*.ips: $(PATCH_DIR)/*.slpatch $(CROSS_CONFIG) $(CROSS_MAPS)/*.map $(NAME_MAP) scripts/genPatch.py
	@rm -f $(PATCH)/*.ips
	$(PYTHON) scripts/genPatch.py $(CROSSVER)

send: all
	$(PYTHON) scripts/sendPatch.py $(IP) $(CROSSVER)

clean:
	$(MAKE) clean -f MakefileNSO
	@rm -fr skyline_patch_*
