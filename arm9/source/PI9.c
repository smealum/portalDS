#include <nds.h>
#include "game/game_main.h"

#define NUMOBBFACES 6

const u8 OBBFaces[NUMOBBFACES][4]={{0,1,2,3},{4,5,6,7},{0,5,4,3},{0,1,6,5},{1,2,7,6},{2,3,4,7}};

OBB_struct objects[NUMOBJECTS];
AAR_struct aaRectangles[NUMAARS];
u8 selectID; //debug

void initPI9(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		objects[i].used=false;
		objects[i].id=i;
	}
	for(i=0;i<NUMAARS;i++)
	{
		aaRectangles[i].used=false;
		aaRectangles[i].id=i;
	}
	selectID=0;
}

OBB_struct* newBox(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(!objects[i].used)
		{
			objects[i].used=true;
			return &objects[i];
		}
	}
	return NULL;
} 

AAR_struct* newAAR(void)
{
	int i;
	for(i=0;i<NUMAARS;i++)
	{
		if(!aaRectangles[i].used)
		{
			aaRectangles[i].used=true;
			return &aaRectangles[i];
		}
	}
	return NULL;
} 

void startPI(void)
{
	fifoSendValue32(FIFO_USER_08,PI_START);
}

void pausePI(void)
{
	fifoSendValue32(FIFO_USER_08,PI_PAUSE);
}

void makeGrid(void)
{
	fifoSendValue32(FIFO_USER_08,PI_MAKEGRID);
}

void resetPI(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)objects[i].used=false;
	fifoSendValue32(FIFO_USER_08,PI_RESET);
}

OBB_struct* createBox(vect3D pos, int32 mass, md2Model_struct* model) //(id;[sizex|sizey][sizez|mass][posx][posy][posz])
{
	OBB_struct* o=newBox();
	if(!o)return NULL;
	
	initModelInstance(&o->modelInstance,model);
	
	o->size=vectDivInt(vectDifference(model->frames[0].max,model->frames[0].min),64);
	// o->size=vect(192,192,192);
	NOGBA("%d %d %d",o->size.x,o->size.y,o->size.z);
	o->position=pos;
	o->mass=mass>>3; //reduce precision to improve range
	o->transformationMatrix[0]=inttof32(1);o->transformationMatrix[1]=0;o->transformationMatrix[2]=0;
	o->transformationMatrix[3]=0;o->transformationMatrix[4]=inttof32(1);o->transformationMatrix[5]=0;
	o->transformationMatrix[6]=0;o->transformationMatrix[7]=0;o->transformationMatrix[8]=inttof32(1);
	
	fifoSendValue32(FIFO_USER_08,PI_ADDBOX|((o->id)<<PISIGNALDATA));
	fifoSendValue32(FIFO_USER_08,(o->size.x&((1<<16)-1))|((o->size.y&((1<<16)-1))<<16));	
	fifoSendValue32(FIFO_USER_08,(o->size.z&((1<<16)-1))|((o->mass&((1<<16)-1))<<16));
	fifoSendValue32(FIFO_USER_08,(o->position.x));
	fifoSendValue32(FIFO_USER_08,(o->position.y));
	fifoSendValue32(FIFO_USER_08,(o->position.z));
	
	return o;
}

void createAAR(vect3D size, vect3D pos, vect3D normal) //(id;[sizex|sizey][sizez|normal][posx][posy][posz])
{
	AAR_struct* a=newAAR();
	if(!a)return;
	a->size=size;
	a->position=pos;
	
	u16 n=((normal.x<0))|((normal.x>0)<<1)
		 |((normal.y<0)<<2)|((normal.y>0)<<3)
		 |((normal.z<0)<<4)|((normal.z>0)<<5);
	
	fifoSendValue32(FIFO_USER_08,PI_ADDAAR|((a->id)<<PISIGNALDATA));
	fifoSendValue32(FIFO_USER_08,(a->size.x&((1<<16)-1))|((a->size.y&((1<<16)-1))<<16));	
	fifoSendValue32(FIFO_USER_08,(a->size.z&((1<<16)-1))|((n)<<16));
	fifoSendValue32(FIFO_USER_08,(a->position.x));
	fifoSendValue32(FIFO_USER_08,(a->position.y));
	fifoSendValue32(FIFO_USER_08,(a->position.z));
	
	NOGBA("aar : %d",a->id);
}

