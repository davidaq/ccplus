#!/usr/bin/env python
# This is a script used to resize tml

from json import JSONDecoder
from json import JSONEncoder
import string
import os

class Resizer():
    def __init__(self, tmlfile):
        fp = open(tmlfile)
        content = fp.read()
        fp.close()
        self.tmlfile = tmlfile
        self.dirname = filedir(tmlfile)
        self.tml = JSONDecoder().decode(content)

    def usedfiles(self):
        files = {}
        for compname in self.tml['compositions']:
            comp = self.tml['compositions'][compname]
            layers = comp['layers']
            for layer_index in range(0, len(layers)):
                layer = layers[layer_index]
                if layer['uri'][0:7] == 'file://':
                    fname = layer['uri'][7:]
                    if fname not in files:
                        files[fname] = []
                    files[fname].append((compname,layer_index))
        return files

    def resizefiles(self):
        self.preparedir()
        files = self.usedfiles()
        sizes = self.tml['usedfiles']
        for f in files:
            sz = (sizes[f]['width'], sizes[f]['height'])
            fsz = self.favoredsize(sz)
            src_file = self.dirname + f
            out_file = self.dirname + '(resized)' + os.path.sep + f
            os.system('ffmpeg -i "' + src_file + '" -s ' + str(fsz[0]) + 'x' + str(fsz[1]) + ' "' + out_file + '"')

    def preparedir(self):
        rmdir(self.dirname + '(resized)' + os.path.sep)
        dirs = []
        os.path.walk(self.dirname, scandir , dirs)
        preflen = len(self.dirname)
        for dir in dirs:
            dir = self.dirname + '(resized)' + os.path.sep + dir[preflen:]
            os.mkdir(dir)
        
    def resizetml(self):
        comps = self.tml['compositions']
        for comp in comps:
            comps[comp]['resolution']['width'] *= self.scale
            comps[comp]['resolution']['height'] *= self.scale
            for layer in comps[comp]['layers']:
                transform = layer['properties']['transform']
                for time in transform:
                    trans = transform[time]
                    for i in range(0, 4):
                        trans[i] *= self.scale
        del self.tml['usedfiles']
        del self.tml['usedcolors']
        json = JSONEncoder().encode(self.tml)
        fp = open(self.dirname + '(resized)' + os.path.sep + 'result.tml', 'w')
        fp.write(json)
        fp.close()
        cleandir(self.dirname + '(resized)' + os.path.sep)
    
    def favoredsize(self, size):
        w, h = size
        w = int(round(self.scale * w))
        if (w & 1) > 0:
            w += 1
        h = int(round(self.scale * h))
        if (h & 1) > 0:
            h += 1
        return (w, h)
    
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
        self.resizefiles()
        self.resizetml()

    def mkcolors(self):
        if 'usedcolors' not in self.tml:
            return
        rmdir(self.dirname + '(colors)' + os.path.sep)
        os.mkdir(self.dirname + '(colors)' + os.path.sep)
        for path in self.tml['usedcolors']:
            color = self.tml['usedcolors'][path]
            rawpath = self.dirname + path + '.raw'
            cfile = open(rawpath, 'w')
            content = bytearray()
            for i in range(0, 3):
                c = int(float(color['color'][i]) * 255)
                content.append(c)
            cfile.write(content)
            cfile.close()
            os.system('ffmpeg -s 1x1 -f rawvideo -pix_fmt rgb24 -i "' + rawpath + '" -s ' + str(color['width']) + 'x' + str(color['height']) + ' -n "' + self.dirname + path + '"')

def main():
    fname = '/Users/apple/Desktop/MyWildJ/render copy (CS6).aep.tml'
    resizer = Resizer(fname)
    resizer.mkcolors()
    resizer.downsize(320, 180)

'''Utility functions'''
def filedir(filename):
    p = string.rfind(filename, os.path.sep)
    if p > -1:
        return filename[0:p + 1]
    return filename
def rmdir(dirname):
    try:
        for item in os.listdir(dirname):
            item = dirname + item
            if os.path.isfile(item):
                os.remove(item)
            else:
                rmdir(item + os.path.sep)
        os.rmdir(dirname)
    except:
        pass
def cleandir(dirname):
    ret = True
    for item in os.listdir(dirname):
        item = dirname + item
        if os.path.isfile(item):
            ret = False
        elif not cleandir(item + os.path.sep):
            ret = False
    if ret:
        os.rmdir(dirname)
    return ret
def scandir(l, dirname, items):
    l.append(dirname)

if __name__ == '__main__':
    main()
