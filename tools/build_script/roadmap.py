
debug_mode    = True
build_dir     = 'build'

source_path   = [
    'src',
    'test',
]

include_path  = [
    'include',
    'dependency/gtest',
    'dependency/opencv/headers',
    'dependency/boost',
    'dependency/freetype',
    'dependency/ffmpeg/headers',
]

cxx_flags     = [
    '-std=c++11',
]
c_flags       = [
    '-std=c99',
]

link_flags    = [
    '-pthread',
    'dependency/gtest/linux-gtest.a',
    'dependency/freetype/linux-freetype.a',
    'dependency/ffmpeg/ffmpeg-linux.a',
    'dependency/opencv/linux-opencv.a',
]
