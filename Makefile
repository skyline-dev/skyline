subsdk9: target/aarch64-skyline-switch/release/librust_skyline.nso
	@cp $< $@

clean:
	@rm -f subsdk9 target/aarch64-skyline-switch/release/librust_skyline.nso

target/aarch64-skyline-switch/release/librust_skyline.nso:
	@cargo skyline build --release --nso
