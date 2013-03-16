#include "game/game_main.h"

#define mcoord(i,j) ((i)+(j)*4)

camera_struct playerCamera;

//repurposed from libnds
void initProjectionMatrix(camera_struct* c, int fovy, int32 aspect, int32 near, int32 far)
{
	if(!c)c=&playerCamera;
	int32* m=c->projectionMatrix;
	
	int32 right, left, top, bottom;

	top = mulf32(near, tanLerp(fovy>>1));

	bottom = -top;
	left = mulf32(bottom, aspect);
	right = mulf32(top, aspect);
	
	*(m++) = divf32(2*near, right - left);
	*(m++) = 0;
	*(m++) = 0;
	*(m++) = 0;

	*(m++) = 0;
	*(m++) = divf32(2*near, top - bottom);
	*(m++) = 0;
	*(m++) = 0;

	*(m++) = divf32(right + left, right - left);
	*(m++) = divf32(top + bottom, top - bottom);
	*(m++) = -divf32(far + near, far - near);
	*(m++) = inttof32(-1);

	*(m++) = 0;
	*(m++) = 0;
	*(m++) = -divf32(2 * mulf32(far, near), far - near);
	*(m++) = 0;

	// NOGBA("PROJECTION");
	// NOGBA("%d %d %d %d",c->projectionMatrix[0],c->projectionMatrix[1],c->projectionMatrix[2],c->projectionMatrix[3]);
	// NOGBA("%d %d %d %d",c->projectionMatrix[4],c->projectionMatrix[5],c->projectionMatrix[6],c->projectionMatrix[7]);
	// NOGBA("%d %d %d %d",c->projectionMatrix[8],c->projectionMatrix[9],c->projectionMatrix[10],c->projectionMatrix[11]);
	// NOGBA("%d %d %d %d",c->projectionMatrix[12],c->projectionMatrix[13],c->projectionMatrix[14],c->projectionMatrix[15]);

	frustum_struct* f=&c->frustum;

	f->fovy=fovy;f->aspect=aspect;
	f->near=near;f->far=far;
	f->nTop=top;f->nLeft=left;
	f->fTop=mulf32(inttof32(1), tanLerp(fovy>>1));f->fLeft=-mulf32(f->fTop, aspect); //not real far top
}

void initProjectionMatrixBottom(camera_struct* c, int fovy, int32 aspect, int32 near, int32 far)
{
	if(!c)c=&playerCamera;
	int32* m=c->projectionMatrix;
	
	int32 right, left, top, bottom;

	top = mulf32(near, tanLerp(fovy>>1));
	bottom=-top;

	left = mulf32(bottom, aspect);
	right = mulf32(top, aspect);

	bottom=-top*3;
	top=-top;
	
	*(m++) = divf32(2*near, right - left);
	*(m++) = 0;
	*(m++) = 0;
	*(m++) = 0;

	*(m++) = 0;
	*(m++) = divf32(2*near, top - bottom);
	*(m++) = 0;
	*(m++) = 0;

	*(m++) = divf32(right + left, right - left);
	*(m++) = divf32(top + bottom, top - bottom);
	*(m++) = -divf32(far + near, far - near);
	*(m++) = inttof32(-1);

	*(m++) = 0;
	*(m++) = 0;
	*(m++) = -divf32(2 * mulf32(far, near), far - near);
	*(m++) = 0;

	frustum_struct* f=&c->frustum;

	f->fovy=fovy;f->aspect=aspect;
	f->near=near;f->far=far;
	f->nTop=top;f->nLeft=left;
	f->fTop=mulf32(inttof32(1), tanLerp(fovy>>1));f->fLeft=-mulf32(f->fTop, aspect); //not real far top
}

//this too
void initProjectionMatrixOrtho(camera_struct* c, int left, int right, int bottom, int top, int zNear, int zFar)
{
	if(!c)c=&playerCamera;
	int32* m=c->projectionMatrix;
		
	*(m++) = divf32(inttof32(2), right - left);
	*(m++) = 0;
	*(m++) = 0;
	*(m++) = 0;

	*(m++) = 0;
	*(m++) = divf32(inttof32(2), top - bottom);
	*(m++) = 0;
	*(m++) = 0;

	*(m++) = 0;
	*(m++) = 0;
	*(m++) = divf32(inttof32(-2), zFar - zNear);
	*(m++) = 0;

	*(m++) = -divf32(right + left, right - left);
	*(m++) = -divf32(top + bottom, top - bottom);
	*(m++) = -divf32(zFar + zNear, zFar - zNear);
	*(m++) = inttof32(1);
}

