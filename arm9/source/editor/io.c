#include "editor/editor_main.h"

//WRITING STUFF

void writeRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;
	
	writeVect(&rec->position,f);
	writeVect(&rec->size,f);
	writeVect(&rec->lmSize,f);
	writeVect(&rec->lmPos,f);
	writeVect(&rec->normal,f);
	
	u16 mid=getMaterialID(rec->material);
	
	fwrite(&mid,sizeof(u16),1,f);
	fwrite(&rec->rot,sizeof(bool),1,f);
}

void writeRectangleList(rectangleList_struct* rl, FILE* f)
{
	if(!rl || !f)return;

	fwrite(&rl->num,sizeof(int),1,f);

	listCell_struct *lc=rl->first;
	while(lc)
	{
		writeRectangle(&lc->data, f);
		lc=lc->next;
	}
}

void writeMapEditor(editorRoom_struct* er, const char* str)
{
	if(!er)return;

	FILE* f=fopen(str,"wb+");
	if(!f)return;

	room_struct r;
	initRoom(&r, 0, 0, vect(0,0,0));
	r.rectangles=generateOptimizedRectangles(er->blockArray);
	createLight(vect(BLOCKMULTX*32,BLOCKMULTY*32,BLOCKMULTZ*32), TILESIZE*2*16);
	generateLightmaps(&r);

	writeRectangleList(&r.rectangles,f);
	writeVect(&r.lmSize,f);
	fwrite(r.lightMapBuffer,sizeof(u8),r.lmSize.x*r.lmSize.y,f);

	fclose(f);
}
