#include "game/game_main.h"

#define PLAYERSIZEY (TILESIZE*3)
#define PLAYERSIZEY2 (TILESIZE)
#define PLAYERSIZEX (TILESIZE-32)

#define BOXNUM 10

#define ELEVATOR_WIDTH (TILESIZE*2)
#define ELEVATOR_WIDTH_SQ ((ELEVATOR_WIDTH*ELEVATOR_WIDTH)>>12)
#define ELEVATOR_ANGLE (3084)

extern platform_struct platform[NUMPLATFORMS];

vect3D gravityVector=(vect3D){0,-16,0};
vect3D normGravityVector=(vect3D){0,-4096,0};

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

const int32 transY=inttof32(5);

bool checkObjectCollisionCell(gridCell_struct* gc, physicsObject_struct* o, room_struct* r)
{
	if(!gc || !o || !r)return false;
	vect3D o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));

	int i;
	bool ret=false;
	for(i=0;i<gc->numRectangles;i++)
	{
		rectangle_struct* rec=gc->rectangles[i];
		if(rec->collides)
		{
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
			int32 gval=dotProduct(v,normGravityVector);
			vect3D v2=vectDifference(v,vectMult(normGravityVector,gval));
			int32 sqd=mulf32(v2.x,v2.x)+mulf32(v2.y,v2.y)+mulf32(v2.z,v2.z)+divf32(mulf32(gval,gval),transY);
			if(sqd<o->sqRadius)
			{
				// sqd=v.x*v.x+v.y*v.y+v.z*v.z;
				int32 sqd=(v2.x*v.x)+(v2.y*v.y)+(v2.z*v2.z)+divf32(gval*gval,transY);
				u32 d=sqrtf32((sqd));
				v=divideVect(vectMult(vect(v.x,v.y,v.z),-((o->radius<<6)-d)),d);
				o->position=addVect(o->position,v);
				o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));
				rec->touched=true;
				ret=true;
			}
		}
	}
	return ret;
}

bool collideRectangle(physicsObject_struct* o, room_struct* r, vect3D p, vect3D s)
{
	if(!o||!r)return false;
	vect3D o2=getClosestPointRectangle(p,s,o->position);
	vect3D v=vectDifference(o2,o->position);
	int32 gval=dotProduct(v,normGravityVector);
	vect3D v2=vectDifference(v,vectMult(normGravityVector,gval));
	int32 sqd=mulf32(v2.x,v2.x)+mulf32(v2.y,v2.y)+mulf32(v2.z,v2.z)+divf32(mulf32(gval,gval),transY);
	if(sqd<o->sqRadius)
	{
		int32 sqd=(v2.x*v2.x)+(v2.y*v2.y)+(v2.z*v2.z)+divf32(gval*gval,transY);
		u32 d=sqrtf32((sqd));
		v=divideVect(vectMult(vect(v.x,v.y,v.z),-((o->radius<<6)-d)),d);
		o->position=addVect(o->position,v);
		return true;
	}
	return false;
}

bool checkObjectElevatorCollision(physicsObject_struct* o, room_struct* r, elevator_struct* ev)
{
	if(!o || !r || !ev)return false;

	bool ret=false;

	if(collideRectangle(o,r,addVect(ev->realPosition,vect(-ELEVATOR_SIZE/2,0,-ELEVATOR_SIZE/2)),vect(ELEVATOR_SIZE,0,ELEVATOR_SIZE)))ret=true;

	vect3D u=vect(o->position.x-ev->position.x,0,o->position.z-ev->position.z);
	int32 v=magnitude(u);

	switch(ev->direction&(~(1<<ELEVATOR_UPDOWNBIT)))
	{
		case 1:
			if(u.x<-mulf32(v,cosLerp(ELEVATOR_ANGLE)))return ret;
			break;
		case 4:
			if(u.z>mulf32(v,cosLerp(ELEVATOR_ANGLE)))return ret;
			break;
		case 5:
			if(u.z<-mulf32(v,cosLerp(ELEVATOR_ANGLE)))return ret;
			break;
		default:
			if(u.x>mulf32(v,cosLerp(ELEVATOR_ANGLE)))return ret;
			break;
	}

	if(v<ELEVATOR_WIDTH)
	{
		if(v+o->radius>=ELEVATOR_WIDTH)
		{
			u=divideVect(vectMult(u,ELEVATOR_WIDTH-o->radius-v),v);
			o->position=addVect(o->position,u);
			ret=true;
		}
	}else if(v<o->radius+ELEVATOR_WIDTH)
	{
		u=divideVect(vectMult(u,o->radius+ELEVATOR_WIDTH-v),v);
		o->position=addVect(o->position,u);
		ret=true;
	}

	return ret;
}

bool checkObjectCollision(physicsObject_struct* o, room_struct* r)
{	
	// vect3D o1=vectDifference(o->position,convertVect(vect(r->position.x,0,r->position.y)));
	// o1.y-=128; //make ourselves taller
	
	bool ret=false;
	
	gridCell_struct* gc=getCurrentCell(r,o->position);
	if(!gc)return false;
	ret=ret||checkObjectCollisionCell(gc,o,r);

	//platforms
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used && collideRectangle(o,r,addVect(platform[i].position,vect(-PLATFORMSIZE,0,-PLATFORMSIZE)),vect(PLATFORMSIZE*2,0,PLATFORMSIZE*2))) //add culling
		{
			platform[i].touched=true;
			ret=true;
		}
	}

	//elevators
	if(entryWallDoor.used && checkObjectElevatorCollision(o,r,&entryWallDoor.elevator))ret=true;
	
	return ret;
}

void changeGravity(vect3D v, int32 l)
{
	normGravityVector=v;
	gravityVector=vectMult(v,l);
}

void collideObjectRoom(physicsObject_struct* o, room_struct* r)
{
	if(!o)return;
	if(!r)return;
	vect3D pos=o->position;

	o->speed=addVect(o->speed, gravityVector);

	if(o->speed.y>800)o->speed.y=800;
	else if(o->speed.y<-800)o->speed.y=-800;
	
	int32 length=magnitude(o->speed);
	
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
	
	vect3D os=o->speed;
	o->speed=vect(o->position.x-pos.x,o->position.y-pos.y,o->position.z-pos.z);
	o->speed=vect((os.x*o->speed.x>0)?(o->speed.x):(0),(os.y*o->speed.y>0)?(o->speed.y):(0),(os.z*o->speed.z>0)?(o->speed.z):(0));

	if(o->contact)
	{
		 //floor friction
		vect3D s=vectDifference(o->speed,vectMult(normGravityVector,dotProduct(normGravityVector,o->speed)));
		o->speed=vectDifference(o->speed,vectDivInt(s,2));
	}else{
		//air friction
		vect3D s=vectDifference(o->speed,vectMult(normGravityVector,dotProduct(normGravityVector,o->speed)));
		o->speed=vectDifference(o->speed,vectDivInt(s,32));
	}

	if(abs(o->speed.x)<3)o->speed.x=0;
	if(abs(o->speed.z)<3)o->speed.z=0;
}
