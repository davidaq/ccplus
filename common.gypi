{
    'AQ':'hi',
    'configurations': {
      'Debug': {
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',
          'ONLY_ACTIVE_ARCH': 'YES',
        },
      },
    },
    'target_defaults': {
        'default_configuration': 'Debug',
        'cflags_cc': [ '-std=c++1y', '-fvisibility=hidden', '-fexceptions' ],
        'cflags': ['-Wall', '-Wno-null-character'],
        'xcode_settings': {
            'OTHER_CFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'OTHER_CPLUSPLUSFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'CLANG_CXX_LANGUAGE_STANDARD': 'c++1y',
            'CLANG_CXX_LIBRARY': 'libc++',
            'CLANG_ENABLE_OBJC_ARC': 'YES',
        },
        'conditions': [
            ['OS=="ios"', {
                'xcode_settings' : {
                    'SDKROOT': 'iphoneos',
                    'SUPPORTED_PLATFORMS': 'iOS',
                    'OTHER_CFLAGS' : ['-D_RELEASE'],
                },
                'cflags': ['-D_RELEASE'],
            }],
            ['OS=="mac"', {
                'defines': ['__OSX__'],
                'xcode_settings' : {
                    'SDKROOT': 'macosx10.9',
                },
                'link_settings' : {
                    'libraries' : [
                        #'-framework',
                        #'OpenGL'
                        '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework'
                    ]
                }
            }],
            ['OS=="android"', {
            }],
            ['OS=="linux"', {
                'cflags': ['-c' , '-w', '-pthread', '-std=c++11'],
                'cxxflags': ['-c', '-pthread'],
                'link_settings':{
                    "libraries":[
                        "-pthread", "-lGL"
                    ]
                }
            }],
        ],
        'configurations': {
            'Debug': {
                'defines': [ 'DEBUG' ],
                'cflags' : [ '-g', '-O0' ],
                'xcode_settings' : {
                },
            },
            'Release': {
                'defines': [ 'NDEBUG' ],
                'cflags': [
                    '-O3',
                    '-ffast-math',
                    '-fomit-frame-pointer',
                    '-fdata-sections',
                    '-ffunction-sections',
                ],
                'xcode_settings': {
                    'DEAD_CODE_STRIPPING': 'YES',
                },
            },
        },
    },
}
