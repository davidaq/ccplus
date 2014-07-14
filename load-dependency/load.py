import urllib
import os
import shutil
from json import JSONDecoder

REPO_URL = 'http://developer.ccme.me/ccplus-dependency/'
DEP_DIR = 'dependency/'

try:
    os.mkdir(DEP_DIR)
except:
    pass
decoder = JSONDecoder()
listing = decoder.decode(urllib.urlopen(REPO_URL).read())

def __main__():
    for item in listing:
        print '>> Load depending library ['+item+']'
        try:
            os.mkdir(DEP_DIR + item)
            loaditem(item)
            continue
        except:
            pass
        try:
            hashfile = open(DEP_DIR + item + '/.hash')
            libhash = hashfile.read()
            hashfile.close()
            if libhash == listing[item]['hash']:
                continue
        except:
            pass
        loaditem(item)

def loaditem(item):
    try:
        os.remove('tmp.zip')
    except:
        pass
    try:
        shutil.rmtree(DEP_DIR+item)
    except:
        pass
    os.system('curl ' + listing[item]['url'] + ' > tmp.zip')
    print 'unpacking....'
    os.system('unzip -o tmp.zip -d '+DEP_DIR+item+' > /dev/null')
    hashfile = open(DEP_DIR + item + '/.hash', 'w')
    libhash = hashfile.write(listing[item]['hash'])
    hashfile.close()
    try:
        os.remove('tmp.zip')
    except:
        pass

__main__()

