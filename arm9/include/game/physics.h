#ifndef __PHYSICS9__
#define __PHYSICS9__

#define MAXSTEP 2
#define GRAVITY (inttof32(1)>>7)
#define MARGIN (1)

typedef struct
{
	vect3D position, speed;
	int32 radius, sqRadius;
	bool contact;
}physicsObject_struct;

room_struct* getRoomPoint(vect3D p);
void updatePhysicsObject(physicsObject_struct* o);
void collideObjectRoom(physicsObject_struct* o, room_struct* r);
bool objectInRoom(room_struct* r, physicsObject_struct* o, vect3D* v);
s16 updateSimplePhysicsObjectRoom(room_struct* r, physicsObject_struct* o);
void updatePhysicsObjectRoom(room_struct* r, physicsObject_struct* o, bool both);
vect3D convertCoord(room_struct* r, vect3D p);

#endif
