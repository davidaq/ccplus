{
    'targets': [
        {
            'target_name': 'libccplus',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [
            ],
            'sources': [
                '<!@(find src -type f -name "*.cpp")',
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
                'dependency/gtest/gtest.gyp:gtest',
                'dependency/opencv/opencv.gyp:opencv'
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
