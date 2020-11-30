#include "game/game_main.h"

#define RESERVED_SIZE_DISPLAY_LISTS   (64*1024) //TEMP?

static u32 dl_displayLists[RESERVED_SIZE_DISPLAY_LISTS];
static u32 dl_displayLists_filled = 0;

static u32 dl_curdisplayList_filled_start;

static u8 dl_commands_buffer[4];
static u32 dl_commands_buffer_filled = 0;

static u32 dl_attributes_buffer[8];
static u32 dl_attributes_buffer_filled = 0;

void packCommandsForDisplayList()
{
	u32 i;

	// NOGBA("packing : %d %d",dl_commands_buffer_filled,dl_attributes_buffer_filled);
	while (dl_commands_buffer_filled < 4)
	{
		// NOGBA("NOP");
		dl_commands_buffer[dl_commands_buffer_filled] = FIFO_NOP;
		dl_commands_buffer_filled++;
	}
	dl_displayLists[dl_displayLists_filled] = FIFO_COMMAND_PACK(dl_commands_buffer[0], dl_commands_buffer[1], dl_commands_buffer[2], dl_commands_buffer[3]);
	dl_displayLists_filled++;
	for (i=0; i<dl_attributes_buffer_filled; i++)
	{
		dl_displayLists[dl_displayLists_filled] = dl_attributes_buffer[i];
		dl_displayLists_filled++;
	}

	dl_commands_buffer_filled = 0;
	dl_attributes_buffer_filled = 0;

	if(dl_displayLists_filled > RESERVED_SIZE_DISPLAY_LISTS){NOGBA("overflow in model loading!\n");}
}

u32* glBeginListDL()
{
	dl_displayLists_filled = 0;
	dl_attributes_buffer_filled = 0;
	dl_commands_buffer_filled = 0;
	dl_curdisplayList_filled_start = dl_displayLists_filled;
	dl_displayLists_filled++; // current position will need to contain how many commands there are in the end, so we skip that one for now
	return (dl_displayLists + dl_curdisplayList_filled_start);
}

u32 glEndListDL()
{
	packCommandsForDisplayList();
	// start of the display list now needs to be updated as to how much commands it holds
	dl_displayLists[dl_curdisplayList_filled_start] = (dl_displayLists_filled - dl_curdisplayList_filled_start) - 1;
	return dl_displayLists_filled;
}

void glVertex3v16DL(v16 x, v16 y, v16 z)
{
    if (dl_attributes_buffer_filled > 2)packCommandsForDisplayList();
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_VERTEX16;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) ((y << 16) | (x & 0xFFFF));
    dl_attributes_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) (z & 0xFFFF);
    dl_attributes_buffer_filled++;
    if (dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
}

u32 glVertexPackedDL(u32 packed)
{
	if(dl_attributes_buffer_filled > 2)packCommandsForDisplayList();
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_VERTEX10;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) (packed);
	dl_attributes_buffer_filled++;
	u32 temp=dl_displayLists_filled+dl_commands_buffer_filled;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
	return temp;
}

void glTexCoordPACKED(u32 uv)
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_TEX_COORD;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) (uv);
	dl_attributes_buffer_filled++;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
}

u32 glNormalDL(uint32 normal)
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_NORMAL;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) normal;
	dl_attributes_buffer_filled++;
	u32 temp=dl_displayLists_filled+dl_commands_buffer_filled;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
	return temp;
}

void glColorDL(rgb color)
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_COLOR;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) color;
	dl_attributes_buffer_filled++;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
}

void glBindPaletteDL(u32 addr)
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_PAL_FORMAT;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) addr;
	dl_attributes_buffer_filled++;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
}

void glBindTextureDL(u32 addr)
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_TEX_FORMAT;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) addr;
	dl_attributes_buffer_filled++;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
}

void glPolyFmtDL(u32 fmt)
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_POLY_FORMAT;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) fmt;
	dl_attributes_buffer_filled++;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
}

void applyMTLDL(mtlImg_struct* mtl)
{
	if(mtl)
	{
		glBindTextureDL(mtl->param);
		glBindPaletteDL(((uint32)mtl->pal)>>(4));
	}else glBindTextureDL(0);
}

u32 glBeginDL(u32 type)
{
	packCommandsForDisplayList();
	u32 r=(dl_displayLists_filled - dl_curdisplayList_filled_start) - 1;
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_BEGIN;
	dl_commands_buffer_filled++;
	dl_attributes_buffer[dl_attributes_buffer_filled] = type;
	dl_attributes_buffer_filled++;
	if(dl_commands_buffer_filled==4 || dl_attributes_buffer_filled==4)packCommandsForDisplayList();
	return r;
}

void glEndDL()
{
	dl_commands_buffer[dl_commands_buffer_filled] = FIFO_END;
	dl_commands_buffer_filled++;
	if(dl_commands_buffer_filled==4)packCommandsForDisplayList();
}
