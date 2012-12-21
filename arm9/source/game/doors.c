#include "editor/editor_main.h"

doorCollection_struct doorCollection;
mtlImg_struct* doorMtl;

extern roomEdit_struct roomEdits[NUMROOMEDITS];

void initDoor(door_struct* d)
{
	if(!d)return;
	d->used=false;
	d->primaryRoom=d->secondaryRoom=NULL;
	d->position=0;
	d->height=0;
}

void deleteDoor(door_struct* d)
{
	if(!d)return;
	d->used=false;
}

void initDoorCollection(doorCollection_struct* dc)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		initDoor(&dc->door[i]);
	}
	doorMtl=createTexture("door.pcx", "textures");
}

bool doorCollides(doorCollection_struct* dc, room_struct* r, doorDirection dd, u16 p, door_struct* d2)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && dc->door[i].primaryRoom==r)
		{
			door_struct* d=&dc->door[i];
			if(d!=d2 && d->direction==dd && abs(p-d->position)<DOORSIZE)return true;
		}
	}
	return false;
}

bool checkDoorWay(door_struct* d, room_struct* r, int x, int y, bool secondary, bool override)
{
	if(!d||!d->primaryRoom||(secondary&&!d->secondaryRoom)||(!override&&(!d->open||d->percentage)))return false;
	switch(d->direction)
	{
		case doorUp:
			if(!secondary)return y<0&&x<d->position+DOORSIZE&&x>=d->position;
			else {x+=r->position.x-d->primaryRoom->position.x;return y>=r->height&&x<d->position+DOORSIZE&&x>=d->position;}
			break;
		case doorDown:
			if(!secondary)return y>=r->height&&x<d->position+DOORSIZE&&x>=d->position;
			else {x+=r->position.x-d->primaryRoom->position.x;return y<0&&x<d->position+DOORSIZE&&x>=d->position;}
			break;
		case doorLeft:
			if(!secondary)return x<0&&y<d->position+DOORSIZE&&y>=d->position;
			else {y+=r->position.y-d->primaryRoom->position.y;return x>=r->width&&y<d->position+DOORSIZE&&y>=d->position;}
			break;
		case doorRight:
			if(!secondary)return x>=r->width&&y<d->position+DOORSIZE&&y>=d->position;
			else {y+=r->position.y-d->primaryRoom->position.y;return x<0&&y<d->position+DOORSIZE&&y>=d->position;}
			break;
	}
	return false;
}

door_struct* inDoorWay(doorCollection_struct* dc, room_struct* r, int x, int y, bool secondary, bool override)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && ((!secondary && dc->door[i].primaryRoom==r)||(secondary && dc->door[i].secondaryRoom==r)))
		{
			door_struct* d=&dc->door[i];
			if(checkDoorWay(d,r,x,y,secondary,override))return d;
		}
	}
	return NULL;
}

vect2D getDoorPosition2(room_struct* r, door_struct* d)
{
	vect2D v=vect2(0,0);
	if(!d || !d->primaryRoom)return v;
	room_struct* pr=d->primaryRoom;
	switch(d->direction)
	{
		case doorUp:
			v=vect2(d->position,0);
			break;
		case doorDown:
			v=vect2(d->position,pr->height);
			break;
		case doorLeft:
			v=vect2(0,d->position);
			break;
		case doorRight:
			v=vect2(pr->width,d->position);
			break;
	}
	v.x-=r->position.x-pr->position.x;
	v.y-=r->position.y-pr->position.y;
	return v;
}