void applyForce(u8 id, vect3D pos, vect3D v) //(id;[posx|posy][posz][vx][vy][vz])
{	
	if(id>NUMOBJECTS || !objects[id].used)return;
	fifoSendValue32(FIFO_USER_08,PI_APPLYFORCE|((id)<<PISIGNALDATA));
	fifoSendValue32(FIFO_USER_08,(((s16)pos.x))|(((s16)pos.y)<<16));	
	fifoSendValue32(FIFO_USER_08,((s16)pos.z));
	fifoSendValue32(FIFO_USER_08,(v.x));
	fifoSendValue32(FIFO_USER_08,(v.y));
	fifoSendValue32(FIFO_USER_08,(v.z));
}

void setVelocity(u8 id, vect3D v) //(id;[vx][vy][vz])
{	
	if(id>NUMOBJECTS || !objects[id].used)return;
	fifoSendValue32(FIFO_USER_08,PI_SETVELOCITY|((id)<<PISIGNALDATA));
	fifoSendValue32(FIFO_USER_08,(v.x));
	fifoSendValue32(FIFO_USER_08,(v.y));
	fifoSendValue32(FIFO_USER_08,(v.z));
}

void updatePlayerPI(player_struct* p) //([vx][vy][vz])
{	
	if(!p)p=getPlayer();
	fifoSendValue32(FIFO_USER_08,PI_UPDATEPLAYER);
	fifoSendValue32(FIFO_USER_08,(p->object->position.x));
	fifoSendValue32(FIFO_USER_08,(p->object->position.y));
	fifoSendValue32(FIFO_USER_08,(p->object->position.z));
}

void updatePortalPI(u8 id, vect3D pos, vect3D normal, int32 angle) //(id;[px][py][pz][n][cos|sin])
{	
	u16 n=((normal.x<0))|((normal.x>0)<<1)
		 |((normal.y<0)<<2)|((normal.y>0)<<3)
		 |((normal.z<0)<<4)|((normal.z>0)<<5);
	
	fifoSendValue32(FIFO_USER_08,PI_UPDATEPORTAL|((id)<<PISIGNALDATA));
	fifoSendValue32(FIFO_USER_08,(pos.x*4));
	fifoSendValue32(FIFO_USER_08,(pos.y*4));
	fifoSendValue32(FIFO_USER_08,(pos.z*4));
	fifoSendValue32(FIFO_USER_08,(n));
	fifoSendValue32(FIFO_USER_08,((cosLerp(angle)+inttof32(1))|((sinLerp(angle)+inttof32(1))<<16)));
}

void listenPI9(void)
{
	while(fifoCheckValue32(FIFO_USER_01))
	{
		const int k=fifoGetValue32(FIFO_USER_01);
		while(!fifoCheckValue32(FIFO_USER_02));
		while(!fifoCheckValue32(FIFO_USER_03));
		while(!fifoCheckValue32(FIFO_USER_04));
		while(!fifoCheckValue32(FIFO_USER_05));
		while(!fifoCheckValue32(FIFO_USER_06));
		while(!fifoCheckValue32(FIFO_USER_07));
		if(k<NUMOBJECTS)
		{
			OBB_struct* o=&objects[k];
			// o->used=true;
			o->position.x=fifoGetValue32(FIFO_USER_02);
			o->position.y=fifoGetValue32(FIFO_USER_03);
			o->position.z=fifoGetValue32(FIFO_USER_04);
			
			int32 x=fifoGetValue32(FIFO_USER_05);
			o->transformationMatrix[0]=(s16)x-4096;
			o->transformationMatrix[3]=(s16)(x>>16)-4096;
			x=fifoGetValue32(FIFO_USER_06);
			o->transformationMatrix[6]=(s16)(x)-4096;				
			o->transformationMatrix[1]=(s16)(x>>16)-4096;
			x=fifoGetValue32(FIFO_USER_07);
			o->transformationMatrix[4]=(s16)x-4096;
			o->transformationMatrix[7]=(s16)(x>>16)-4096;
			
			const vect3D v1=vect(o->transformationMatrix[0],o->transformationMatrix[3],o->transformationMatrix[6]);
			const vect3D v2=vect(o->transformationMatrix[1],o->transformationMatrix[4],o->transformationMatrix[7]);
			const vect3D v=normalize(vectProduct(v1,v2));
			
			// NOGBA("0 : %d %d %d",v1.x,v1.y,v1.z);
			// NOGBA("1 : %d %d %d",v2.x,v2.y,v2.z);
			// NOGBA("2 : %d %d %d",v.x,v.y,v.z);
			// NOGBA("4 : %d %d %d",warpVector(&portal1,v1));
			
			o->transformationMatrix[2]=v.x;
			o->transformationMatrix[5]=v.y;
			o->transformationMatrix[8]=v.z;
		}
	}
	
	if(fifoCheckValue32(FIFO_USER_08))
	{
		NOGBA("fifo 8 : %d",fifoGetValue32(FIFO_USER_08));
	}
}

