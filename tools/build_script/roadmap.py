
debug_mode    = True

source_dir    = 'src'
build_dir     = 'build'

includes      = [
    'include',
    'dependency/gtest',
    'dependency/opencv/headers',
    'dependency/boost',
    'dependency/freetype',
    'dependency/ffmpeg/headers',
]

cxx_flags       = '-std=c++11'
c_flags         = '-std=c99'

links          = [
    'dependency/gtest/linux-gtest.a',
    'dependency/freetype/linux-freetype.a',
    'dependency/ffmpeg/ffmpeg-linux.a',
    'dependency/opencv/libs-linux/libmingwex_.a',
    'dependency/opencv/libs-linux/libopencv_imgproc.a',
    'dependency/opencv/libs-linux/libopencv_imgproc_pch_dephelp.a',
    'dependency/opencv/libs-linux/libcoldname_.a',
    'dependency/opencv/libs-linux/libopencv_flann_pch_dephelp.a',
    'dependency/opencv/libs-linux/libopencv_flann.a',
    'dependency/opencv/libs-linux/libwsock32_.a',
    'dependency/opencv/libs-linux/libopencv_photo.a',
    'dependency/opencv/libs-linux/libopencv_highgui.a',
    'dependency/opencv/libs-linux/libopencv_photo_pch_dephelp.a',
    'dependency/opencv/libs-linux/liblibtiff.a',
    'dependency/opencv/libs-linux/libgcc_.a',
    'dependency/opencv/libs-linux/libopencv_highgui_pch_dephelp.a',
    'dependency/opencv/libs-linux/_dep_liblibjasper.a',
    'dependency/opencv/libs-linux/_dep_libzlib.a',
    'dependency/opencv/libs-linux/libopencv_core_pch_dephelp.a',
    'dependency/opencv/libs-linux/liblibpng.a',
    'dependency/opencv/libs-linux/libopencv_core.a',
    'dependency/opencv/libs-linux/libIlmImf.a',
    'dependency/opencv/libs-linux/liblibjpeg.a',
    'dependency/opencv/libs-linux2/libmingwex_.a',
    'dependency/opencv/libs-linux2/libopencv_imgproc.a',
    'dependency/opencv/libs-linux2/libopencv_imgproc_pch_dephelp.a',
    'dependency/opencv/libs-linux2/libcoldname_.a',
    'dependency/opencv/libs-linux2/libopencv_flann_pch_dephelp.a',
    'dependency/opencv/libs-linux2/libopencv_flann.a',
    'dependency/opencv/libs-linux2/libwsock32_.a',
    'dependency/opencv/libs-linux2/libopencv_photo.a',
    'dependency/opencv/libs-linux2/libopencv_highgui.a',
    'dependency/opencv/libs-linux2/libopencv_photo_pch_dephelp.a',
    'dependency/opencv/libs-linux2/liblibtiff.a',
    'dependency/opencv/libs-linux2/libgcc_.a',
    'dependency/opencv/libs-linux2/libopencv_highgui_pch_dephelp.a',
    'dependency/opencv/libs-linux2/_dep_liblibjasper.a',
    'dependency/opencv/libs-linux2/_dep_libzlib.a',
    'dependency/opencv/libs-linux2/libopencv_core_pch_dephelp.a',
    'dependency/opencv/libs-linux2/liblibpng.a',
    'dependency/opencv/libs-linux2/libopencv_core.a',
    'dependency/opencv/libs-linux2/libIlmImf.a',
    'dependency/opencv/libs-linux2/liblibjpeg.a',
]