bool checkDoorTouches(door_struct* d, roomEdit_struct* re, int x, int y, bool secondary)
{
	if(!d || !d->primaryRoom)return false;
	switch(d->direction)
	{
		case doorUp:
			if(!secondary)return y==0&&x<d->position+DOORSIZE&&x>=d->position;
			else {x+=re->position.x-d->primaryRoom->position.x;return y==re->size.y-1&&x<d->position+DOORSIZE&&x>=d->position;}
			break;
		case doorDown:
			if(!secondary)return (y==(re->size.y-1))&&x<d->position+DOORSIZE&&x>=d->position;
			else {x+=re->position.x-d->primaryRoom->position.x;return (y==0)&&x<d->position+DOORSIZE&&x>=d->position;}
			break;
		case doorLeft:
			if(!secondary)return x==0&&y<d->position+DOORSIZE&&y>=d->position;
			else {y+=re->position.y-d->primaryRoom->position.y;return x==re->size.x-1&&y<d->position+DOORSIZE&&y>=d->position;}
			break;
		case doorRight:
			if(!secondary)return (x==(re->size.x-1))&&y<d->position+DOORSIZE&&y>=d->position;
			else {y+=re->position.y-d->primaryRoom->position.y;return (x==0)&&y<d->position+DOORSIZE&&y>=d->position;}
			break;
	}
	return false;
}

door_struct* doorTouches(doorCollection_struct* dc, roomEdit_struct* re, int x, int y, bool secondary)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && ((!secondary && dc->door[i].primaryRoom==&re->data)||(secondary && dc->door[i].secondaryRoom==&re->data)))
		{
			door_struct* d=&dc->door[i];
			if(checkDoorTouches(d,re,x,y,secondary))return d;
		}
	}
	return NULL;
}

void doorGetSecondaryRoom(door_struct* d)
{
	if(!d || !d->primaryRoom)return;
	d->secondaryRoom=NULL;
	room_struct* pr=d->primaryRoom;
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used && &roomEdits[i].data!=pr)
		{
			room_struct* r=&roomEdits[i].data;
			switch(d->direction)
			{
				case doorUp:
					if(r->position.y+r->height==pr->position.y && r->position.x<d->position+pr->position.x && r->position.x+r->width>d->position+pr->position.x+DOORSIZE)
					{
						d->secondaryRoom=r;
						return;
					}
					break;
				case doorDown:
					if(r->position.y==pr->position.y+pr->height && r->position.x<d->position+pr->position.x && r->position.x+r->width>d->position+pr->position.x+DOORSIZE)
					{
						d->secondaryRoom=r;
						return;
					}
					break;
				case doorLeft:
					if(r->position.x+r->width==pr->position.x && r->position.y<d->position+pr->position.y && r->position.y+r->height>d->position+pr->position.y+DOORSIZE)
					{
						d->secondaryRoom=r;
						return;
					}
					break;
				case doorRight:
					if(r->position.x==pr->position.x+pr->width && r->position.y<d->position+pr->position.y && r->position.y+r->height>d->position+pr->position.y+DOORSIZE)
					{
						d->secondaryRoom=r;
						return;
					}
					break;
			}
		}
	}
}

void getSecondaryRooms(doorCollection_struct* dc)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used)
		{
			doorGetSecondaryRoom(&dc->door[i]);
			NOGBA("door %d : %p (vs %p)",i,dc->door[i].secondaryRoom,dc->door[i].primaryRoom);
		}
	}
}

door_struct* createDoor(doorCollection_struct* dc, room_struct* pr, u16 position, doorDirection dir, bool override)
{
	if(!dc)dc=&doorCollection;
	if(!override && doorCollides(dc, pr, dir, position, NULL))return NULL;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(!dc->door[i].used)
		{
			door_struct* d=&dc->door[i];
			d->used=true;
			d->open=false;
			d->selected=false;
			d->position=position;
			d->percentage=128;
			d->direction=dir;
			d->primaryRoom=pr;
			d->secondaryRoom=NULL;
			return d;
		}
	}
	return NULL;
}