void getUnprojectedZLine(camera_struct* c, s16 x, s16 y, vect3D* o, vect3D* v)
{
	if(!c)c=&playerCamera;
	if(!v || !o)return;
	frustum_struct* f=&c->frustum;

	*o=addVect(vect(0,0,f->near),vect((x*f->nLeft)/128,(y*f->nTop)/96, 0));
	*v=addVect(vect(0,0,inttof32(1)),vect((x*f->fLeft)/128,(y*f->fTop)/96, 0));
	*v=vectDifference(*v,*o);
}

void initTransformationMatrix(camera_struct* c)
{
	if(!c)c=&playerCamera;
	int32* m=c->transformationMatrix;
	
	*(m++)=inttof32(1);
	*(m++)=0;
	*(m++)=0;
	
	*(m++)=0;
	*(m++)=inttof32(1);
	*(m++)=0;
	
	*(m++)=0;
	*(m++)=0;
	*(m)=inttof32(1);
}

void updateViewMatrix(camera_struct* c)
{
	int i, j;
	if(!c)c=&playerCamera;
	int32* m1=c->projectionMatrix;
	int32 m2[4*4];
	const int fact=f32toint(SCALEFACT);
	
	for(i=0;i<3;i++)for(j=0;j<3;j++)m2[i+j*4]=c->transformationMatrix[i+j*3]*fact;
	
	m2[3+0*4]=0;
	m2[3+1*4]=0;
	m2[3+2*4]=0;
	
	// m2[0+3*4]=-c->position.x*fact;
	// m2[1+3*4]=-c->position.y*fact;
	// m2[2+3*4]=-c->position.z*fact;
	m2[0+3*4]=0;
	m2[1+3*4]=0;
	m2[2+3*4]=0;
	m2[3+3*4]=inttof32(1);
	translateMatrix(m2,-c->viewPosition.x,-c->viewPosition.y,-c->viewPosition.z);
	
	multMatrix44(m2,m1,c->viewMatrix);
}

void multMatrixGfx33(int32* n)
{
	if(!n)return;
	int32* m=n;
	MATRIX_MULT3x3=*(m++);
	MATRIX_MULT3x3=*(m++);
	MATRIX_MULT3x3=*(m++);
	
	MATRIX_MULT3x3=*(m++);
	MATRIX_MULT3x3=*(m++);
	MATRIX_MULT3x3=*(m++);
	
	MATRIX_MULT3x3=*(m++);
	MATRIX_MULT3x3=*(m++);
	MATRIX_MULT3x3=*m;
}

void multTransformationMatrix(camera_struct* c)
{
	if(!c)c=&playerCamera;
	multMatrixGfx33(c->transformationMatrix);
}

void multProjectionMatrix(camera_struct* c)
{
	if(!c)c=&playerCamera;
	int32* m=c->projectionMatrix;
	
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*(m++);
	MATRIX_MULT4x4=*m;
}

static inline void projectVectorPlane(vect3D* v, vect3D n)
{
	if(!v)return;
	int32 r=dotProduct(*v,n);
	*v=vectDifference(*v,vectMult(n,r));
}

void fixMatrix(int32* m) //3x3
{
	if(!m)return;
	vect3D x=vect(m[0],m[3],m[6]);
	vect3D y=vect(m[1],m[4],m[7]);
	vect3D z=vect(m[2],m[5],m[8]);
	
	projectVectorPlane(&x,y);
	projectVectorPlane(&z,y);
	projectVectorPlane(&z,x);
	
	x=normalize(x);
	y=normalize(y);
	z=normalize(z);
	
	m[0]=x.x;m[3]=x.y;m[6]=x.z;
	m[1]=y.x;m[4]=y.y;m[7]=y.z;
	m[2]=z.x;m[5]=z.y;m[8]=z.z;
}

