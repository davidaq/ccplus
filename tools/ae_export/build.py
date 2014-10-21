#!/usr/bin/env python
import os

def main():
    jslist = scandir('ae_export/', filterjs)
    content = ''
    for item in jslist:
        content += "/*******\n * " + item + "\n *******/\n"
        content += readfile(item) + "\n"
    content += "main();\n";
    writefile('ae_export.jsx', content)

def filterjs(filename):
    if filename[-3:] == '.js':
        return True
    return False

def readfile(filename):
    f = open(filename, 'r')
    ret = f.read()
    f.close()
    return str(ret)

def writefile(filename, content):
    f = open(filename, 'w')
    f.write(content)
    f.close()

def scandir(dirname, filterfunc=None, rlist=[]):
    for item in os.listdir(dirname):
        item = dirname + item
        if os.path.isfile(item) and (filterfunc == None or filterfunc(item)):
            rlist.append(item)
        elif os.path.isdir(item):
            scandir(item + os.path.sep, filterfunc, rlist)
    return rlist

main()
