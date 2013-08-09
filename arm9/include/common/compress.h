#ifndef COMPRESS_H
#define COMPRESS_H

uint compressRLE(u16 **dst, u16 *srcD, uint srcS);
uint decompressRLE(u16 *dst, u16 *src, uint dstS);

#endif