void multMatrix33(int32* m1, int32* m2, int32* m) //3x3
{
	int i, j;
	// for(i=0;i<4;i++)for(j=0;j<4;j++)m[i+j*4]=m1[i+0*4]*m2[0+j*4]+m1[i+1*4]*m2[1+j*4]+m1[i+2*4]*m2[2+j*4]+m1[i+3*4]*m2[3+j*4];
	for(i=0;i<3;i++)for(j=0;j<3;j++)m[j+i*3]=mulf32(m1[0+i*3],m2[j+0*3])+mulf32(m1[1+i*3],m2[j+1*3])+mulf32(m1[2+i*3],m2[j+2*3]);
}

void multMatrix44(int32* m1, int32* m2, int32* m) //4x4
{
	int i, j;
	// for(i=0;i<4;i++)for(j=0;j<4;j++)m[i+j*4]=mulf32(m1[0+j*4],m2[i+0*4])+mulf32(m1[1+j*4],m2[i+1*4])+mulf32(m1[2+j*4],m2[i+2*4])+mulf32(m1[3+j*4],m2[i+3*4]);
	for(i=0;i<4;i++)for(j=0;j<4;j++)m[i+j*4]=mulf32(m1[0+j*4],m2[i+0*4])+mulf32(m1[1+j*4],m2[i+1*4])+mulf32(m1[2+j*4],m2[i+2*4])+mulf32(m1[3+j*4],m2[i+3*4]);
}

void changeBase(int32* tm, vect3D x, vect3D y, vect3D z, bool r)
{
	int32 m[9], bcm[9];
	
	bcm[0]=x.x;bcm[1]=y.x;bcm[2]=z.x;
	bcm[3]=x.y;bcm[4]=y.y;bcm[5]=z.y;
	bcm[6]=x.z;bcm[7]=y.z;bcm[8]=z.z;
	
	// NOGBA("MATRIX");
	// NOGBA("%d %d %d",bcm[0],bcm[1],bcm[2]);
	// NOGBA("%d %d %d",bcm[0+3],bcm[1+3],bcm[2+3]);
	// NOGBA("%d %d %d",bcm[0+3+3],bcm[1+3+3],bcm[2+3+3]);
	
	if(r)multMatrix33(tm,bcm,m);
	else multMatrix33(bcm,tm,m);
	memcpy(tm,m,9*sizeof(int32));
}

void rotateMatrixX(int32* tm, int32 x, bool r)
{
	int i;
	int32 rm[9], m[9];
	for(i=0;i<9;i++)rm[i]=0;
	rm[0]=inttof32(1);
	rm[4]=cosLerp(x);
	rm[5]=sinLerp(x);
	rm[7]=-sinLerp(x);
	rm[8]=cosLerp(x);
	if(r)multMatrix33(rm,tm,m);
	else multMatrix33(tm,rm,m);
	memcpy(tm,m,9*sizeof(int32));
}

void rotateMatrixY(int32* tm, int32 x, bool r)
{
	int i;
	int32 rm[9], m[16];
	for(i=0;i<9;i++)rm[i]=0;
	rm[0]=cosLerp(x);
	rm[2]=sinLerp(x);
	rm[4]=inttof32(1);
	rm[6]=-sinLerp(x);
	rm[8]=cosLerp(x);
	if(r)multMatrix33(rm,tm,m);
	else multMatrix33(tm,rm,m);
	memcpy(tm,m,9*sizeof(int32));
}

void rotateMatrixZ(int32* tm, int32 x, bool r)
{
	int i;
	int32 rm[9], m[16];
	for(i=0;i<9;i++)rm[i]=0;
	rm[0]=cosLerp(x);
	rm[1]=sinLerp(x);
	rm[3]=-sinLerp(x);
	rm[4]=cosLerp(x);
	rm[8]=inttof32(1);
	if(r)multMatrix33(rm,tm,m);
	else multMatrix33(tm,rm,m);
	memcpy(tm,m,9*sizeof(int32));
}

