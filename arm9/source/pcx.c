/*
* pcx.c -- pcx texture loader
* last modification: aug. 14, 2007
*
* Copyright (c) 2005-2007 David HENRY
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* gcc -Wall -ansi -lGL -lGLU -lglut pcx.c -o pcx
*/

#include "game/game_main.h"

int fileptr, filesize;

static void
ReadPCX1bit (FILE *fp, const struct pcx_header_t *hdr, struct gl_texture_t *texinfo)
{
	int y, i, bytes;
	int colorIndex;
	int rle_count = 0, rle_value = 0;
	u8 *ptr = texinfo->texels;

	for (y = 0; y < texinfo->height; ++y)
	{
		ptr = &texinfo->texels[(texinfo->height - (y + 1)) * texinfo->width * 3];
		bytes = hdr->bytesPerScanLine;

/* Decode line number y */
		while (bytes--)
		{
			if (rle_count == 0)
			{
				if ( (rle_value = fgetc (fp)) < 0xc0)
				{
					rle_count = 1;
				}
				else
				{
					rle_count = rle_value - 0xc0;
					rle_value = fgetc (fp);
				}
			}

			rle_count--;

			for (i = 7; i >= 0; --i, ptr += 3)
			{
				colorIndex = ((rle_value & (1 << i)) > 0);

				ptr[0] = hdr->palette[colorIndex * 3 + 0];
				ptr[1] = hdr->palette[colorIndex * 3 + 1];
				ptr[2] = hdr->palette[colorIndex * 3 + 2];
			}
		}
	}
}

static void
ReadPCX4bits (u8 *buffer, const struct pcx_header_t *hdr, struct gl_texture_t *texinfo)
{
	u8 *colorIndex, *line;
	u8 *pLine, *ptr;
	int rle_count = 0, rle_value = 0;
	int x, y, c, i;
	int bytes;

	colorIndex = (u8 *)malloc (sizeof (u8) * texinfo->width);
	line = (u8 *)malloc (sizeof (u8) * hdr->bytesPerScanLine);

/* Convert palette */
	for(i=0;i<16;i++)
	{
		texinfo->palette[i]=RGB15(hdr->palette[i*3+0]>>3,hdr->palette[i*3+1]>>3,hdr->palette[i*3+2]>>3);
	}

	for (y = 0; y < texinfo->height; ++y)
	{
// ptr = &texinfo->texels[(texinfo->height - (y + 1)) * texinfo->width * 3];
		ptr = &texinfo->texels[((y) * texinfo->width)/2];

		memset (colorIndex, 0, texinfo->width * sizeof (u8));

		for (c = 0; c < 4; ++c)
		{
			pLine = line;
			bytes = hdr->bytesPerScanLine;

/* Decode line number y */
			while (bytes--)
			{
				if (rle_count == 0)
				{
// if ( (rle_value = fgetc (fp)) < 0xc0)
					if ( (rle_value = buffer[fileptr++]) < 0xc0)
					{
						rle_count = 1;
					}
					else
					{
						rle_count = rle_value - 0xc0;
// rle_value = fgetc (fp);
						rle_value = buffer[fileptr++];
					}
				}

				rle_count--;
				*(pLine++) = rle_value;
			}

/* Compute line's color indexes */
			for (x = 0; x < texinfo->width; ++x)
			{
				if (line[x / 8] & (128 >> (x % 8)))colorIndex[x] += (1 << c);
			}
		}

/* Decode scan line.  color index => rgb  */
		int addr=0;
		for (x = 0; x < texinfo->width; ++x)
		{
// ptr[0] = hdr->palette[colorIndex[x] * 3 + 0];
// ptr[1] = hdr->palette[colorIndex[x] * 3 + 1];
// ptr[2] = hdr->palette[colorIndex[x] * 3 + 2];
			if((addr%2)){*ptr|=(colorIndex[x]&15)<<4;ptr++;}
			else {*ptr=(colorIndex[x]&15);}
			addr++;
		}
	}

/* Release memory */
	free (colorIndex);
	free (line);
}

static void
ReadPCX8bits (u8 *buffer, const struct pcx_header_t *hdr,
	struct gl_texture_t *texinfo)
{
	int rle_count = 0, rle_value = 0, i;
	u8 palette[768];
	u8 magic;
	u8 *ptr;
	fpos_t curpos;
	int y, bytes;

	/* The palette is contained in the last 769 bytes of the file */
	// fgetpos (fp, &curpos);
	curpos=fileptr;
	// fseek (fp, -769, SEEK_END);
	fileptr=filesize-769;
	// magic = fgetc (fp);
	magic=buffer[fileptr++];


	/* First byte must be equal to 0x0c (12) */
	if (magic != 0x0c)
	{
		NOGBA("error: colormap's first byte must be 0x0c! "
			"(%#x)\n", magic);

		free (texinfo->texels);
		texinfo->texels = NULL;
		return;
	}

	/* Read palette */
	// fread (palette, sizeof (u8), 768, fp);
	memcpy(palette,&buffer[fileptr],768);

	/* Convert palette */
	for(i=0;i<256;i++)
	{
		texinfo->palette[i]=RGB15(palette[i*3+0]>>3,palette[i*3+1]>>3,palette[i*3+2]>>3);
	}

	// fsetpos (fp, &curpos);
	fileptr=curpos;