bool confirmDoor(door_struct* d)
{
	if(!d||!d->primaryRoom)return false;
	int k1=0, k2=0;
	room_struct* pr=d->primaryRoom;
	switch(d->direction)
	{
		case doorUp:
			k1=d->position+0*pr->width;
			k2=d->position+1+0*pr->width;
			break;
		case doorDown:
			k1=d->position+(pr->height-1)*pr->width;
			k2=d->position+1+(pr->height-1)*pr->width;
			break;
		case doorRight:
			k1=(pr->width-1)+d->position*pr->width;
			k2=(pr->width-1)+(d->position+1)*pr->width;
			break;
		case doorLeft:
			k1=(0)+d->position*pr->width;
			k2=(0)+(d->position+1)*pr->width;
			break;
	}
	d->height=pr->floor[k1];
	return (pr->floor[k1]==pr->floor[k2])&&(pr->materials[k1]==pr->materials[k2])&&(pr->floor[k1]+DOORHEIGHT<pr->ceiling[k1])&&(pr->floor[k2]+DOORHEIGHT<pr->ceiling[k2]);
}

void confirmRoomDoors(doorCollection_struct* dc, room_struct* r)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && dc->door[i].primaryRoom==r && !confirmDoor(&dc->door[i]))deleteDoor(&dc->door[i]);
	}
}

void confirmDoors(doorCollection_struct* dc)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && !confirmDoor(&dc->door[i]))deleteDoor(&dc->door[i]);
	}
}

void drawDoor(roomEdit_struct* re, door_struct* d)
{
	if(!d)return;
	glPushMatrix();
		if(d->selected)glColor3b(0,0,255);
		else glColor3b(0,255,0);
		switch(d->direction)
		{
			case doorUp:
				glTranslate3f32(d->position*(1<<9),0,0);
				glScalef32(DOORSIZE*(1<<9),inttof32(1),inttof32(1));
				glBegin(GL_QUADS);
					glVertex3v16(0, OUTLINEWIDTH, 0);
					glVertex3v16(inttof32(1), OUTLINEWIDTH, 0);
					glVertex3v16(inttof32(1), 0, 0);
					glVertex3v16(0, 0, 0);
				break;
			case doorDown:
				glTranslate3f32(d->position*(1<<9),re->size.y*(1<<9)-OUTLINEWIDTH,0);
				glScalef32(DOORSIZE*(1<<9),inttof32(1),inttof32(1));
				glBegin(GL_QUADS);
					glVertex3v16(0, OUTLINEWIDTH, 0);
					glVertex3v16(inttof32(1), OUTLINEWIDTH, 0);
					glVertex3v16(inttof32(1), 0, 0);
					glVertex3v16(0, 0, 0);
				break;
			case doorRight:
				glTranslate3f32(re->size.x*(1<<9)-OUTLINEWIDTH,d->position*(1<<9),0);
				glScalef32(inttof32(1),DOORSIZE*(1<<9),inttof32(1));
				glBegin(GL_QUADS);
					glVertex3v16(0, 0, 0);
					glVertex3v16(0, inttof32(1), 0);
					glVertex3v16(OUTLINEWIDTH, inttof32(1), 0);
					glVertex3v16(OUTLINEWIDTH, 0, 0);
				break;
			case doorLeft:
				glTranslate3f32(0,d->position*(1<<9),0);
				glScalef32(inttof32(1),DOORSIZE*(1<<9),inttof32(1));
				glBegin(GL_QUADS);
					glVertex3v16(0, 0, 0);
					glVertex3v16(0, inttof32(1), 0);
					glVertex3v16(OUTLINEWIDTH, inttof32(1), 0);
					glVertex3v16(OUTLINEWIDTH, 0, 0);
				break;
		}
	glPopMatrix(1);
}

void drawRoomDoors(doorCollection_struct* dc, roomEdit_struct* re)
{
	if(!dc)dc=&doorCollection;
	int i;
	room_struct* r=&re->data;
	glPushMatrix();
		glTranslate3f32(re->position.x*(1<<9),re->position.y*(1<<9),64);
		for(i=0;i<NUMDOORS;i++)
		{
			if(dc->door[i].used && dc->door[i].primaryRoom==r)
			{
				door_struct* d=&dc->door[i];
				drawDoor(re, d);
			}
		}
	glPopMatrix(1);
}

