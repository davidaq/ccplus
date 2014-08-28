#!/usr/bin/env python

'''
This is a utility to be used with ffmpeg to extract the alpha part from a video
'''

import os
from rescale_template import rmdir,filedir
from json import JSONDecoder

def main():
    import sys
    export(sys.argv[1])

def export(videofile):
    extract_video(videofile)
    filter_alpha(videofile)
    fps = merge_alpha(videofile)
    extract_rgb(videofile, fps)
    clean(videofile)


def extract_video(videofile):
    tmpdir = videofile + '__frames/'
    rmdir(tmpdir)
    os.mkdir(tmpdir)
    os.system('ffmpeg -n -i "' + videofile + '" -f image2 -c:v rawvideo -pix_fmt argb "' + tmpdir + '%09d.argb"')

def filter_alpha(videofile):
    tmpdir = videofile + '__frames/'
    for frame in os.listdir(tmpdir):
        fp = open(tmpdir + frame, 'rb')
        data = fp.read()
        fp.close()
        alpha = bytearray()
        for i in range(0, len(data) / 4):
            alpha.append(data[i * 4])
        fp = open(tmpdir + frame, 'wb')
        fp.write(alpha)
        fp.close()

def merge_alpha(videofile):
    tmpdir = videofile + '__frames/'
    fmt = os.popen('ffprobe "' + videofile + '" -show_streams -print_format json -loglevel fatal').read()
    fmt = JSONDecoder().decode(fmt)
    for stream in fmt['streams']:
        if 'width' in stream and 'height' in stream:
            width = stream['width']
            height = stream['height']
            fps = stream['r_frame_rate']
    scale = str(width) + 'x' + str(height)
    os.system('ffmpeg -r ' + fps + ' -s ' + scale + ' -f image2 -c:v rawvideo -pix_fmt gray -i "' + tmpdir + '%09d.argb" -r ' + fps + ' -c:v libx264 -pix_fmt yuv420p "' + videofile + '.opacity.mp4"')
    return fps

def extract_rgb(videofile, fps):
    os.system('ffmpeg -n -i "' + videofile + '" -c:v libx264 -pix_fmt yuv420p -c:a aac -strict -2 -ac 1 -r ' + fps + ' "' + videofile + '.mp4"')

def clean(videofile):
    tmpdir = videofile + '__frames/'
    rmdir(tmpdir)

if __name__ == '__main__':
    main()
    
