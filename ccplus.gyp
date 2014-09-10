{
    'targets': [
        {
            'target_name': 'libccplus',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [
                'dependency/boost/boost.gyp:boost',
                'dependency/opencv/opencv.gyp:opencv',
                'dependency/ffmpeg/ffmpeg.gyp:ffmpeg',
                'dependency/freetype/freetype.gyp:freetype',
            ],
            'sources': [
                '<!@(find src -type f -name "*.cpp")',
                '<!@(find src -type f -name "*.c")',
            ],
            'include_dirs': [
                '.', 
                'include',
                'dependency'
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    #'include',
                ]
            },
            'defines': [
                'IN_CCPLUS_PRIVATE_CONTEXT'
            ]
        },
        {
            'target_name': 'test',
            'type': 'executable',
            'dependencies': [
                'libccplus',
                'dependency/gtest/gtest.gyp:gtest'
            ],
            'include_dirs': [
                'test',
                'include',
            ],
            'sources': [
                '<!@(find test -type f -name "*.cpp")',
            ]
        },
    ],
}