void getDoorRectangle(door_struct* d, bool primary, rectangle_struct* rrrec)
{
	rectangle_struct rec=(rectangle_struct){vect(0,0,0),vect(0,0,0),vect(0,0,0)};
	if(!d || !d->primaryRoom)return;
	if(primary)
	{
		room_struct* pr=d->primaryRoom;
		switch(d->direction)
		{
			case doorUp:
				rec.position=vect(d->position,d->height,0);
				rec.size=vect(DOORSIZE,DOORHEIGHT,0);
				rec.normal=vect(0,0,-inttof32(1));
				break;
			case doorDown:
				rec.position=vect(d->position+DOORSIZE,d->height,pr->height);
				rec.size=vect(-DOORSIZE,DOORHEIGHT,0);
				rec.normal=vect(0,0,inttof32(1));
				break;
			case doorLeft:
				rec.position=vect(0,d->height,d->position+DOORSIZE);
				rec.size=vect(0,DOORHEIGHT,-DOORSIZE);
				rec.normal=vect(-inttof32(1),0,0);
				break;
			case doorRight:
				rec.position=vect(pr->width,d->height,d->position);
				rec.size=vect(0,DOORHEIGHT,DOORSIZE);
				rec.normal=vect(inttof32(1),0,0);
				break;
		}
	}else{
		if(!d->secondaryRoom)return;
		room_struct* pr=d->secondaryRoom;
		switch(d->direction)
		{
			case doorUp:
				rec.position=vect(d->primaryRoom->position.x-d->secondaryRoom->position.x+d->position+DOORSIZE,d->height,pr->height);
				rec.size=vect(-DOORSIZE,DOORHEIGHT,0);
				rec.normal=vect(0,0,-inttof32(1));
				break;
			case doorDown:
				rec.position=vect(d->primaryRoom->position.x-d->secondaryRoom->position.x+d->position,d->height,0);
				rec.size=vect(DOORSIZE,DOORHEIGHT,0);
				rec.normal=vect(0,0,inttof32(1));
				break;
			case doorLeft:
				rec.position=vect(pr->width,d->height,d->primaryRoom->position.y-d->secondaryRoom->position.y+d->position);
				rec.size=vect(0,DOORHEIGHT,DOORSIZE);
				rec.normal=vect(-inttof32(1),0,0);
				break;
			case doorRight:
				rec.position=vect(0,d->height,d->primaryRoom->position.y-d->secondaryRoom->position.y+d->position+DOORSIZE);
				rec.size=vect(0,DOORHEIGHT,-DOORSIZE);
				rec.normal=vect(inttof32(1),0,0);
				break;
		}
	}
	rrrec->position=rec.position;
	rrrec->size=rec.size;
	rrrec->normal=rec.normal;
}

bool collideLineDoor(door_struct* d, vect3D o, vect3D v, int32 dist)
{
	if(!d || !d->primaryRoom)return false;
	rectangle_struct rec;
	getDoorRectangle(d,true,&rec);
	return collideLineRectangle(&rec, o, v, dist, NULL, NULL);
}

bool collideSegmentDoors(doorCollection_struct* dc, room_struct* r, vect3D p1, vect3D p2)
{
	if(!dc)dc=&doorCollection;
	vect3D u=vectDifference(p2,p1);
	
	//(precision problem)
	u=vectMultInt(u,100);
	int32 dist=magnitude(u);
	u=divideVect(u,dist);
	dist/=100;
	
	vect3D o=p1;
	
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && dc->door[i].primaryRoom)
		{
			room_struct* pr=dc->door[i].primaryRoom;
			o=convertSize(vectDifference(vect(r->position.x,0,r->position.y),vect(pr->position.x,0,pr->position.y)));
			o=addVect(p1,convertSize(vectDifference(vect(r->position.x,0,r->position.y),vect(pr->position.x,0,pr->position.y))));
			if(collideLineDoor(&dc->door[i],o,u,dist))return true;
		}
	}
	return false;
}

