all: mac ios android

clean:
	-rm -rf build/
	-rm -rf .dep/
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
	-rm -rf dependency

.dependency:
	./load-dependency/load

build/test/Makefile: .dependency
	dependency/gyp/gyp ccplus.gyp --depth=. -f make --generator-output=./build/test -Icommon.gypi

test: build/test/Makefile
	-rm -rf tmp/
	BUILDTYPE=Debug make -C build/test test -j4 && ./build/test/out/Debug/test 
