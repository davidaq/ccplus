#!/usr/bin/env python
# This is a script used to resize tml

from json import JSONDecoder
import string
import os

class Resizer():
    def __init__(self, tmlfile):
        fp = open(tmlfile)
        content = fp.read()
        fp.close()
        self.tmlfile = tmlfile
        self.tml = JSONDecoder().decode(content)

    def usedfiles(self):
        files = {}
        for compname in tml['compositions']:
            comp = tml['compositions'][compname]
            layers = comp['layers']
            for layer_index in range(0, len(layers)):
                layer = layers[layer_index]
                if layer['uri'][0:7] == 'file://':
                    fname = layer['uri'][7:]
                    if fname not in files:
                        files[fname] = []
                    files[fname].append((compname,layer_index))
        return files

    def resize_files(self):
        newdir(filedir(self.tmlfile) + '(resized)')
        
    def resize_tml(self):
        pass
    
    def downsize(self, maxwidth, maxheight):
        topsize = self.tml['compositions'][self.tml['main']]['resolution']
        w, h = topsize['width'], topsize['height']
        r = 2
        while w / r > maxwidth:
            r += 1
        while h / r > maxheight:
            r += 1
        w /= r
        h /= r
        self.scale = 1.0 / r
        self.resize_files()
        self.resize_tml()


def main():
    fname = '/Users/apple/Desktop/AE Template/ZY/postcards/render.aep.tml'
    resizer = Resizer(fname)
    resizer.downsize(640, 360)

'''Utility functions'''
def filedir(filename):
    p = string.rfind(filename, os.path.sep)
    if p > -1:
        return filename[0:p + 1]
    return filename
def newdir(dirname):
    rmdir(dirname)
    os.mkdir(dirname)
def rmdir(dirname):
    os.path.walk(dirname, rmdirin, None)
def rmdirin(arg, dirname, files):
    for f in files:
        os.remove(dirname + os.path.sep + f)
    os.rmdir(dirname)

if __name__ == '__main__':
    main()
