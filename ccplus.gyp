{
    'targets': [
        {
            'target_name': 'libccplus',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [
                'dependency/boost/boost.gyp:boost',
                'dependency/freetype/freetype.gyp:freetype',
                'dependency/opencv/opencv.gyp:opencv',
                'dependency/ffmpeg/ffmpeg.gyp:ffmpeg',
            ],
            'sources': [
                '<!@(find src -type f -name "*.cpp")',
                '<!@(find src -type f -name "*.c")',
            ],
            'include_dirs': [
                'include',
                'build',
                'dependency',
                '.', 
                'include/externals/lua',
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    #'include',
                ]
            },
            'defines': [
                'IN_CCPLUS_PRIVATE_CONTEXT'
            ],
            'conditions': [
                ['OS=="ios"', {
                    'sources': [
                        'src/platform/ios.mm',
                        'src/platform/video-decoder-ios.mm',
                        'src/platform/video-encoder-ios.mm',
                    ]
                }],
            ],
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
