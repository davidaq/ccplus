NDK_PATH := /Users/apple/Lib/android-ndk-r10d
NDK_TOOLCHAIN_PREFIX := ${NDK_PATH}/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-
ANDROID_SYS_ROOT := ${NDK_PATH}/platforms/android-9/arch-arm/
NDK_CC:=${NDK_TOOLCHAIN_PREFIX}gcc -isysroot=${ANDROID_SYS_ROOT} \
	-Ibuild/ -Iinclude -Idependency/boost -Idependency/opencv/headers -Idependency/ffmpeg/headers -Idependency/freetype \
	-I${ANDROID_SYS_ROOT}/usr/include \
	-Iinclude/externals/lua \
	-std=c99 -D__ANDROID__ -DGLSLES \
	-D__STDC_CONSTANT_MACROS  -D_STDC_FORMAT_MACROS \
	-O3 -ffast-math 
NDK_CXX:=${NDK_TOOLCHAIN_PREFIX}g++ -isysroot=${ANDROID_SYS_ROOT} \
	-Ibuild/ -Iinclude -Idependency/boost -Idependency/opencv/headers -Idependency/ffmpeg/headers -Idependency/freetype \
	-I${ANDROID_SYS_ROOT}/usr/include \
	-Iinclude/externals/lua \
	-I{$NDK_PATH}/sources/cxx-stl/llvm-libc++/libcxx/include \
	-I${NDK_PATH}/sources/cxx-stl/gnu-libstdc++/4.8/include \
	-I${NDK_PATH}/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi/include \
	-std=c++11 -D__ANDROID__ -DGLSLES \
	-D__STDC_CONSTANT_MACROS  -D_STDC_FORMAT_MACROS \
	-O3 -ffast-math 

NDK_AR:=${NDK_TOOLCHAIN_PREFIX}ar

MEVIDEO_PATH := /Users/apple/Desktop/meVideo-iOS

all: todo

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

make_bin_header:
	#mkdir -p build/res
	#find res -type f -exec ./tools/make_bin_header.py {} build/{} \;

.dependency:make_bin_header
	#@-./scripts/run load.py

build/Makefile: .dependency
	-dependency/gyp/gyp ccplus.gyp --depth=. -f make --generator-output=./build -Icommon.gypi

testbuild: build/Makefile
	-rm -rf tmp/
	mkdir tmp/
	CC=gcc CXX=g++ BUILDTYPE=Debug make -C ./build/ test -j4

build/android/_:
	mkdir -p build/android/
	-touch $@

android_a:build/android/_
	@find src -type d -exec mkdir -p build/android/{} \;
	@find src -name \*.cpp | while read x; do \
		if [ ! -f "build/android/$$x.o" ] || [ `stat -f %m "$$x"` -gt `stat -f %m "build/android/$$x.o"` ];then \
		echo "\033[1;32m"$$x"\033[0m"; \
		${NDK_CXX} $$x -c -o build/android/$$x.o; \
		else echo "\033[2;32m"$$x"\033[0m"; \
		fi;\
		done
	@find src -name \*.c | while read x; do \
		if [ ! -f "build/android/$$x.o" ] || [ `stat -f %m "$$x"` -gt `stat -f %m "build/android/$$x.o"` ];then \
		echo "\033[1;32m"$$x"\n\033[0m"; \
		${NDK_CC} $$x -c -o build/android/$$x.o; \
		fi;\
		done
	@echo "\033[1;32mMake static lib\n\033[0m"
	@${NDK_AR} cr build/android/libccplus.a `find build/android/ -type f -name \*.o`

android_so:
	@echo "\033[1;32mCompile shared library\n\033[0m"
	-mv -f build/android/libccplus.a port/android/jni/armeabi/libccplus.a
	ndk-build -C port/android

android:android_a android_so
	@echo "\033[1;32mDone!!\n\033[0m"

ios:
	./tools/make_ios_assets_bundle.sh
	dependency/gyp/gyp ccplus.gyp --depth=. -f xcode --generator-output=./build/ios -Icommon.gypi -DOS=ios
	xcodebuild -project build/ios/ccplus.xcodeproj -configuration Release ARCHS='x86_64 i386 armv7 armv7s arm64' IPHONEOS_DEPLOYMENT_TARGET='6.0' -target libccplus
	mv -f ./build/Release-iphoneos/libccplus.a ./port/iOS/ccplus.framework/ccplus
	cp -f ./port/iOS/ccplus.framework/ccplus ${MEVIDEO_PATH}/dependency/ccplus.framework/ccplus 
	rm -r -f ${MEVIDEO_PATH}/dependency/ccplus.bundle
	cp -r -f ./port/iOS/ccplus.bundle ${MEVIDEO_PATH}/dependency/ccplus.bundle
	cp -f ./include/ccplus.hpp ${MEVIDEO_PATH}/dependency/ccplus.framework/Headers/ccplus.hpp
	cp -f ./include/ccplay.hpp ${MEVIDEO_PATH}/dependency/ccplus.framework/Headers/ccplay.hpp

test: testbuild
	./test.sh '*'

xcode:
	-dependency/gyp/gyp ccplus.gyp --depth=. -f xcode --generator-output=./build/xcode -Icommon.gypi

todo:
	@grep -n TODO `find src -type f -name *.cpp -o -name *.hpp`
	@grep -n TODO `find include -type f -name *.cpp -o -name *.hpp`