void renderDoorRectangle(door_struct* d, rectangle_struct rec)
{
	s16 percentage=d->percentage;
	vect3D pos=convertVect(rec.position);
	vect3D size=convertSize(rec.size);
	size.y*=percentage;
	size.y/=128;
	vect3D p1=vect(0,(inttot16(63)*(128-percentage))/128,0);
	vect3D p2=vect(inttot16(63),inttot16(63),0);
	int32 t1=TEXTURE_PACK(p1.x, p1.y);
	int32 t2=TEXTURE_PACK(p2.x, p1.y);
	int32 t3=TEXTURE_PACK(p2.x, p2.y);
	int32 t4=TEXTURE_PACK(p1.x, p2.y);
	if(!size.x)
	{
		glBegin(GL_QUAD);
			GFX_TEX_COORD = t2;
			glVertex3v16(pos.x, pos.y, pos.z);

			GFX_TEX_COORD = t1;
			glVertex3v16(pos.x, pos.y, pos.z+size.z);

			GFX_TEX_COORD = t4;
			glVertex3v16(pos.x, pos.y+size.y, pos.z+size.z);

			GFX_TEX_COORD = t3;
			glVertex3v16(pos.x, pos.y+size.y, pos.z);
	}else{
		glBegin(GL_QUAD);
			GFX_TEX_COORD = t2;
			glVertex3v16(pos.x+size.x, pos.y, pos.z);

			GFX_TEX_COORD = t3;
			glVertex3v16(pos.x+size.x, pos.y+size.y, pos.z);

			GFX_TEX_COORD = t4;
			glVertex3v16(pos.x, pos.y+size.y, pos.z);

			GFX_TEX_COORD = t1;
			glVertex3v16(pos.x, pos.y, pos.z);
	}
}

void renderDoorLightmap(door_struct* d, rectangle_struct rec)
{
	s16 percentage=d->percentage;
	vect3D pos=convertVect(rec.position);
	vect3D size=convertSize(rec.size);
	size.y*=percentage;
	size.y/=128;
	
	vect3D p1=vect(inttot16(rec.lmPos.x),inttot16(rec.lmPos.y),0);
	vect3D p2=vect(inttot16(rec.lmPos.x+rec.lmSize.x-1),inttot16(rec.lmPos.y+rec.lmSize.y-1),0);
	int32 t1=TEXTURE_PACK(p1.x, p1.y);
	int32 t2=TEXTURE_PACK(p1.x, p2.y);
	int32 t3=TEXTURE_PACK(p2.x, p2.y);
	int32 t4=TEXTURE_PACK(p2.x, p1.y);
	if(rec.rot)
	{
		p2=vect(inttot16(rec.lmPos.x+rec.lmSize.y-1),inttot16(rec.lmPos.y+rec.lmSize.x-1),0);
		t1=TEXTURE_PACK(p1.x, p1.y);
		t4=TEXTURE_PACK(p1.x, p2.y);
		t3=TEXTURE_PACK(p2.x, p2.y);
		t2=TEXTURE_PACK(p2.x, p1.y);
	}
	
	if(!size.x)
	{
		glBegin(GL_QUAD);
			GFX_TEX_COORD = t1;
			glVertex3v16(pos.x, pos.y, pos.z);

			GFX_TEX_COORD = t2;
			glVertex3v16(pos.x, pos.y, pos.z+size.z);

			GFX_TEX_COORD = t3;
			glVertex3v16(pos.x, pos.y+size.y, pos.z+size.z);

			GFX_TEX_COORD = t4;
			glVertex3v16(pos.x, pos.y+size.y, pos.z);
	}else{
		glBegin(GL_QUAD);
			GFX_TEX_COORD = t1;
			glVertex3v16(pos.x+size.x, pos.y, pos.z);

			GFX_TEX_COORD = t2;
			glVertex3v16(pos.x+size.x, pos.y+size.y, pos.z);

			GFX_TEX_COORD = t3;
			glVertex3v16(pos.x, pos.y+size.y, pos.z);

			GFX_TEX_COORD = t4;
			glVertex3v16(pos.x, pos.y, pos.z);
	}
}

