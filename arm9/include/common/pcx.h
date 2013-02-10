#ifndef __PCX9__
#define __PCX9__

/* OpenGL texture info */
struct gl_texture_t
{
	u16 width;
	u16 height;

	int format;
	int internalFormat;
	u32 id;

	u8 *texels;
	u16 *texels16;
	u16 *palette;
};

#pragma pack(1)
/* PCX header */
struct pcx_header_t
{
	u8 manufacturer;
	u8 version;
	u8 encoding;
	u8 bitsPerPixel;

	u16 xmin, ymin;
	u16 xmax, ymax;
	u16 horzRes, vertRes;

	u8 palette[48];
	u8 reserved;
	u8 numColorPlanes;

	u16 bytesPerScanLine;
	u16 paletteType;
	u16 horzSize, vertSize;

	u8 padding[54];
};
#pragma pack(4)

struct gl_texture_t * ReadPCXFile (const char *filename, char* directory);
void convertPCX16Bit(struct gl_texture_t* pcx);
void freePCX(struct gl_texture_t * pcx);

#endif