//DEBUG stuff

#define NUMOBBSEGMENTS (12)

const u8 OBBSegments[NUMOBBSEGMENTS][2]={{0,1},{1,2},{3,2},{0,3},
										 {5,4},{5,6},{6,7},{4,7},
										 {3,4},{0,5},{1,6},{2,7}};
										 
void getOBBVertices(OBB_struct* o, vect3D* v)
{
	if(!o || !v)return;
	int32 m2[9];
	// int32* m=o->transformationMatrix;
	// m2[0]=mulf32(m[0],o->size.x);m2[3]=mulf32(m[3],o->size.x);m2[6]=mulf32(m[6],o->size.x);
	// m2[1]=mulf32(m[1],o->size.y);m2[4]=mulf32(m[4],o->size.y);m2[7]=mulf32(m[7],o->size.y);
	// m2[2]=mulf32(m[2],o->size.z);m2[5]=mulf32(m[5],o->size.z);m2[8]=mulf32(m[8],o->size.z);
	int32* m=o->transformationMatrix;
	m2[0]=o->size.x;m2[3]=0;m2[6]=0;
	m2[1]=0;m2[4]=o->size.y;m2[7]=0;
	m2[2]=0;m2[5]=0;m2[8]=o->size.z;

	v[0]=vect(-m2[0]-m2[1]-m2[2],-m2[3]-m2[4]-m2[5],-m2[6]-m2[7]-m2[8]);
	v[1]=vect(m2[0]-m2[1]-m2[2],m2[3]-m2[4]-m2[5],m2[6]-m2[7]-m2[8]);
	v[2]=vect(m2[0]-m2[1]+m2[2],m2[3]-m2[4]+m2[5],m2[6]-m2[7]+m2[8]);
	v[3]=vect(-m2[0]-m2[1]+m2[2],-m2[3]-m2[4]+m2[5],-m2[6]-m2[7]+m2[8]);
	
	v[4]=vect(-v[1].x,-v[1].y,-v[1].z);
	v[5]=vect(-v[2].x,-v[2].y,-v[2].z);
	v[6]=vect(-v[3].x,-v[3].y,-v[3].z);
	v[7]=vect(-v[0].x,-v[0].y,-v[0].z);

	// int i;
	// for(i=0;i<8;i++)
	// {
		// v[i]=addVect(v[i],o->position);
	// }
}

void multTMatrix(int32* m)
{
	if(!m)return;
	
	MATRIX_MULT3x3=m[0];
	MATRIX_MULT3x3=m[3];
	MATRIX_MULT3x3=m[6];
	
	MATRIX_MULT3x3=m[1];
	MATRIX_MULT3x3=m[4];
	MATRIX_MULT3x3=m[7];
	
	MATRIX_MULT3x3=m[2];
	MATRIX_MULT3x3=m[5];
	MATRIX_MULT3x3=m[8];
}

