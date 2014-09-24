#!/usr/bin/env python
import sys

inputfile = sys.argv[1]
outputfile = sys.argv[2]

ifile = open(inputfile, 'rb')
ofile = open(outputfile, 'w')

chunk = ifile.read()
ifile.close()

ofile.write('"')
counter = 0
for k in chunk:
    ofile.write('\\')
    c = hex(ord(k))[1:]
    if len(c) < 3:
        c = 'x0' + c[1]
    ofile.write(c)
    counter += 1
    if counter % 16 == 0:
        ofile.write('"\n"')
ofile.write('"\n')
ofile.close()

ofile = open(outputfile + '.count', 'w')
ofile.write(str(len(chunk)))
ofile.close()
