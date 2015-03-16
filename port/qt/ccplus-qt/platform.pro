LIBS += \
    -lbz2 \
    $$PWD/../../../dependency/ffmpeg/ffmpeg-mac.a \
    $$PWD/../../../dependency/freetype/mac-freetype.a \
    $$PWD/../../../dependency/opencv/libs-mac/libIlmImf.a \
    $$PWD/../../../dependency/opencv/libs-mac/liblibjasper.a \
    $$PWD/../../../dependency/opencv/libs-mac/liblibjpeg.a \
    $$PWD/../../../dependency/opencv/libs-mac/liblibpng.a \
    $$PWD/../../../dependency/opencv/libs-mac/liblibtiff.a \
    $$PWD/../../../dependency/opencv/libs-mac/libopencv_core.a \
    $$PWD/../../../dependency/opencv/libs-mac/libopencv_highgui.a \
    $$PWD/../../../dependency/opencv/libs-mac/libopencv_imgproc.a \
    $$PWD/../../../dependency/opencv/libs-mac/libopencv_objdetect.a \
    $$PWD/../../../dependency/opencv/libs-mac/libopencv_photo.a \
    $$PWD/../../../dependency/opencv/libs-mac/libzlib.a

DEFINES += __OSX__
