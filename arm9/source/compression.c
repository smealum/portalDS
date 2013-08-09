#include "common/general.h"

// code borrowed from GRIT 
// http://www.coranac.com/projects/grit/

//made it into 16bit compression code because screw it 8bit sucks

#define ALIGN4(n) ( ((n)+3)&~3 )

//! Compression type tags.
enum ECprsTag
{
	CPRS_FAKE_TAG	= 0x00,		//<! No compression.
	CPRS_LZ77_TAG	= 0x10,		//<! GBA LZ77 compression.
	CPRS_HUFF_TAG	= 0x20, 
//	CPRS_HUFF4_TAG	= 0x24,		//<! GBA Huffman, 4bit.
	CPRS_HUFF8_TAG	= 0x28,		//<! GBA Huffman, 8bit.
	CPRS_RLE_TAG	= 0x30,		//<! GBA RLE compression.
//	CPRS_DIFF8_TAG	= 0x81,		//<! GBA Diff-filter, 8bit.
//	CPRS_DIFF16_TAG	= 0x82,		//<! GBA Diff-filter, 16bit.
};

//! Create the compression header word (little endian)
u32	cprs_create_header(uint size, u8 tag)
{
	u8 data[4];

	data[0]= tag;
	data[1]= (size    ) & 255;
	data[2]= (size>> 8) & 255;
	data[3]= (size>>16) & 255;
	return *(u32*)data;
}

uint compressRLE(u16 **dst, u16 *srcD, uint srcS)
{
	if(!srcD || !dst)return 0;

	uint ii, rle, non;
	u16 curr, prev;

	// Annoyingly enough, rle _can_ end up being larger than
	// the original. A checker-board will do it for example.
	// if srcS is the size of the alternating pattern, then
	// the endresult will be 4 + srcS + (srcS+0x80-1)/0x80.
	uint dstS= 8+2*(srcS*2);
	u16 *dstD = (u16*)malloc(dstS), *dstL= dstD;
	if(!dstD)return 0;

	prev=srcD[0];
	rle=non= 1;

	// NOTE! non will always be 1 more than the actual non-stretch
	// PONDER: why [1,srcS] ?? (to finish up the stretch)
	for(ii=1; ii<=srcS; ii++)
	{
		if(ii!=srcS)curr=srcD[ii];

		if(rle==0x82 || ii==srcS)prev= ~curr;	// stop rle			

		if(rle<3 && (non+rle > 0x80 || ii==srcS))	// ** mini non
		{
			non += rle;
			dstL[0]= non-2;	
			memcpy(&dstL[1], &srcD[ii-non+1], (non-1)*2);
			dstL += non;
			non= rle= 1;
		}
		else if(curr == prev)		// ** start rle / non on hold
		{
			rle++;
			if( rle==3 && non>1 )	// write non-1 u16s
			{
				dstL[0]= non-2;
				memcpy(&dstL[1], &srcD[ii-non-1], (non-1)*2);
				dstL += non;
				non= 1;
			}
		}else{						// ** rle end / non start
			if(rle>=3)	// write rle
			{
				dstL[0]= 0x80 | (rle-3);
				dstL[1]= srcD[ii-1];
				NOGBA("RLE1 : %d %d",rle,dstL[1]);
				dstL += 2;
				non= 0;
				rle= 1;
			}
			non += rle;
			rle= 1;
		}
		prev= curr;
	}
	
	dstS=ALIGN4(dstL-dstD)+4;

	dstL=(u16*)malloc(dstS*2);
	if(!dstL)
	{
		free(dstD);
		return 0;
	}

	*(u32*)dstL=cprs_create_header(srcS, CPRS_RLE_TAG);
	memcpy(dstL+4, dstD, dstS*2-4);
	*dst=dstL;

	free(dstD);

	return dstS;
}

uint decompressRLE(u16 *dst, u16 *src, uint dstS)
{
	if(!dst || !src)return 0;

	uint ii, size=0;
	u16 *srcL=src+4, *dstD=dst;

	for(ii=0; ii<dstS; ii += size)
	{
		// Get header byte
		u32 header= *srcL++;

		if(header&0x80)		// compressed stint
		{
			size= min( (header&~0x80)+3, dstS-ii);
			// NOGBA("RLE1- : %d %d",size,*srcL);
			// memset(&dstD[ii], *srcL, size);
			int j; for(j=0;j<size;j++)dstD[ii+j]=*srcL; //can't used memset for 16bit
			srcL++;
		}
		else				// noncompressed stint
		{
			size= min(header+1, dstS-ii);
			memcpy(&dstD[ii], srcL, size*2);
			// memset(&dstD[ii], 0, size*2);
			// NOGBA("RLE2- : %d",size);
			srcL += size;
		}
	}

	return dstS;
}