	/* Read pixel data */
	for (y = 0; y < texinfo->height; ++y)
	{
	// ptr = &texinfo->texels[(texinfo->height - (y + 1)) * texinfo->width * 3];
		ptr = &texinfo->texels[(y) * texinfo->width];
		bytes = hdr->bytesPerScanLine;

	/* Decode line number y */
		while (bytes--)
		{
			if (rle_count == 0)
			{
	// if( (rle_value = fgetc (fp)) < 0xc0)
				if( (rle_value = buffer[fileptr++]) < 0xc0)
				{
					rle_count = 1;
				}
				else
				{
					rle_count = rle_value - 0xc0;
	// rle_value = fgetc (fp);
					rle_value = buffer[fileptr++];
				}
			}

			rle_count--;

	// ptr[0] = palette[rle_value * 3 + 0];
	// ptr[1] = palette[rle_value * 3 + 1];
	// ptr[2] = palette[rle_value * 3 + 2];
			*ptr = rle_value;
	// ptr += 3;
			ptr++;
		}
	}
}

	/*static void
ReadPCX24bits (FILE *fp, const struct pcx_header_t *hdr,
struct gl_texture_t *texinfo)
{
u8 *ptr = texinfo->texels;
int rle_count = 0, rle_value = 0;
int y, c;
int bytes;

for (y = 0; y < texinfo->height; ++y)
{
for (c = 0; c < 3; ++c)
{
ptr = &texinfo->texels[(texinfo->height - (y + 1)) * texinfo->width * 3];
bytes = hdr->bytesPerScanLine;

while (bytes--)
{
if (rle_count == 0)
{
if( (rle_value = fgetc (fp)) < 0xc0)
{
rle_count = 1;
}
else
{
rle_count = rle_value - 0xc0;
rle_value = fgetc (fp);
}
}

rle_count--;
ptr[c] = (u8)rle_value;
ptr += 3;
}
}
}
}*/

void convertPCX16Bit(struct gl_texture_t* pcx)
{
	if(!pcx || !pcx->texels || !pcx->palette || pcx->format!=8)return;

	pcx->format=16;
	pcx->texels16=(u16*)malloc(sizeof(u16)*pcx->width*pcx->height);
	if(!pcx->texels16)return;

	int i, j;
	for(i=0;i<pcx->width;i++)
	{
		for(j=0;j<pcx->height;j++)
		{
			pcx->texels16[i+j*pcx->width]=pcx->palette[pcx->texels[i+j*pcx->width]]|((pcx->texels[i+j*pcx->width]==0&&pcx->palette[0]==RGB15(31,0,31))?0:(1<<15));
		}
	}

	free(pcx->texels);
	free(pcx->palette);
	pcx->texels=pcx->palette=NULL;
}

extern int lastSize;

struct gl_texture_t * ReadPCXFile (const char *filename, char* directory)
{
	struct gl_texture_t *texinfo;
	struct pcx_header_t header;
// FILE *fp = NULL;
	int bitcount;
	u8* buffer;

/* Open image file */
// fp = fopen (filename, "rb");
// fp = DS_OpenFile(filename, directory, false, true);
	buffer = bufferizeFile((char*)filename, directory, NULL, true);
	filesize=lastSize;
	fileptr=0;
	if (!buffer)
	{
		char path[255];
		getcwd(path, 255);
		NOGBA("error: couldn't open \"%s\"! (%s)\n", filename, path);
		return NULL;
	}

/* Read header file */
// fread (&header, sizeof (struct pcx_header_t), 1, fp);
	memcpy(&header,buffer,sizeof (struct pcx_header_t));
	fileptr+=sizeof (struct pcx_header_t);
	if (header.manufacturer != 0x0a)
	{
		NOGBA("error: bad version number! (%i)\n",
			header.manufacturer);
		return NULL;
	}

/* Initialize texture parameters */
	texinfo = (struct gl_texture_t *)malloc (sizeof (struct gl_texture_t));
	texinfo->width = header.xmax - header.xmin + 1;
	texinfo->height = header.ymax - header.ymin + 1;
// texinfo->format = GL_RGB;
	texinfo->internalFormat = 3;

	bitcount = header.bitsPerPixel * header.numColorPlanes;
	texinfo->format = bitcount;
	texinfo->texels=NULL;
	texinfo->palette=NULL;
	texinfo->texels16=NULL;

/* Read image data */
	switch (bitcount)
	{
		/*case 1:
		ReadPCX1bit (fp, &header, texinfo);
		break;*/

		case 4:
		/* 4 bits color index */
		NOGBA("LOADING 4BIT");
		texinfo->texels = (u8 *) malloc ((sizeof (u8) * texinfo->width * texinfo->height) / 2);
		texinfo->palette = (u16 *) malloc (sizeof (u16) * 16);
		ReadPCX4bits (buffer, &header, texinfo);
		break;

		case 8:
		/* 8 bits color index */
		texinfo->texels = (u8 *) malloc (sizeof (u8) * texinfo->width * texinfo->height);
		texinfo->palette = (u16 *) malloc (sizeof (u16) * 256);
		NOGBA("TEXELS %p",texinfo->texels);
		ReadPCX8bits (buffer, &header, texinfo);
		break;

		/*case 24:
		ReadPCX24bits (fp, &header, texinfo);
		break;*/

		default:
		/* Unsupported */
		NOGBA("error: unknown %i bitcount pcx files\n", bitcount);
		// free (texinfo->texels);
		free (texinfo);
		texinfo = NULL;
		break;
	}

	// fclose (fp);
	free(buffer);
	return texinfo;
}

void freePCX(struct gl_texture_t * pcx)
{
	if(!pcx)return;
	if(pcx->texels)free(pcx->texels);
	if(pcx->texels16)free(pcx->texels16);
	if(pcx->palette)free(pcx->palette);
	pcx->texels=pcx->texels16=pcx->palette=NULL;
	free(pcx);
}
