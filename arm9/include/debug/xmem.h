/*
 *	xmem.h
 *	  part of the xlibrary by SunDEV (http://sundev.890m.com)
 *
 *	Changelog :
 *	  21-03-09 : First public release
 *
 */

#ifndef _XMEM_H
#define _XMEM_H

size_t latestUsed, latestFree;

u8 *getHeapStart();
u8 *getHeapEnd();
u8 *getHeapLimit();
size_t getMemUsed();
size_t getMemFree();

#endif 
