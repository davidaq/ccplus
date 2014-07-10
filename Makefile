all: mac ios android

clean:
	-rm -rf build/
	-rm -rf deps/build/
	-rm -rf build_mac/
	-rm -rf build_ios/
	-rm -rf obj/
	-rm GypAndroid.mk
	-rm *.target.mk
	-rm deps/*.target.mk
	-rm -rf test_ldb
	-rm test.sqlite
	-rm play

clean-all: clean
	-rm -rf .deps

gyp: .deps/gyp

.deps/gyp:
	git clone --depth 1 https://github.com/svn2github/gyp.git .deps/gyp

build/test/Makefile: gyp
	.deps/gyp/gyp ccplus.gyp --depth=. -f make --generator-output=./build/test -Icommon.gypi

test: build/test/Makefile
	BUILDTYPE=Debug make -C build/test test && ./build/test/out/Debug/test 
