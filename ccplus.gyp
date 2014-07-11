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
                # Assume gtest is put on root too
            ],
            'include_dirs': [
                'test',
                'include',
                'dependency/gtest',
            ],
            'libraries': [
                '<!(pwd)/dependency/gtest/gtest_main.a'
            ],
            'sources': [
                '<!@(find test -type f -name "*.cpp")',
            ]
        },
    ],
}
