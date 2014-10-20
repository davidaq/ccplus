#!/usr/bin/env python
# This is a script used to resize tml

from json import JSONDecoder
from json import JSONEncoder
from math import ceil
import string
import os
import sys
import rgb_video

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
        self.favor_rescale = {};
        for f in files:
            sz = (sizes[f]['width'], sizes[f]['height'])
            fsz = self.favoredsize(sz)
            self.favor_rescale["file://" + f] = (sz[0] * self.scale / fsz[0], sz[1] * self.scale / fsz[1])
            src_file = self.dirname + f
            out_file = self.dirname + '(resized)' + os.path.sep + f
            if out_file[-4:].lower() in ['.mp3','.aac','.wma','.rm','.wav','.flac','.ogg']:
                os.system('ffmpeg -i "' + src_file + '" -c:v copy -c:a copy -n "' + out_file + '"')
            elif out_file[-4:].lower() == '.mov':
                os.system('ffmpeg -i "' + src_file + '" -s ' + str(fsz[0]) + 'x' + str(fsz[1]) + ' -n -c:v png -pix_fmt rgba "' + out_file + '"')
                rgb_video.export(out_file)
                os.remove(out_file)
            elif out_file[-4:].lower() == '.png':
                os.system('ffmpeg -i "' + src_file + '" -s ' + str(fsz[0]) + 'x' + str(fsz[1]) + ' -n -c:v png -pix_fmt rgba "' + out_file + '"')
            else:
                os.system('ffmpeg -i "' + src_file + '" -s ' + str(fsz[0]) + 'x' + str(fsz[1]) + ' -n  "' + out_file + '"')

    def preparedir(self):
        #rmdir(self.dirname + '(resized)' + os.path.sep)
        dirs = []
        os.path.walk(self.dirname, scandir , dirs)
        preflen = len(self.dirname)
        for dir in dirs:
            dir = self.dirname + '(resized)' + os.path.sep + dir[preflen:]
            try:
                os.mkdir(dir)
            except:
                print(dir + ' exists');

    def resizetml(self):
        comps = self.tml['compositions']
        for comp in comps:
            comps[comp]['resolution']['width'] *= self.scale
            comps[comp]['resolution']['height'] *= self.scale
            for layer in comps[comp]['layers']:
                if layer['uri'][0:7] == 'text://':
                    txtProp = layer['text-properties']
                    for t in txtProp['size']:
                        txtProp['size'][t] = int(ceil(txtProp['size'][t] * self.scale))
                transform = layer['properties']['transform']
                for time in transform:
                    trans = transform[time]
                    for j in range(0, len(trans) / 12):
                        for i in range(0, 6):
                            ji = i + j * 12
                            trans[ji] *= self.scale
                    if layer['uri'] in self.favor_rescale:
                        scale = self.favor_rescale[layer['uri']]
                        adjust = [0, 0, 0, 0, 0, 0, scale[0], scale[1], 1, 0, 0, 0]
                        adjust.extend(trans)
                        transform[time] = adjust
                if 'mask' in layer['properties']:
                    mask = layer['properties']['mask']
                    for time in mask:
                        shape = mask[time]
                        for i in range(0, len(shape)):
                            shape[i] *= self.scale
                if 'ramp' in layer['properties']:
                    prop = layer['properties']['ramp']
                    for time in prop:
                        for index in (1, 2):
                            prop[time][index] *= self.scale
                if '4color' in layer['properties']:
                    prop = layer['properties']['4color']
                    for time in prop:
                        for index in (0, 1, 5, 6, 10, 11):
                            prop[time][index] *= self.scale
        del self.tml['usedfiles']
        del self.tml['usedcolors']
        json = JSONEncoder().encode(self.tml)
        fp = open(self.dirname + '(resized)' + os.path.sep + 'tpl.tml', 'w')
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
        if (maxwidth & 1) > 0:
            maxwidth += 1
        if (maxheight & 1) > 0:
            maxheight += 1
        if w > maxwidth:
            scale = maxwidth * 1.0/ w
            w = maxwidth
            h *= scale
        if h > maxheight:
            scale = maxheight * 1.0 / h
            h = maxheight
            w *= scale
        self.scale = scale * 1.0
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
            cfile = open(rawpath, 'wb')
            content = bytearray()
            for i in range(0, 3):
                c = int(float(color['color'][i]) * 255)
                content.append(c)
            cfile.write(content)
            cfile.close()
            os.system('ffmpeg -s 1x1 -f rawvideo -pix_fmt rgb24 -i "' + rawpath + '" -s ' + str(color['width']) + 'x' + str(color['height']) + ' -n "' + self.dirname + path + '"')

def main():
    if len(sys.argv) <= 1:
        print('Please specify input tml file')
        return
    fname = sys.argv[1];
    resizer = Resizer(fname)
    resizer.mkcolors()
    if len(sys.argv) > 2:
        scale = sys.argv[2]
    else:
        scale = '640x360'
    scale = scale.split('x')
    w = int(scale[0])
    h = w
    if len(scale) > 1:
        h = int(scale[1])
    resizer.downsize(w, h)

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
