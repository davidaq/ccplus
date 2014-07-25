ZIM
===

ZIM is a image persisting format declared in this project, these files comes with the
suffix *.zip.
This file format is used to compress a image to a resonably small size fast while
keeping the alpha channel.
PNG wasn't chosed was because it takes too much time to compress.
JPEG it alone will discard the alpha channel.
We tried compressing raw bgra data with zip which gave a working but not optimized
solution, and that's why the format is originally called zim (zipped image).
Now the format is basically a concatenation of jpg, zipped alpha channel and zipped PCM audio data.

This format is only for temporary storage, not for comunicating between devices and
thus will not garantee portability between different platforms.

File binary structure
---------------------

The file binary structure is in the following order:

 - Image width: one unsigned short
 - Image height: one unsigned short
 - JPEG part length(JLEN): one unsigned long
 - JPEG part data: byte array with JLEN length, should be decoded as jpg
 - Alpha part length(ALEN): one unsigned long
 - Alpha part data: byte array with ALEN length, should be used as opacity alpha mask on the final image
 - Audio part length (AULEN): one unsigned long
 - Audio part data: byte array with AULEN length, should be decoded as audio signal sequence
