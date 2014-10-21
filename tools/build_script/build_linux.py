from common import *
import roadmap

def build():
    # build core static library
    target_dir = roadmap.build_dir + '/linux/'

    # prepare directories
    for item in scandir('src'):
        mkdir(target_dir + 'src/' + item)
    for item in scandir('test'):
        mkdir(target_dir + 'test/' + item)

    objs = []
    # compile c++ sources
    cmd = cxx_cmd + ' -D__LINUX__ ' + roadmap.cxx_flags
    if roadmap.debug_mode:
        cmd += ' -g -DDEBUG '
    else:
        cmd += ' -DRELEASE '
    for i in roadmap.includes:
        cmd += ' -I' + i
    for item in scanfile('src', '.*\\.cpp'):
        objs.append(make('src/' + item, target_dir + 'src/' + item + '.o', cmd))
    for item in scanfile('test', '.*\\.cpp'):
        objs.append(make('test/' + item, target_dir + 'test/' + item + '.o', cmd))

    # compile c sources
    cmd = c_cmd + ' -D__LINUX__ ' + roadmap.c_flags
    if roadmap.debug_mode:
        cmd += ' -g -DDEBUG '
    else:
        cmd += ' -DRELEASE '
    for i in roadmap.includes:
        cmd += ' -I' + i
    for item in scanfile('test', '.*\\.c'):
        objs.append(make('test/' + item, target_dir + 'test/' + item + '.o', cmd))
    for item in scanfile('src', '.*\\.c'):
        objs.append(make('src/' + item, target_dir + 'src/' + item + '.o', cmd))

    # link program
    cmd = 'g++ -o %dst% -std=c++11 -pthread -lGL '
    for item in objs:
        cmd += ' ' + item
    for item in roadmap.links:
        if item[-2:] == '.a':
            cmd += ' ' + item
        else:
            cmd += ' -l' + item
    make(None, target_dir + 'ccplus', cmd, objs)