static inline void rotateMatrixAxis(int32* tm, int32 x, vect3D a, bool r)
{
	int32 rm[9], m[9];

	int32 cosval=cosLerp(x);
	int32 sinval=sinLerp(x);
	int32 onemcosval=inttof32(1)-cosval;

	rm[0]=cosval+mulf32(mulf32(a.x,a.x),onemcosval);
	rm[1]=mulf32(mulf32(a.x,a.y),onemcosval)-mulf32(a.z,sinval);
	rm[2]=mulf32(mulf32(a.x,a.z),onemcosval)+mulf32(a.y,sinval);

	rm[3]=mulf32(mulf32(a.x,a.y),onemcosval)+mulf32(a.z,sinval);
	rm[4]=cosval+mulf32(mulf32(a.y,a.y),onemcosval);
	rm[5]=mulf32(mulf32(a.y,a.z),onemcosval)-mulf32(a.x,sinval);

	rm[6]=mulf32(mulf32(a.x,a.z),onemcosval)-mulf32(a.y,sinval);
	rm[7]=mulf32(mulf32(a.y,a.z),onemcosval)+mulf32(a.x,sinval);
	rm[8]=cosval+mulf32(mulf32(a.z,a.z),onemcosval);

	if(r)multMatrix33(rm,tm,m);
	else multMatrix33(tm,rm,m);
	memcpy(tm,m,9*sizeof(int32));
}

void translateMatrix(int32* tm, int32 x, int32 y, int32 z)
{
	int i;
	int32 rm[16], m[16];
	for(i=0;i<16;i++)rm[i]=0;
	rm[0]=inttof32(1);
	rm[1+4*1]=inttof32(1);
	rm[2+4*2]=inttof32(1);
	rm[3+4*3]=inttof32(1);
	
	rm[0+3*4]=x;
	rm[1+3*4]=y;
	rm[2+3*4]=z;
	
	multMatrix44(rm,tm,m);
	memcpy(tm,m,16*sizeof(int32));
}

void initCamera(camera_struct* c)
{
	if(!c)c=&playerCamera;
	
	c->position=vect(0,HEIGHTUNIT*STARTHEIGHT,0);
	c->viewPosition=c->position;
	c->object.position=c->position;
	c->object.speed=vect(0,0,0);
	c->lookAt=false;
	
	initTransformationMatrix(c);
	initProjectionMatrix(c, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));
}

vect3D getUnitVector(camera_struct* c)
{
	if(!c)c=&playerCamera;
	vect3D v;
	// v.x=-mulf32(sinLerp(c->angle.y),cosLerp(c->angle.x));
	// v.y=sinLerp(c->angle.x);
	// v.z=-mulf32(cosLerp(c->angle.y),cosLerp(c->angle.x));
	v.x=-c->transformationMatrix[2];
	v.y=-c->transformationMatrix[5];
	v.z=-c->transformationMatrix[8];
	return v;
}

void projectCamera(camera_struct* c)
{
	if(!c)c=&playerCamera;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// gluPerspective(70, 256.0 / 192.0, 5.0, 1000);
	// gluPerspective(70, 256.0 / 192.0, 1.0, 500);
	// gluPerspectivef32(70*90, inttof32(4)/3, inttof32(1), inttof32(500));
	// gluPerspective(70, 256.0 / 192.0, 0.1, 600);
	multProjectionMatrix(c);
	
	glMatrixMode(GL_MODELVIEW);
}

void transformCamera(camera_struct* c)
{
	if(!c)c=&playerCamera;
	
	multTransformationMatrix(c);


	NOGBA("%d %d %d and %d %d %d",c->position.x,c->position.y,c->position.z,c->viewPosition.x,c->viewPosition.y,c->viewPosition.z);
	glTranslatef32(-c->viewPosition.x,-c->viewPosition.y,-c->viewPosition.z);
}

void untransformCamera(camera_struct* c)
{
	if(!c)c=&playerCamera;

	int32 m[9];
	transposeMatrix33(c->transformationMatrix,m);

	multMatrixGfx33(m);
}

vect3D getCameraPosition(camera_struct* c)
{
	if(!c)c=&playerCamera;
	return c->position;
}

void setCamera(camera_struct* c, vect3D v)
{
	if(!c)c=&playerCamera;
	c->position=v;
}