void drawOBB(OBB_struct* o)
{
	if(!o)return;
	if(!o->used)return;
	vect3D v[8];
	getOBBVertices(o,v);
	
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glPushMatrix();
	
	glTranslatef32(o->position.x/4,o->position.y/4,o->position.z/4);
	multTMatrix(o->transformationMatrix);

	if(selectID==o->id)GFX_COLOR=RGB15(31,0,0);
	else GFX_COLOR=RGB15(31,31,31);
	
	GFX_COLOR=RGB15(31,31,31);
	
	renderModelFrameInterp(o->modelInstance.currentFrame, o->modelInstance.nextFrame, o->modelInstance.interpCounter, o->modelInstance.model, POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_FORMAT_LIGHT0, true);
	
	// int i;
	// glScalef32(inttof32(1)/4,inttof32(1)/4,inttof32(1)/4);
	// glBegin(GL_QUADS);
		// for(i=0;i<NUMOBBFACES;i++)
		// {
			// GFX_TEX_COORD=TEXTURE_PACK(0, 0);
			// glVertex3v16((v[OBBFaces[i][0]].x),(v[OBBFaces[i][0]].y),(v[OBBFaces[i][0]].z));
			// GFX_TEX_COORD=TEXTURE_PACK(inttot16(64), 0);
			// glVertex3v16((v[OBBFaces[i][1]].x),(v[OBBFaces[i][1]].y),(v[OBBFaces[i][1]].z));
			// GFX_TEX_COORD=TEXTURE_PACK(inttot16(64), inttot16(64));
			// glVertex3v16((v[OBBFaces[i][2]].x),(v[OBBFaces[i][2]].y),(v[OBBFaces[i][2]].z));
			// GFX_TEX_COORD=TEXTURE_PACK(0, inttot16(64));
			// glVertex3v16((v[OBBFaces[i][3]].x),(v[OBBFaces[i][3]].y),(v[OBBFaces[i][3]].z));
		// }

	glPopMatrix(1);
}

void copyOBB(OBB_struct* o, OBB_struct* o2)
{
	if(!o || !o2)return;
	o2->position=o->position;
	o2->size=o->size;
	o2->mass=o->mass;
	o2->used=o->used;
	o2->modelInstance=o->modelInstance;
	o2->id=o->id;
	memcpy(o2->transformationMatrix,o->transformationMatrix,sizeof(int32)*9);
}

void drawWarpedOBB(portal_struct* p, OBB_struct* o) //warped md2 : no interpolation ? MORE : display list keyframes ?
{
	OBB_struct o2;
	copyOBB(o,&o2);
	warpMatrix(p, o2.transformationMatrix);
	o2.position=addVect(vectMultInt(p->targetPortal->position,4),warpVector(p,vectDifference(o2.position,vectMultInt(p->position,4))));
	drawOBB(&o2);
}

void drawOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			drawOBB(&objects[i]);
			drawWarpedOBB(&portal1,&objects[i]);
			drawWarpedOBB(&portal2,&objects[i]);
		}
	}
}

void drawAAR(AAR_struct* a)
{
	if(!a)return;
	if(!a->used)return;
	
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	
	glPushMatrix();

	GFX_COLOR=RGB15(0,0,31);
	
	if(!a->size.x)
	{
		glBegin(GL_QUADS);
			glVertex3v16((a->position.x),(a->position.y),(a->position.z));
			GFX_COLOR=RGB15(31,0,31);
			glVertex3v16((a->position.x),(a->position.y+a->size.y),(a->position.z));
			GFX_COLOR=RGB15(31,0,0);
			glVertex3v16((a->position.x),(a->position.y+a->size.y),(a->position.z+a->size.z));
			GFX_COLOR=RGB15(31,31,0);
			glVertex3v16((a->position.x),(a->position.y),(a->position.z+a->size.z));
		glEnd();
	}else if(!a->size.y)
	{
		glBegin(GL_QUADS);
			glVertex3v16((a->position.x),(a->position.y),(a->position.z));
			GFX_COLOR=RGB15(31,0,31);
			glVertex3v16((a->position.x+a->size.x),(a->position.y),(a->position.z));
			GFX_COLOR=RGB15(31,0,0);
			glVertex3v16((a->position.x+a->size.x),(a->position.y),(a->position.z+a->size.z));
			GFX_COLOR=RGB15(31,31,0);
			glVertex3v16((a->position.x),(a->position.y),(a->position.z+a->size.z));
		glEnd();
	}else{
		glBegin(GL_QUADS);
			glVertex3v16((a->position.x),(a->position.y),(a->position.z));
			GFX_COLOR=RGB15(31,0,31);
			glVertex3v16((a->position.x+a->size.x),(a->position.y),(a->position.z));
			GFX_COLOR=RGB15(31,0,0);
			glVertex3v16((a->position.x+a->size.x),(a->position.y+a->size.y),(a->position.z));
			GFX_COLOR=RGB15(31,31,0);
			glVertex3v16((a->position.x),(a->position.y+a->size.y),(a->position.z));
		glEnd();
	}

	glPopMatrix(1);
}

void drawAARs(void)
{
	int i;
	for(i=0;i<NUMAARS;i++)
	{
		if(aaRectangles[i].used)
		{
			drawAAR(&aaRectangles[i]);
		}
	}
}
