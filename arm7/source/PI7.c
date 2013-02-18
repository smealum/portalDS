#include "stdafx.h"

bool PI7running;
player_struct player;
portal_struct portal[2];

void initPI7(void)
{
	initOBBs();
	initAARs();
	initPlatforms();
	
	player.position=vect(0,0,0);
	
	portal[0].targetPortal=&portal[1];
	portal[1].targetPortal=&portal[0];
	portal[0].used=false;portal[1].used=false;
	
	PI7running=false;
}

bool getPI7Status(void)
{
	return PI7running;
}

void listenPI7(void)
{
	while(fifoCheckValue32(FIFO_USER_08))
	{
		u32 signal=fifoGetValue32(FIFO_USER_08);
		switch(signal&PISIGNALMASK)
		{
			case PI_START:
				PI7running=true;
				break;
			case PI_PAUSE:
				PI7running=false;
				break;
			case PI_STOP:
				PI7running=false;
				break;
			case PI_RESET:
				initOBBs();
				PI7running=true;
				break;
			case PI_ADDBOX:
				{
					vect3D pos, size;
					u32 mass;
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						u32 x=fifoGetValue32(FIFO_USER_08);
						size.x=x&((1<<16)-1);
						size.y=(x>>16)&((1<<16)-1);
					while(!fifoCheckValue32(FIFO_USER_08));
						x=fifoGetValue32(FIFO_USER_08);
						size.z=x&((1<<16)-1);
						mass=(x>>16)&((1<<16)-1);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.z=fifoGetValue32(FIFO_USER_08);
					createOBB(id,size,pos,mass);
				}
				break;
			case PI_APPLYFORCE:
				{
					vect3D pos, v;
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						u32 x=fifoGetValue32(FIFO_USER_08);
						pos.x=(s16)x;
						pos.y=(s16)(x>>16);
					while(!fifoCheckValue32(FIFO_USER_08));
						x=fifoGetValue32(FIFO_USER_08);
						pos.z=(s16)x;
					while(!fifoCheckValue32(FIFO_USER_08));
						v.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						v.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						v.z=fifoGetValue32(FIFO_USER_08);
					if(id<NUMOBJECTS && objects[id].used)
					{
						applyOBBForce(&objects[id],addVect(objects[id].position,pos),v);
						objects[id].sleep=false;
						objects[id].counter=0;
					}
				}
				break;
			case PI_ADDAAR:
				{
					vect3D pos, size;
					vect3D normal=vect(0,0,0);
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						size.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						size.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						size.z=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						int32 x=fifoGetValue32(FIFO_USER_08);
						if(x&1)normal.x=-inttof32(1);
						else if(x&2)normal.x=inttof32(1);
						if(x&4)normal.y=-inttof32(1);
						else if(x&8)normal.y=inttof32(1);
						if(x&16)normal.z=-inttof32(1);
						else if(x&32)normal.z=inttof32(1);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.z=fifoGetValue32(FIFO_USER_08);
					createAAR(id,pos,size,normal);
				}
				break;
			case PI_MAKEGRID:
				generateGrid(NULL);
				break;
			case PI_SETVELOCITY:
				{
					vect3D v;
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						v.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						v.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						v.z=fifoGetValue32(FIFO_USER_08);
					if(id<NUMOBJECTS && objects[id].used)
					{
						objects[id].velocity=v;
						objects[id].sleep=false;
						objects[id].counter=0;
					}
				}
				break;
			case PI_UPDATEPLAYER:
				{
					vect3D v;
					while(!fifoCheckValue32(FIFO_USER_08));
						v.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						v.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						v.z=fifoGetValue32(FIFO_USER_08);
					player.position=v;
				}
				break;
			case PI_UPDATEPORTAL:
				{
					vect3D pos;
					vect3D normal=vect(0,0,0);
					vect3D plane0=vect(0,0,0);
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.z=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						u32 x=fifoGetValue32(FIFO_USER_08);
						if(x&1)normal.x=-inttof32(1);
						else if(x&2)normal.x=inttof32(1);
						if(x&4)normal.y=-inttof32(1);
						else if(x&8)normal.y=inttof32(1);
						if(x&16)normal.z=-inttof32(1);
						else if(x&32)normal.z=inttof32(1);
					while(!fifoCheckValue32(FIFO_USER_08));
						plane0.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						plane0.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						plane0.z=fifoGetValue32(FIFO_USER_08);
					if(id<2)
					{
						portal[id].used=true;
						portal[id].position=pos;
						portal[id].normal=normal;
						portal[id].plane[0]=plane0;
						fifoSendValue32(FIFO_USER_08, plane0.x);
						fifoSendValue32(FIFO_USER_08, plane0.y);
						fifoSendValue32(FIFO_USER_08, plane0.z);
						computePortalPlane(&portal[id]);
						wakeOBBs();
					}
				}
				break;
			case PI_ADDPLATFORM:
				{
					vect3D orig, dest;
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						orig.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						orig.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						orig.z=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						dest.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						dest.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						dest.z=fifoGetValue32(FIFO_USER_08);
					createPlatform(id%NUMPLATFORMS,orig,dest,id>=NUMPLATFORMS);
				}
				break;
			case PI_UPDATEPLATFORM:
				{
					vect3D pos;
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.x=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.y=fifoGetValue32(FIFO_USER_08);
					while(!fifoCheckValue32(FIFO_USER_08));
						pos.z=fifoGetValue32(FIFO_USER_08);
					movePlatform(id,pos);
				}
				break;
			case PI_TOGGLEPLATFORM:
				{
					u8 id=signal>>PISIGNALDATA;
					while(!fifoCheckValue32(FIFO_USER_08));
						bool active=fifoGetValue32(FIFO_USER_08);
					togglePlatform(id,active);
				}
				break;
			case PI_KILLBOX:
				{
					u32 mass;
					u8 id=signal>>PISIGNALDATA;
					if(id<NUMOBJECTS)
					{
						objects[id].used=false;
					}
				}
				break;
			case PI_RESETPORTALS:
				{
					portal[0].used=false;
					portal[1].used=false;
				}
				break;
			default:
				while(fifoCheckValue32(FIFO_USER_08))fifoGetValue32(FIFO_USER_08);
				break;
		}
	}
}

void sendDataPI7(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used && !objects[i].sleep)
		{
			fifoSendValue32(FIFO_USER_01,(((u16)(objects[i].groundID+1))<<16)|i);
			fifoSendValue32(FIFO_USER_02,objects[i].position.x);
			fifoSendValue32(FIFO_USER_03,objects[i].position.y);
			fifoSendValue32(FIFO_USER_04,objects[i].position.z);
			fifoSendValue32(FIFO_USER_05,((objects[i].transformationMatrix[3]+4096)<<16)|((u16)objects[i].transformationMatrix[0]+4096));
			fifoSendValue32(FIFO_USER_06,((objects[i].transformationMatrix[1]+4096)<<16)|((u16)objects[i].transformationMatrix[6]+4096));
			fifoSendValue32(FIFO_USER_07,((objects[i].transformationMatrix[7]+4096)<<16)|((u16)objects[i].transformationMatrix[4]+4096));
		}
	}
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)
		{
			fifoSendValue32(FIFO_USER_01,i+NUMOBJECTS);
			fifoSendValue32(FIFO_USER_02,platform[i].position.x);
			fifoSendValue32(FIFO_USER_03,platform[i].position.y);
			fifoSendValue32(FIFO_USER_04,platform[i].position.z);
		}
	}
}