void moveCamera(camera_struct* c, vect3D v)
{
	if(!c)c=&playerCamera;
	
	vect3D v1=vect(c->transformationMatrix[2],c->transformationMatrix[5],c->transformationMatrix[8]);
	v1=normalize(vectDifference(v1,vectMult(normGravityVector,dotProduct(normGravityVector,v1))));
	
	vect3D u=vect(mulf32(v.z,v1.x)+mulf32(v.x,c->transformationMatrix[0]), mulf32(v.z,v1.y)+mulf32(v.x,c->transformationMatrix[3]), mulf32(v.z,v1.z)+mulf32(v.x,c->transformationMatrix[6]));

	c->object.speed=addVect(c->object.speed,u);
}

void moveCameraImmediate(camera_struct* c, vect3D v)
{
	if(!c)c=&playerCamera;
	
	vect3D v1=normalize(vect(c->transformationMatrix[2],c->transformationMatrix[5],c->transformationMatrix[8]));
	
	c->position.x+=mulf32(v.z,v1.x)+mulf32(v.x,c->transformationMatrix[0])+mulf32(v.y,c->transformationMatrix[1]);
	c->position.y+=mulf32(v.z,v1.y)+mulf32(v.x,c->transformationMatrix[3])+mulf32(v.y,c->transformationMatrix[4]);
	c->position.z+=mulf32(v.z,v1.z)+mulf32(v.x,c->transformationMatrix[6])+mulf32(v.y,c->transformationMatrix[7]);
}

void rotateCamera(camera_struct* c, vect3D a)
{
	if(!c)c=&playerCamera;
	
	// rotateMatrixX(c->transformationMatrix,-a.x,false);
	// rotateMatrixY(c->transformationMatrix,a.y,true);
	// rotateMatrixZ(c->transformationMatrix,a.z,false);

	// rotateMatrixAxis(c->transformationMatrix,a.x,vect(c->transformationMatrix[0],c->transformationMatrix[1],c->transformationMatrix[2]),false);
	rotateMatrixX(c->transformationMatrix,-a.x,false);
	rotateMatrixAxis(c->transformationMatrix,-a.y,normGravityVector,true);
}

bool pointInFrustum(frustum_struct* f, vect3D v)
{
	if(!f)f=&playerCamera.frustum;
	return ((evaluatePlanePoint(&f->plane[0],v)>0)&&(evaluatePlanePoint(&f->plane[1],v)>0)&&(evaluatePlanePoint(&f->plane[2],v)>0)
		  &&(evaluatePlanePoint(&f->plane[3],v)>0)&&(evaluatePlanePoint(&f->plane[4],v)>0)&&(evaluatePlanePoint(&f->plane[5],v)>0));
}

