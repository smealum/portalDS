#include "editor/editor_main.h"

mapHeader_struct blankHeader=(mapHeader_struct){0,0,0,0,0};

//WRITING STUFF

void writeTranslatedVect(vect3D v, FILE* f)
{
	writeVect(vectDifference(v,vect((ROOMARRAYSIZEX/2)*BLOCKMULTX,(ROOMARRAYSIZEY/2)*BLOCKMULTY,(ROOMARRAYSIZEZ/2)*BLOCKMULTZ)),f);
}

void writeRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;

	writeVect(rec->position,f);
	writeVect(rec->size,f);
	writeVect(rec->lmSize,f);
	writeVect(rec->lmPos,f);
	writeVect(rec->normal,f);
	
	fwrite(&rec->portalable,sizeof(bool),1,f);

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

bool writeEntity(entity_struct* e, FILE* f)
{
	if(!e || !f || !e->used || !e->type)return false;

	fwrite(&e->type->id,sizeof(u8),1,f);
	writeVect(e->position, f);
	fwrite(&e->direction, sizeof(u8), 1, f);

	switch(e->type->id)
	{
		case 0:
			//energy ball catcher
			//BLOCKMULTY/2 etc should depend on orientation ?
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 1:
			//energy ball launcher
			//BLOCKMULTY/2 etc should depend on orientation ?
			writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
			return true;
		case 3:
			//pressure button
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 4:
			//turret
			{
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
			}
			return true;
		case 5: case 6: case 7:
			//cubes and dispenser
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 8:
			//emancipation grid
			{
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
			}
			return true;
		case 9:
			//platform
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				vect3D tp=e->position;
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
				if(e->target)tp=e->target->position;
				writeVect(vect(tp.x*BLOCKMULTX+BLOCKMULTX/2,tp.y*BLOCKMULTY,tp.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 11:
			//light
			writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY-BLOCKMULTY/2,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
			return true;
		case 12:
			//platform target
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		default:
			return true;
	}
}

extern entity_struct entity[NUMENTITIES];

void writeEntities(FILE* f)
{
	int i;
	u16 cnt=0;
	size_t pos=ftell(f);
	fwrite(&cnt,sizeof(u16),1,f);
	int k=0;for(i=0;i<NUMENTITIES;i++)if(entity[i].used && entity[i].type)entity[i].writeID=k++;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(writeEntity(&entity[i], f))cnt++;
	}
	fseek(f, pos, SEEK_SET);
	fwrite(&cnt,sizeof(u16),1,f);
	fseek(f, 0, SEEK_END);
}

u8* compressBlockArray(u8* ba, u32* size)
{
	u8* dst;
	*size=compressRLE(&dst, ba, sizeof(u8)*ROOMARRAYSIZEX*ROOMARRAYSIZEY*ROOMARRAYSIZEZ);
	return dst;
}

void writeHeader(mapHeader_struct* h, FILE* f)
{
	if(!h || !f)return;
	fseek(f, 0, SEEK_SET);
	fwrite(h, sizeof(mapHeader_struct), 1, f);
}

void writeMapEditor(editorRoom_struct* er, const char* str)
{
	if(!er)return;

	FILE* f=fopen(str,"wb+");
	if(!f)return;

	mapHeader_struct h=blankHeader;
	writeHeader(&h,f);

	room_struct r;
	initRoom(&r, 0, 0, vect(0,0,0));

	r.rectangles=generateOptimizedRectangles(er->blockArray);
	generateLightsFromEntities();
	generateLightmaps(&r);

	h.dataPosition=ftell(f);
		u8* compressed=compressBlockArray(er->blockArray, &h.dataSize);	// decompress(compressed, er->blockArray, RLE);
		if(!compressed){return;} //TEMP : clean up first !
		fwrite(compressed,sizeof(u8),h.dataSize,f);
		free(compressed);

	h.rectanglesPosition=ftell(f);
		writeRectangleList(&r.rectangles,f);

	h.lightPosition=ftell(f);
		writeVect(r.lmSize,f);
		fwrite(r.lightMapBuffer,sizeof(u8),r.lmSize.x*r.lmSize.y,f);

	h.entityPosition=ftell(f);
		writeEntities(f);

	writeHeader(&h,f);

	fclose(f);
}

//READING STUFF

void readHeader(mapHeader_struct* h, FILE* f)
{
	if(!h || !f)return;
	fseek(f, 0, SEEK_SET);
	fread(h, sizeof(mapHeader_struct), 1, f);
}

void readEntityEditor(FILE* f)
{
	if(!f)return;

	u8 type; vect3D v;

	fread(&type,sizeof(u8),1,f);
	entity_struct* e=createEntity(vect(0,0,0), type, true);
	readVect(&e->position, f);
	fread(&e->direction, sizeof(u8), 1, f);

	switch(type)
	{
		case 0:
			//energy ball catcher
			{
				readVect(&v, f);
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
				if(target>=0 && target<NUMENTITIES)e->target=&entity[target];
			}
			break;
		case 1:
			//energy ball launcher
			readVect(&v, f);
			break;
		case 3:
			//pressure button
			{
				readVect(&v, f);
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
				if(target>=0 && target<NUMENTITIES)e->target=&entity[target];
			}
			break;
		case 4:
			//turret
			readVect(&v, f);
			break;
		case 5: case 6:
			//cubes
			{
				readVect(&v, f);
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
				if(target>=0 && target<NUMENTITIES)e->target=&entity[target];
			}
			break;
		case 7:
			//dispenser
			{
				readVect(&v, f);
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
				if(target>=0 && target<NUMENTITIES)e->target=&entity[target];
			}
			break;
		case 8:
			//emancipation grid
			readVect(&v, f);
			break;
		case 9:
			//platform
			{
				readVect(&v, f);
				readVect(&v, f);
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
				if(target>=0 && target<NUMENTITIES)e->target=&entity[target];
			}
			return;
		case 11:
			//light
			readVect(&v, f);
			break;
		case 12:
			//platform target
			{
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
				if(target>=0 && target<NUMENTITIES)e->target=&entity[target];
			}
			return;
		default:
			removeEntity(e);
			break;
	}

}

void readEntitiesEditor(FILE* f)
{
	int i; u16 cnt;
	removeEntities();
	fread(&cnt,sizeof(u16),1,f);
	for(i=0;i<cnt;i++)
	{
		readEntityEditor(f);
	}
}

void loadMapEditor(editorRoom_struct* er, const char* str)
{
	if(!er || !str)return;

	FILE* f=fopen(str,"rb");
	if(!f)return;

	mapHeader_struct h;
	readHeader(&h, f);

	fseek(f, h.dataPosition, SEEK_SET);
		u8* compressed=malloc(sizeof(u8)*h.dataSize);
		if(!compressed){return;} //TEMP : clean up first !
		fread(compressed, sizeof(u8), h.dataSize, f);
		decompress(compressed, er->blockArray, RLE); // decompressRLE(er->blockArray, compressed, ROOMARRAYSIZEX*ROOMARRAYSIZEY*ROOMARRAYSIZEZ);		
		free(compressed);

	fseek(f, h.entityPosition, SEEK_SET);
		readEntitiesEditor(f);

	fclose(f);

	generateBlockFacesRange(er->blockArray, &er->blockFaceList, vect(0,0,0), vect(ROOMARRAYSIZEX,ROOMARRAYSIZEY,ROOMARRAYSIZEZ), false);
	getEntityBlockFaces(er->blockFaceList);
}
