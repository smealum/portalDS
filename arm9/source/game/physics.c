#include "game/game_main.h"

#define PLAYERSIZEY (TILESIZE*3)
#define PLAYERSIZEY2 (TILESIZE)
#define PLAYERSIZEX (TILESIZE-32)

#define BOXNUM 10

extern roomEdit_struct roomEdits[NUMROOMEDITS];
extern platform_struct platform[NUMPLATFORMS];

vect3D boxDefinition[]={vect(0,-PLAYERSIZEY,0),
						vect(-PLAYERSIZEX,-PLAYERSIZEY+MAXSTEP*HEIGHTUNIT+2,-PLAYERSIZEX),
						vect(PLAYERSIZEX,-PLAYERSIZEY+MAXSTEP*HEIGHTUNIT+2,-PLAYERSIZEX),
						vect(PLAYERSIZEX,-PLAYERSIZEY+MAXSTEP*HEIGHTUNIT+2,PLAYERSIZEX),
						vect(-PLAYERSIZEX,-PLAYERSIZEY+MAXSTEP*HEIGHTUNIT+2,PLAYERSIZEX),
						vect(0,PLAYERSIZEY2,0),
						vect(-PLAYERSIZEX,PLAYERSIZEY2,-PLAYERSIZEX),
						vect(PLAYERSIZEX,PLAYERSIZEY2,-PLAYERSIZEX),
						vect(PLAYERSIZEX,PLAYERSIZEY2,PLAYERSIZEX),
						vect(-PLAYERSIZEX,PLAYERSIZEY2,PLAYERSIZEX)};

bool pointInRoom(room_struct* r, vect3D p, vect3D* v)
{
	if(!r)return false;
	vect3D rp=vectDifference(p,convertVect(vect(r->position.x,0,r->position.y)));
	vect3D s=convertSize(vect(r->width,0,r->height));
	if(v)*v=rp;
	return rp.x>=0&&rp.z>=0&&rp.x<s.x&&rp.z<s.z;
}

bool objectInRoom(room_struct* r, physicsObject_struct* o, vect3D* v)
{
	if(!r || !o)return false;
	return pointInRoom(r,o->position,v);
}

room_struct* getRoomPoint(vect3D p)
{
	int i;
	for(i=0;i<NUMROOMEDITS;i++)
	{
		if(roomEdits[i].used && pointInRoom(&roomEdits[i].data, p, NULL))
		{
			return &roomEdits[i].data;
		}
	}
	return NULL;
}

vect3D convertCoord(room_struct* r, vect3D p)
{
	if(!r)return vect(0,0,0);
	vect3D rp=vectDifference(p,convertVect(vect(r->position.x,0,r->position.y)));
	// rp.x+=TILESIZE;
	// rp.z+=TILESIZE;
	if(rp.x>=0)rp.x/=TILESIZE*2;
	else rp.x=rp.x/(TILESIZE*2)-1;
	if(rp.z>=0)rp.z/=TILESIZE*2;
	else rp.z=rp.z/(TILESIZE*2)-1;
	if(rp.y>=0)rp.y/=HEIGHTUNIT;
	else rp.y=rp.y/(HEIGHTUNIT)-1;
	return rp;
}

bool boxInRoom(room_struct* r, physicsObject_struct* o)
{
	int j;
	for(j=0;j<BOXNUM;j++)
	{
		vect3D t;
		vect3D p=addVect(o->position,boxDefinition[j]);
		if(pointInRoom(r,p,&t))return true;
	}
	return false;
}

const int32 transX=inttof32(1);
const int32 transY=inttof32(5);
const int32 transZ=inttof32(1);

bool checkObjectCollisionCell(gridCell_struct* gc, physicsObject_struct* o, room_struct* r)
{
	if(!gc || !o || !r)return false;
	vect3D o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));
	// o1.y-=128; //make us taller
	int i;
	bool ret=false;
	for(i=0;i<gc->numRectangles;i++)
	{
		rectangle_struct* rec=gc->rectangles[i];
		vect3D o2=getClosestPointRectangleStruct(rec,o1);
			if(portal1.used&&portal2.used)
			{
				o2=addVect(o2,convertVect(vect(r->position.x,0,r->position.y)));
				collidePortal(r,rec,&portal1,&o2);
				collidePortal(r,rec,&portal2,&o2);
				o2=vectDifference(o2,convertVect(vect(r->position.x,0,r->position.y)));
			}
		vect3D v=vectDifference(o2,o1);
		rec->touched=false;
		// int sqd=sqMagnitude(v);
		int32 sqd=divf32(mulf32(v.x,v.x),transX)+divf32(mulf32(v.y,v.y),transY)+divf32(mulf32(v.z,v.z),transZ);
		if(sqd<o->sqRadius)
		{
			// sqd=v.x*v.x+v.y*v.y+v.z*v.z;
			int32 sqd=divf32((v.x*v.x),transX)+divf32((v.y*v.y),transY)+divf32((v.z*v.z),transZ);
			u32 d=sqrtf32((sqd));
			v=divideVect(vectMult(vect(v.x,v.y,v.z),-((o->radius<<6)-d)),d);
			o->position=addVect(o->position,v);
			o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));
			rec->touched=true;
			ret=true;
		}
	}
	return ret;
}

