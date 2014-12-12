#!/usr/bin/env python

import time

def build_for_platform(platform_name):
    try:
        exec('from build_' + platform_name + ' import build')
    except:
        print('Target platform "' + platform_name + '" not supported currently')
        return
    for i in range(0, 5):
        print('')
    print('#################################')
    print('# Building for "' + platform_name + '" ')
    print('# ' + time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()))
    print('#################################')
    print('')
    import os
    os.chdir(os.path.dirname(os.path.realpath(__file__)) + '/../../')
    build()
    print('##  Build Success');
    for i in range(0, 5):
        print('')

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        build_for_platform(sys.argv[1].lower())
    else:
        import platform
        build_for_platform(platform.system().lower())
