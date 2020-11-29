#ifndef __D3D9__
#define __D3D9__

extern bool d3dScreen;

void initD3D();
void updateD3D();
void setRegCapture(bool enable, uint8 srcBlend, uint8 destBlend, uint8 bank, uint8 offset, uint8 size, uint8 source, uint8 srcOffset);
#endif