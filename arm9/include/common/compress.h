#ifndef COMPRESS_H
#define COMPRESS_H

uint compressRLE(u8 **dst, u8 *srcD, uint srcS);
uint decompressRLE(u8 *dst, u8 *src, uint dstS);

#endif