void renderDoor(room_struct* r, door_struct* d)
{
	glPushMatrix();
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
		
		applyMTL(doorMtl);
		
		if(r==d->primaryRoom)renderDoorRectangle(d, d->primaryRec);
		else if(r==d->secondaryRoom)renderDoorRectangle(d, d->secondaryRec);
				
		if(r->lmSlot)
		{
			glPolyFmt(POLY_ALPHA(31) | (1<<14) | POLY_CULL_BACK);
			applyMTL(r->lmSlot->mtl);
			if(r==d->primaryRoom)renderDoorLightmap(d, d->primaryRec);
			else if(r==d->secondaryRoom)renderDoorLightmap(d, d->secondaryRec);
		}
	glPopMatrix(1);
}

void getRoomDoorRectangles(doorCollection_struct* dc, room_struct* r)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && (r==dc->door[i].primaryRoom || r==dc->door[i].secondaryRoom))
		{
			door_struct* d=&dc->door[i];
			getDoorRectangle(d,true,&d->primaryRec);
			getDoorRectangle(d,false,&d->secondaryRec);
		}
	}
}

void renderRoomDoors(doorCollection_struct* dc, room_struct* r)
{
	if(!dc)dc=&doorCollection;
	glPushMatrix();
		int i;
		for(i=0;i<NUMDOORS;i++)
		{
			if(dc->door[i].used && (dc->door[i].primaryRoom==r||dc->door[i].secondaryRoom==r))
			{
				door_struct* d=&dc->door[i];
				renderDoor(r, d);
			}
		}
	glPopMatrix(1);
}

void updateDoor(door_struct* d)
{
	if(!d)return;
	if(!d->open && d->percentage<128)
	{
		d->percentage-=4;
		if(d->percentage<=0)
		{
			d->percentage=0;
			d->open=true;
			d->timer=0;
		}
	}else if(d->open && d->percentage>0)
	{
		d->percentage+=4;
		if(d->percentage>=128)
		{
			d->percentage=128;
			d->open=false;
		}
	}
	if(d->open && !d->percentage)
	{
		d->timer++;
		if(d->timer>DOORPERIOD)d->percentage++;
	}
}

void updateDoors(doorCollection_struct* dc)
{
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used)updateDoor(&dc->door[i]);
	}
}

bool toggleDoor(room_struct* r, door_struct* d)
{
	if(!d || !d->secondaryRoom)return false;
	if(!d->open && d->percentage){d->percentage--;return true;}
	else if(d->open && !d->percentage){d->percentage++;return true;}
	return false;
}

bool isDoorOpen(doorCollection_struct* dc, room_struct* r1, room_struct* r2)
{
	if(!r1 || !r2)return false;
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		door_struct* d=&dc->door[i];
		if(d->used && ((d->primaryRoom==r1 && d->secondaryRoom==r2) || (d->primaryRoom==r2 && d->secondaryRoom==r1)))
		{
			if((d->open || (d->percentage!=0 && d->percentage!=128)))return true;
		}
	}
	return false;
}

void closeRoomDoors(doorCollection_struct* dc, room_struct* r, door_struct* d)
{
	if(!r)return;
	if(!dc)dc=&doorCollection;
	int i;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && d!=&dc->door[i] && (dc->door[i].primaryRoom==r || dc->door[i].secondaryRoom==r))
		{
			dc->door[i].open=false;
			dc->door[i].percentage=128;
		}
	}
}

door_struct* getClosestDoorRoom(doorCollection_struct* dc, room_struct* r, vect2D p, int* distt)
{
	if(!dc)dc=&doorCollection;
	int i;
	int mindist=999999999;
	door_struct* closestDoor=NULL;
	for(i=0;i<NUMDOORS;i++)
	{
		if(dc->door[i].used && (dc->door[i].primaryRoom==r||dc->door[i].secondaryRoom==r))
		{
			door_struct* d=&dc->door[i];
			vect2D v=getDoorPosition2(r, d);
			int dist=(v.x-p.x)*(v.x-p.x)+(v.y-p.y)*(v.y-p.y);
			if(dist<mindist)
			{
				mindist=dist;
				closestDoor=d;
			}
			
		}
	}
	*distt=mindist;
	return closestDoor;
}
