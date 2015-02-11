# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccpluscore
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libccplus.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libavcodec
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libavdevice
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavdevice.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libavfilter
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libavformat
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libavutil
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libpostproc
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libpostproc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libswresample
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libswscale
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libx264
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libx264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libopencv
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libopencv.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libfreetype
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := ccplus
LOCAL_SRC_FILES := jni_ccplus.cpp jni_ccplay.cpp jni_videoinfo.cpp
LOCAL_STATIC_LIBRARIES := ccpluscore opencv freetype \
	avformat avcodec avdevice avfilter postproc swresample swscale libx264 avutil \
	-L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi \
	             -lgnustl_static
LOCAL_C_INCLUDES += /Users/apple/Documents/workspace/ccplus/dependency/opencv/headers \
					/Users/apple/Documents/workspace/ccplus/dependency/boost \
					/Users/apple/Documents/workspace/ccplus/dependency/ffmpeg/headers \
					/Users/apple/Documents/workspace/ccplus/dependency/freetype \
					/Users/apple/Documents/workspace/ccplus/include 
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -lz -llog -lGLESv2
LOCAL_ALLOW_UNDEFINED_SYMBOLS := false

include $(BUILD_SHARED_LIBRARY)
