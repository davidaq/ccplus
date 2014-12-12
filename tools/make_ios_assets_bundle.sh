#!/bin/bash
rm -r -f port/iOS/ccplus.bundle
mkdir -p port/iOS/ccplus.bundle
cd assets
find . -type d -not -name .\* -exec mkdir -p ../port/iOS/ccplus.bundle/{} \;
find . -type f -not -name .\* -exec cp {} ../port/iOS/ccplus.bundle/{}.f \;
