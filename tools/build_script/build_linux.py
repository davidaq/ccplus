from common import *
import roadmap
import cmdhelper

def build():
    # build core static library
    target_dir = roadmap.build_dir + '/linux/'

    # scan source path
    sources = []
    sources = []
    for src_dir in roadmap.source_path:
        for item in scan4dir(src_dir):
            mkdir(target_dir + src_dir + '/' + item)
        for item in scan4file(src_dir, cmdhelper.source_file_filter):
            sources.append(src_dir + '/' + item)

    # prepare commands
    cxx_cmd = cmdhelper.cxx_cmd
    c_cmd = cmdhelper.c_cmd
    if roadmap.debug_mode:
        c_cmd += ' -g -DDEBUG '
        cxx_cmd += ' -g -DDEBUG '
    else:
        c_cmd += ' -DRELEASE '
        cxx_cmd += ' -DRELEASE '
    for item in cmdhelper.parselist(roadmap.include_path):
        c_cmd += ' -I' + item
        cxx_cmd += ' -I' + item
    for item in cmdhelper.parselist(roadmap.cxx_flags):
        cxx_cmd += ' ' + item
    for item in cmdhelper.parselist(roadmap.c_flags):
        c_cmd += ' ' + item

    # compile sources
    objs = []
    for item in sources:
        if item[-1] == 'm':     # skip obj-c sources
            continue
        dest = target_dir + item + '.o'
        objs.append(dest)
        depfile = dest + '.d'
        check = []
        if file_exists(depfile):
            fd = open(depfile, 'r')
            dep  = fd.read()
            fd.close()
            check = dep.split("\n")
        if item[-1] == 'c':
            make(item, dest, c_cmd, check)
        else:
            make(item, dest, cxx_cmd, check)
        rdepfile = target_dir + item + '.d'
        if file_exists(rdepfile):
            fd = open(rdepfile, 'r')
            dep  = fd.read()
            dep = dep[dep.find(':') + 1:].strip()
            dep = dep.replace('!', '!x')
            dep = dep.replace('\\ ', '!s')
            dep = dep.replace('\\', '')
            dep = dep.replace("\n", '')
            deplist = []
            for i in dep.split(' '):
                i = i.strip()
                if len(i) > 0:
                    i = i.replace('!s', ' ')
                    i = i.replace('!x', '!')
                    deplist.append(i)
            fd.close()
            fd = open(depfile, 'w')
            fd.write("\n".join(deplist))
            fd.close()

    # link program
    cmd = 'g++ -o %dst%'
    for item in cmdhelper.parselist(objs):
        cmd += ' ' + item
    for item in cmdhelper.parselist(roadmap.link_flags):
        cmd += ' ' + item
    make(None, target_dir + 'ccplus', cmd, objs)

