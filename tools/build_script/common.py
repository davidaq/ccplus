# Scan dir

import os
import re

cxx_cmd       = 'g++ -c %src% -o %dst%'
c_cmd         = 'gcc -c %src% -o %dst%'

def os_sep(path):
    return re.sub('/+', os.path.sep, path.replace('/./', '/'))

def mkdir(name):
    name = os_sep(name)
    if not os.path.isdir(name):
        os.makedirs(name, 0777)

def scandir(dirname, filter='.*'):
    if dirname[-1] <> '/':
        dirname = dirname + '/'
    dirs = ['']
    while len(dirs) > 0:
        dir = dirs.pop()
        dirpath = os_sep(dirname + dir)
        for name in os.listdir(dirpath):
            item = dir + name
            if os.path.isdir(os_sep(dirname + item)):
                dirs.append(item + '/')
                if re.match(filter + '$', name):
                    yield item

def scanfile(dirname, filter='.*'):
    if dirname[-1] <> '/':
        dirname = dirname + '/'
    dirs = ['']
    while len(dirs) > 0:
        dir = dirs.pop()
        dirpath = os_sep(dirname + dir)
        for name in os.listdir(dirpath):
            item = dir + name
            if os.path.isdir(os_sep(dirname + item)):
                dirs.append(item + '/')
            elif re.match(filter + '$', name):
                    yield item

def checkupdate(dest, check):
    if not os.path.exists(dest):
        return True
    dtime = os.path.getmtime(dest)
    for item in check:
        if not os.path.exists(item) or os.path.getmtime(item) > dtime:
            return True
    return False

def make(source, dest, cmd, check=[]):
    if source == None:
        source = dest
    check.append(source)
    if checkupdate(dest, check):
        cmd = cmd.replace('%src%', source).replace('%dst%', dest)
        print('##     ==>  ' + dest)
        if 0 <> os.system(cmd):
            print('#####  Build Failed with command:')
            print('#      ' + cmd)
            print('#################################')
            for i in range(0, 5):
                print('')
            os._exit(1)
        else:
            print('#####  pass')
    return dest
