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
	writeVect(rec->normal,f);
	
	fwrite(&rec->portalable,sizeof(bool),1,f);

	u16 mid=getMaterialID(rec->material);
	
	fwrite(&mid,sizeof(u16),1,f);
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

vect3D adaptVector(vect3D v, u8 dir)
{
	switch(dir)
	{
		case 0:
			v=vect(v.x*BLOCKMULTX,v.y*BLOCKMULTY+BLOCKMULTY/2,v.z*BLOCKMULTZ+BLOCKMULTZ/2);
			break;
		case 1:
			v=vect(v.x*BLOCKMULTX+BLOCKMULTX,v.y*BLOCKMULTY+BLOCKMULTY/2,v.z*BLOCKMULTZ+BLOCKMULTZ/2);
			break;
		case 3:
			v=vect(v.x*BLOCKMULTX+BLOCKMULTX/2,v.y*BLOCKMULTY+BLOCKMULTY,v.z*BLOCKMULTZ+BLOCKMULTZ/2);
			break;
		case 4:
			v=vect(v.x*BLOCKMULTX+BLOCKMULTX/2,v.y*BLOCKMULTY+BLOCKMULTY/2,v.z*BLOCKMULTZ);
			break;
		case 5:
			v=vect(v.x*BLOCKMULTX+BLOCKMULTX/2,v.y*BLOCKMULTY+BLOCKMULTY/2,v.z*BLOCKMULTZ+BLOCKMULTZ);
			break;
		case 6:
			//CENTER
			v=vect(v.x*BLOCKMULTX+BLOCKMULTX/2,v.y*BLOCKMULTY+BLOCKMULTY/2,v.z*BLOCKMULTZ+BLOCKMULTZ/2);
			break;
		default:
			v=vect(v.x*BLOCKMULTX+BLOCKMULTX/2,v.y*BLOCKMULTY,v.z*BLOCKMULTZ+BLOCKMULTZ/2);
			break;
	}
	return v;
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
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				writeVect(adaptVector(e->position, e->direction), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 1:
			//energy ball launcher
			writeVect(adaptVector(e->position, e->direction), f);
			return true;
		case 3:
			//pressure button
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				writeVect(adaptVector(e->position, e->direction), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 4:
			//turret
			{
				writeVect(adaptVector(e->position, 6), f);
			}
			return true;
		case 5: case 6: case 7:
			//cubes and dispenser
			{
				s16 target=(e->target)?(e->target->writeID):(-1);
				writeVect(adaptVector(e->position, e->direction), f);
				fwrite(&target, sizeof(s16), 1, f);
			}
			return true;
		case 8:
			//emancipation grid
			{
				int32 l=getGridLength(e);
				// NOGBA("L %d",l);
				fwrite(&l,sizeof(int32),1,f);
				writeVect(adaptVector(e->position, e->direction), f);
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
		case 10:
			//door
			{
				writeVect(vect(e->position.x*BLOCKMULTX+BLOCKMULTX/2,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ+BLOCKMULTZ/2), f);
				bool orientation=false; fwrite(&orientation,sizeof(bool),1,f);
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
		case 13:
			//wall door (start)
			{
				writeVect(adaptVector(e->position, e->direction), f);
				fwrite(&e->direction,sizeof(u8),1,f);
			}
			return true;
		case 14:
			//wall door (exit)
			{
				writeVect(adaptVector(e->position, e->direction), f);
				fwrite(&e->direction,sizeof(u8),1,f);
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

u8* compressBlockArray(BLOCK_TYPE* ba, u32* size)
{
	u8* dst;
	*size=compressRLE(&dst, (u8*)ba, sizeof(BLOCK_TYPE)*ROOMARRAYSIZEX*ROOMARRAYSIZEY*ROOMARRAYSIZEZ);
	return dst;
}

void writeHeader(mapHeader_struct* h, FILE* f)
{
	if(!h || !f)return;
	fseek(f, 0, SEEK_SET);
	fwrite(h, MAPHEADER_SIZE, 1, f);
}

void writeVertexLightingData(vertexLightingData_struct* vld, FILE* f)
{
	if(!vld)return;

	fwrite(&vld->width, sizeof(u8), 1, f);
	fwrite(&vld->height, sizeof(u8), 1, f);

	fwrite(vld->values, sizeof(u8), vld->width*vld->height, f);
}

void writeLightingData(lightingData_struct* ld, FILE* f)
{
	if(!ld || !f)return;

	switch(ld->type)
	{
		case LIGHTMAP_DATA:
			writeVect(ld->data.lightMap.lmSize, f);
			fwrite(ld->data.lightMap.buffer, sizeof(u8), ld->data.lightMap.lmSize.x*ld->data.lightMap.lmSize.y, f);
			fwrite(ld->data.lightMap.coords, sizeof(lightMapCoordinates_struct), ld->size, f);
			break;
		default:
			{
				int i;
				for(i=0;i<ld->size;i++)writeVertexLightingData(&ld->data.vertexLighting[i],f);
			}
			break;
	}
}

void writeMapEditor(editorRoom_struct* er, const char* str)
{
	if(!er)return;

	FILE* f=fopen(str,"wb+");
	// if(!f)return;

	mapHeader_struct h=blankHeader;
	writeHeader(&h,f);

	room_struct r;
	lightingData_struct* ld=&r.lightingData;
	initRoom(&r, 0, 0, vect(0,0,0));

	rectangleList_struct sludgeList;
	r.rectangles=generateOptimizedRectangles(er->blockArray, &sludgeList);
	generateLightsFromEntities();
	// generateLightmaps(&r, ld);
	generateVertexLighting(&r, ld);

	h.dataPosition=ftell(f);
		u8* compressed=compressBlockArray(er->blockArray, &h.dataSize);	// decompress(compressed, er->blockArray, RLE);
		if(!compressed){return;} //TEMP : clean up first !
		fwrite(compressed,sizeof(u8),h.dataSize,f);
		free(compressed);

	h.rectanglesPosition=ftell(f);
		writeRectangleList(&r.rectangles, f);

	h.lightPosition=ftell(f);
		writeLightingData(ld, f);

	h.entityPosition=ftell(f);
		writeEntities(f);

	h.sludgePosition=ftell(f);
		writeRectangleList(&sludgeList, f);

	writeHeader(&h,f);

	//empty sludgelist here
	while(sludgeList.num)popRectangle(&sludgeList);

	freeRoom(&r);
	fclose(f);
}

//READING STUFF

void readHeader(mapHeader_struct* h, FILE* f)
{
	if(!h || !f)return;
	fseek(f, 0, SEEK_SET);
	fread(h, MAPHEADER_SIZE, 1, f);
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
			fseek(f, sizeof(int32), SEEK_CUR);
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
		case 10:
			//door
			{
				readVect(&v, f);
				fseek(f, sizeof(bool), SEEK_CUR);
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
		case 13:
			//wall door (start)
			readVect(&v, f);
			fseek(f, sizeof(u8), SEEK_CUR);
			break;
		case 14:
			//wall door (end)
			readVect(&v, f);
			fseek(f, sizeof(u8), SEEK_CUR);
			break;
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
