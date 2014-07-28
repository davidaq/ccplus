#!/usr/bin/env python
from sys import argv

def sha1OfFile(filepath):
    import hashlib
    sha = hashlib.sha1()
    with open(filepath, 'rb') as f:
        while True:
            block = f.read(2**10)
            if not block: break
            sha.update(block)
        return sha.hexdigest()

print sha1OfFile(argv[1])