void updateFrustum(camera_struct* c)
{
	if(!c)c=&playerCamera;
	frustum_struct* f=&c->frustum;

	//near
	f->plane[0].A=c->viewMatrix[mcoord(2,0)]+c->viewMatrix[mcoord(3,0)];
	f->plane[0].B=c->viewMatrix[mcoord(2,1)]+c->viewMatrix[mcoord(3,1)];
	f->plane[0].C=c->viewMatrix[mcoord(2,2)]+c->viewMatrix[mcoord(3,2)];
	f->plane[0].D=c->viewMatrix[mcoord(2,3)]+c->viewMatrix[mcoord(3,3)];
	// f->plane[0].D=0;

	//far
	f->plane[1].A=-c->viewMatrix[mcoord(2,0)]+c->viewMatrix[mcoord(3,0)];
	f->plane[1].B=-c->viewMatrix[mcoord(2,1)]+c->viewMatrix[mcoord(3,1)];
	f->plane[1].C=-c->viewMatrix[mcoord(2,2)]+c->viewMatrix[mcoord(3,2)];
	f->plane[1].D=-c->viewMatrix[mcoord(2,3)]+c->viewMatrix[mcoord(3,3)];

	//left
	f->plane[2].A=c->viewMatrix[mcoord(0,0)]+c->viewMatrix[mcoord(3,0)];
	f->plane[2].B=c->viewMatrix[mcoord(0,1)]+c->viewMatrix[mcoord(3,1)];
	f->plane[2].C=c->viewMatrix[mcoord(0,2)]+c->viewMatrix[mcoord(3,2)];
	f->plane[2].D=c->viewMatrix[mcoord(0,3)]+c->viewMatrix[mcoord(3,3)];

	//right
	f->plane[3].A=-c->viewMatrix[mcoord(0,0)]+c->viewMatrix[mcoord(3,0)];
	f->plane[3].B=-c->viewMatrix[mcoord(0,1)]+c->viewMatrix[mcoord(3,1)];
	f->plane[3].C=-c->viewMatrix[mcoord(0,2)]+c->viewMatrix[mcoord(3,2)];
	f->plane[3].D=-c->viewMatrix[mcoord(0,3)]+c->viewMatrix[mcoord(3,3)];

	//bottom
	f->plane[4].A=c->viewMatrix[mcoord(1,0)]+c->viewMatrix[mcoord(3,0)];
	f->plane[4].B=c->viewMatrix[mcoord(1,1)]+c->viewMatrix[mcoord(3,1)];
	f->plane[4].C=c->viewMatrix[mcoord(1,2)]+c->viewMatrix[mcoord(3,2)];
	f->plane[4].D=c->viewMatrix[mcoord(1,3)]+c->viewMatrix[mcoord(3,3)];

	//top
	f->plane[5].A=-c->viewMatrix[mcoord(1,0)]+c->viewMatrix[mcoord(3,0)];
	f->plane[5].B=-c->viewMatrix[mcoord(1,1)]+c->viewMatrix[mcoord(3,1)];
	f->plane[5].C=-c->viewMatrix[mcoord(1,2)]+c->viewMatrix[mcoord(3,2)];
	f->plane[5].D=-c->viewMatrix[mcoord(1,3)]+c->viewMatrix[mcoord(3,3)];

	int i=0;
	for(i=0;i<6;i++)
	{
		int32 r=sqrtf32(mulf32(f->plane[i].A,f->plane[i].A)+mulf32(f->plane[i].B,f->plane[i].B)+mulf32(f->plane[i].C,f->plane[i].C));
		f->plane[i].A=divf32(f->plane[i].A,r);
		f->plane[i].B=divf32(f->plane[i].B,r);
		f->plane[i].C=divf32(f->plane[i].C,r);
		f->plane[i].D=divf32(f->plane[i].D,r);
		f->plane[i].point.x=-mulf32(f->plane[i].D,f->plane[i].A);
		f->plane[i].point.y=-mulf32(f->plane[i].D,f->plane[i].B);
		f->plane[i].point.z=-mulf32(f->plane[i].D,f->plane[i].C);
	}
	//correct and prevents some precision problems
	f->plane[2].point=c->viewPosition;
	f->plane[3].point=c->viewPosition;
	f->plane[4].point=c->viewPosition;
	f->plane[5].point=c->viewPosition;
}

vect3D getViewPosition(vect3D p)
{
	return vectDifference(p,vectDivInt(normGravityVector,32));
}

vect3D reverseViewPosition(vect3D p)
{
	return vectDifference(p,vectDivInt(normGravityVector,-32));
}

void updateCamera(camera_struct* c)
{
	if(!c)c=&playerCamera;

	c->position=c->object.position;
	c->viewPosition=getViewPosition(c->position);
	updateViewMatrix(c);
	updateFrustum(c);
	
	int32 alignment=-dotProduct(vect(c->transformationMatrix[0],c->transformationMatrix[3],c->transformationMatrix[6]),normGravityVector);
	iprintf("alignment : %d  \n",alignment);

	{
		if(alignment>32)
		{
			if(alignment>512)rotateMatrixZ(c->transformationMatrix, (1<<10), false);
			else if(alignment>256)rotateMatrixZ(c->transformationMatrix, (1<<9), false);
			else if(alignment>128)rotateMatrixZ(c->transformationMatrix, (1<<8), false);
		}else if(alignment<-32)
		{
			if(alignment<-512)rotateMatrixZ(c->transformationMatrix, -(1<<10), false);
			else if(alignment<-256)rotateMatrixZ(c->transformationMatrix, -(1<<9), false);
			else if(alignment<-128)rotateMatrixZ(c->transformationMatrix, -(1<<8), false);
		}
	}
	
	fixMatrix(c->transformationMatrix); //compensate fixed point errors
}

camera_struct* getPlayerCamera(void){return &playerCamera;}
