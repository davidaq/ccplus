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
	#git clone --depth 1 https://chromium.googlesource.com/external/gyp.git .deps/gyp
	git clone --depth 1 https://github.com/svn2github/gyp.git .deps/gyp

# TODO: Currently they are useless
#build_mac/mx3.xcodeproj: 
#	.deps/gyp/gyp mx3.gyp -DOS=mac --depth=. -f xcode --generator-output=./build_mac -Icommon.gypi
#
#build_ios/mx3.xcodeproj: deps/gyp deps/json11 mx3.gyp
#	deps/gyp/gyp mx3.gyp -DOS=ios --depth=. -f xcode --generator-output=./build_ios -Icommon.gypi
#
#GypAndroid.mk: deps/gyp deps/json11 mx3.gyp
#	ANDROID_BUILD_TOP=dirname $(which ndk-build) deps/gyp/gyp --depth=. -f android -DOS=android --root-target libmx3 -Icommon.gypi mx3.gyp
#
#xb-prettifier := $(shell command -v xcpretty >/dev/null 2>&1 && echo "xcpretty -c" || echo "cat")
#
#play: build_mac/mx3.xcodeproj objc/play.m
#	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target play_objc | ${xb-prettifier} && ./build/Debug/play_objc
#
#mac: build_mac/mx3.xcodeproj
#	xcodebuild -project build_mac/mx3.xcodeproj -configuration Release -target libmx3_objc | ${xb-prettifier}
#
#ios: build_ios/mx3.xcodeproj
#	xcodebuild -project build_ios/mx3.xcodeproj -configuration Release -target libmx3_objc | ${xb-prettifier}
#
#android: GypAndroid.mk
#	GYP_CONFIGURATION=Release NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=Application.mk -j4
#
#test: build_mac/mx3.xcodeproj
#	xcodebuild -project build_mac/mx3.xcodeproj -configuration Debug -target test | ${xb-prettifier} && ./build/Debug/test
#	

build_mac/Makefile: gyp
	.deps/gyp/gyp ccplus.gyp --depth=. -f make --generator-output=./build_mac -Icommon.gypi

test: build_mac/Makefile
	BUILDTYPE=Debug make -C build_mac test && ./build_mac/out/Debug/test 
