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

clean-zim: 
	-rm -rf tmp/*.zim

.dependency:
	./scripts/run load.py

build/Makefile: .dependency
	dependency/gyp/gyp ccplus.gyp --depth=. -f make --generator-output=./build -Icommon.gypi

testbuild: build/Makefile
	-rm -rf tmp/
	mkdir tmp/
	BUILDTYPE=Debug make -C ./build/ test -j4

ios:
	dependency/gyp/gyp ccplus.gyp --depth=. -f xcode --generator-output=./build/ios -Icommon.gypi -Dios

test: testbuild
	./test.sh '*'

todo:
	@grep TODO `find src -type f -name *.cpp -o -name *.hpp`
	@grep TODO `find include -type f -name *.cpp -o -name *.hpp`