bool checkObjectCollision(physicsObject_struct* o, room_struct* r)
{	
	vect3D o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));
	// o1.y-=128; //make us taller
	
	bool ret=false;
	
	gridCell_struct* gc=getCurrentCell(r,o->position);
	if(!gc)return false;
	ret=ret||checkObjectCollisionCell(gc,o,r);
	/*vect3D v=vect(o1.x/(CELLSIZE*TILESIZE*2),0,o1.z/(CELLSIZE*TILESIZE*2));
	vect3D v1=vect((o1.x+o->radius)/(CELLSIZE*TILESIZE*2),0,(o1.z+o->radius)/(CELLSIZE*TILESIZE*2));
	vect3D v2=vect((o1.x+o->radius)/(CELLSIZE*TILESIZE*2),0,(o1.z+o->radius)/(CELLSIZE*TILESIZE*2));
	if(v1.x!=v.x && v1.x>=0 && v1.x<r->rectangleGridSize.x)
	{
		ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(o->radius,0,0))),o,r);
		if(v1.y!=v.y && v1.y>=0 && v1.y<r->rectangleGridSize.y)
		{
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(0,0,o->radius))),o,r);
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(o->radius,0,o->radius))),o,r);
		}else if(v2.y!=v.y && v2.y>=0 && v2.y<r->rectangleGridSize.y){
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(0,0,-o->radius))),o,r);
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(o->radius,0,-o->radius))),o,r);
		}
	}else if(v2.x!=v.x && v2.x>=0 && v2.x<r->rectangleGridSize.x)
	{
		ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(-o->radius,0,0))),o,r);
		if(v1.y!=v.y && v1.y>=0 && v1.y<r->rectangleGridSize.y)
		{
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(0,0,o->radius))),o,r);
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(-o->radius,0,o->radius))),o,r);
		}else if(v2.y!=v.y && v2.y>=0 && v2.y<r->rectangleGridSize.y){
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(0,0,-o->radius))),o,r);
			ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(-o->radius,0,-o->radius))),o,r);
		}
	}else if(v1.y!=v.y && v1.y>=0 && v1.y<r->rectangleGridSize.y)
	{
		ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(0,0,o->radius))),o,r);
	}else if(v2.y!=v.y && v2.y>=0 && v2.y<r->rectangleGridSize.y)
	{
		ret=ret||checkObjectCollisionCell(getCurrentCell(r,addVect(o->position,vect(0,0,-o->radius))),o,r);
	}*/
	
	//platforms
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used) //add culling
		{
			platform_struct* pf=&platform[i];
			vect3D o2=getClosestPointRectangle(addVect(pf->position,vect(-PLATFORMSIZE,0,-PLATFORMSIZE)),vect(PLATFORMSIZE*2,0,PLATFORMSIZE*2),o->position);
			vect3D v=vectDifference(o2,o->position);
			int32 sqd=divf32(mulf32(v.x,v.x),transX)+divf32(mulf32(v.y,v.y),transY)+divf32(mulf32(v.z,v.z),transZ);
			if(sqd<o->sqRadius)
			{
				int32 sqd=divf32((v.x*v.x),transX)+divf32((v.y*v.y),transY)+divf32((v.z*v.z),transZ);
				u32 d=sqrtf32((sqd));
				v=divideVect(vectMult(vect(v.x,v.y,v.z),-((o->radius<<6)-d)),d);
				o->position=addVect(o->position,v);
				o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));
				pf->touched=true;
				ret=true;
			}
		}
	}
	
	return ret;
}

void collideObjectRoom(physicsObject_struct* o, room_struct* r)
{
	if(!o)return;
	if(!r)return;
	vect3D pos=o->position;
	o->speed.y-=16;
	// if(o->speed.y>200)o->speed.y=200;
	// else if(o->speed.y<-200)o->speed.y=-200; //temporary limit (hopefully)
	if(o->speed.y>800)o->speed.y=800;
	else if(o->speed.y<-800)o->speed.y=-800;
	
	int32 length=magnitude(o->speed);
	
	iprintf("length %d  \n",length);
	
	bool ret=false;
	
	if(length<200)
	{
		o->position=addVect(o->position,o->speed);
		ret=checkObjectCollision(o,r);
	}else{
		vect3D v=divideVect(o->speed,length);
		v=vectDivInt(v,32);
		
		while(length>128)
		{
			o->position=addVect(o->position,v);
			ret=ret||checkObjectCollision(o,r);
			length-=128;
		}
		
		v=vectMult(v,length*32);
		o->position=addVect(o->position,v);
		checkObjectCollision(o,r);
	}
	
	o->contact=ret;
	
	// o->speed=vect(0,o->position.y-pos.y,0);
	o->speed=vect(o->position.x-pos.x,o->position.y-pos.y,o->position.z-pos.z);
	if(o->contact){o->speed.x-=o->speed.x/2;o->speed.z-=o->speed.z/2;} //floor friction
	else {o->speed.x-=o->speed.x/32;o->speed.z-=o->speed.z/32;} //air friction
	if(abs(o->speed.x)<3)o->speed.x=0;
	if(abs(o->speed.z)<3)o->speed.z=0;
}
