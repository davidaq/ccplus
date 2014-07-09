{
    'targets': [
        {
            'target_name': 'libccplus',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [
            ],
            'sources': [
                # just automatically include all cpp and hpp files in src/ (for now)
                # '<!' is shell expand
                # '@' is to splat the arguments into list items
                # todo(kabbes) this will not work on windows
                '<!@(find src -name "*.cpp" -o -name "*.hpp")',
            ],
            'include_dirs': [
                '.', 
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    'include',
                ],
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
                'gtest',
            ],
            'libraries': [
                '../gtest/gtest_main.a'
            ],
            'sources': [
                '<!@(find test -name "*.cpp" -o -name "*.hpp")',
            ]
        },
    ],
}
