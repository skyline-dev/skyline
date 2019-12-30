# TODO (Khangaroo): Make this process a lot less hacky (no, export did not work)
# See MakefileNSO

.PHONY: all clean skyline send

CROSSVER ?= 600

all: skyline

skyline:
	$(MAKE) all -f MakefileNSO CROSSVER=$(CROSSVER)
	$(MAKE) skyline_patch_$(CROSSVER)/*.ips

skyline_patch_$(CROSSVER)/*.ips: patches/*.slpatch patches/configs/$(CROSSVER).config patches/maps/$(CROSSVER)/*.map \
								build$(CROSSVER)/$(shell basename $(CURDIR))$(CROSSVER).map scripts/genPatch.py
	@rm -f skyline_patch_$(CROSSVER)/*.ips
	python3 scripts/genPatch.py $(CROSSVER)

send: all
	python3.7 scripts/sendPatch.py $(IP) $(CROSSVER)

clean:
	$(MAKE) clean -f MakefileNSO
	@rm -fr skyline_patch_*
