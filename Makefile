HFILES       :=  $(shell find src/ -type f -name '*.h')
HPPFILES     :=  $(shell find src/ -type f -name '*.hpp')
CFILES       :=  $(shell find src/ -type f -name '*.c')
CPPFILES     :=  $(shell find src/ -type f -name '*.cpp')
SFILES       :=  $(shell find src/ -type f -name '*.s')
RSFILES      :=  $(shell find src/ -type f -name '*.rs') build.rs

SOURCE_FILES := $(CFILES) $(CPPFILES) $(SFILES) $(RSFILES) $(HFILES) $(HPPFILES)

build: subsdk9

clean:
	@rm -rf subsdk9 target

subsdk9: target/aarch64-skyline-switch/release/librust_skyline.nso
	@cp $< $@

target/aarch64-skyline-switch/release/librust_skyline.nso: $(SOURCE_FILES)
	@cargo skyline build --release --nso

