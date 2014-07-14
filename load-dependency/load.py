import urllib
import os
from json import JSONDecoder

REPO_URL = 'http://developer.ccme.me/ccplus-dependency/'

try:
    os.mkdir('dependency')
except:
    pass
decoder = JSONDecoder()
listing = decoder.decode(urllib.urlopen(REPO_URL).read())

for item in listing:
    try:
        print '>> Load depending library ['+item+']'
        os.mkdir('dependency/' + item)
        try:
            os.remove('tmp.zip')
        except:
            pass
        os.system('curl ' + listing[item]['url'] + ' > tmp.zip')
        print 'unpacking....'
        os.system('unzip -o tmp.zip -d dependency > /dev/null')
        try:
            os.remove('tmp.zip')
        except:
            pass
    except Exception,e:
        print item, 'already exists or failed to load'

