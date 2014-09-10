NDK_PATH := /Users/apple/Lib/android-ndk-r9d
NDK_TOOLCHAIN_PREFIX := ${NDK_PATH}/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-
ANDROID_SYS_ROOT := ${NDK_PATH}/platforms/android-9/arch-arm/
NDK_CXX:=${NDK_TOOLCHAIN_PREFIX}g++ -isysroot=${ANDROID_SYS_ROOT} \
	-Iinclude -Idependency/boost -Idependency/opencv/headers -Idependency/ffmpeg/headers \
	-I${ANDROID_SYS_ROOT}/usr/include \
	-I${NDK_PATH}/sources/cxx-stl/gnu-libstdc++/4.8/include \
	-I${NDK_PATH}/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi/include \
	-std=c++11 -D__ANDROID__ \
	-D__STDC_CONSTANT_MACROS  -D_STDC_FORMAT_MACROS \
	-O3 -ffast-math 

NDK_AR:=${NDK_TOOLCHAIN_PREFIX}ar

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

.dependency:
	-./scripts/run load.py

build/Makefile: .dependency
	dependency/gyp/gyp ccplus.gyp --depth=. -f make --generator-output=./build -Icommon.gypi

testbuild: build/Makefile
	-rm -rf tmp/
	mkdir tmp/
	BUILDTYPE=Debug make -C ./build/ test -j4

build/android/_:
	mkdir -p build/android/
	-touch $@

android_a:build/android/_
	echo '(echo "\033[1;32m"$$@" \n\033[0m" && $$@) || killall make' > .tmp.sh
	chmod a+x .tmp.sh
	find src -type d -exec mkdir -p build/android/{} \;
	find src -name \*.cpp -exec "./.tmp.sh" ${NDK_CXX} {} -c -o build/android/{}.o \;
	rm -f .tmp.sh
	@echo "\033[1;32mMake static lib\n\033[0m"
	${NDK_AR} cr build/android/libccplus.a `find build/android/ -type f -name \*.o`

android_so:
	@echo "\033[1;32mCompile shared library\n\033[0m"
	-mv -f build/android/libccplus.a port/android/jni/armeabi/libccplus.a
	ndk-build -C port/android

android:android_a android_so
	@echo "\033[1;32mDone!!\n\033[0m"

ios:
	dependency/gyp/gyp ccplus.gyp --depth=. -f xcode --generator-output=./build/ios -Icommon.gypi -DOS=ios
	xcodebuild -project build/ios/ccplus.xcodeproj -configuration Release ARCHS='armv7 armv7s' IPHONEOS_DEPLOYMENT_TARGET='6.0' -target libccplus
	mv -f ./build/Release-iphoneos/libccplus.a ./port/iOS/ccplus.framework/ccplus

test: testbuild
	./test.sh '*'

xcode:
	dependency/gyp/gyp ccplus.gyp --depth=. -f xcode --generator-output=./build/xcode -Icommon.gypi

todo:
	@grep TODO `find src -type f -name *.cpp -o -name *.hpp`

	@grep TODO `find include -type f -name *.cpp -o -name *.hpp`

