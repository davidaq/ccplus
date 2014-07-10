{
    'targets': [
        {
            'target_name': 'libccplus',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [
            ],
            'sources': [
                '<!@(find src -name "*.cpp")',
            ],
            'include_dirs': [
                '.', 
                'include',
                'dependency'
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    'include',
                ]
            },
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
                'dependency/gtest',
            ],
            'libraries': [
                '<!(pwd)/dependency/gtest/gtest_main.a'
            ],
            'sources': [
                '<!@(find test -name "*.cpp")"',
            ]
        },
    ],
}
