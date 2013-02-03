#include "stdafx.h"

#define TIMEPREC (6)

OBB_struct objects[NUMOBJECTS];

u32 coll, integ, impul;

/*s16 divLUT[4097];

void initDivision(void)
{
	int i;
	for(i=1;i<4097;i++)
	{
		divLUT[i]=divv16(4096,i);
	}
	divLUT[0]=0;
}*/

static inline int32 divv(int32 a, int32 b) // b in 1-4096
{
	/*return divf32(a,b);
	s32 r=(((s32)a)*((s32)divLUT[b]))>>12;
	return (s16)r;*/
	return divv16(a,b);
}

void initOBB(OBB_struct* o, vect3D size, vect3D pos, int32 mass)
{
	if(!o)return;
	
	o->used=true;
	o->position=pos;
	o->angularMomentum=vect(0,0,0);
	o->numContactPoints=0;
	o->size=size;
	o->mass=mass;
	o->maxPenetration=0;
	
	o->energy=0;
	o->sleep=false;
	
	o->velocity=vect(0,0,0);
	o->angularVelocity=vect(0,0,0);
	o->moment=vect(0,0,0);
	o->forces=vect(0,0,0);

	initTransformationMatrix(o->transformationMatrix);

	int32 x2=mulf32(o->size.x,o->size.x);
	int32 y2=mulf32(o->size.y,o->size.y);
	int32 z2=mulf32(o->size.z,o->size.z);
	
	int i;for(i=0;i<9;i++){o->invInertiaMatrix[i]=0;}
    //o->invInertiaMatrix[0]=divf32(inttof32(3),(mulf32(o->mass,(y2+z2))));
    //o->invInertiaMatrix[4]=divf32(inttof32(3),(mulf32(o->mass,(x2+z2))));
    //o->invInertiaMatrix[8]=divf32(inttof32(3),(mulf32(o->mass,(x2+y2))));
    o->invInertiaMatrix[0]=divv16(inttof32(3),(mulf32(o->mass,(y2+z2))));
    o->invInertiaMatrix[4]=divv16(inttof32(3),(mulf32(o->mass,(x2+z2))));
    o->invInertiaMatrix[8]=divv16(inttof32(3),(mulf32(o->mass,(x2+y2))));
	
	//temporary
	o->contactPoints=contactPoints;
	
	//rotateMatrixX(o->transformationMatrix,4096,false);
	//rotateMatrixZ(o->transformationMatrix,4096,false);
	
	if(portal[0].used&&portal[1].used)
	{
		updateOBBPortals(o,0,true);
		updateOBBPortals(o,1,true);
	}
}

void initOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		objects[i].used=false;
	}
}

void copyOBB(OBB_struct* o1, OBB_struct* o2)
{
	if(!o1 || !o2)return;
	
	o2->angularMomentum=o1->angularMomentum;
	o2->numContactPoints=o1->numContactPoints;
	o2->mass=o1->mass;
	o2->position=o1->position;
	o2->size=o1->size;
	o2->maxPenetration=o1->maxPenetration;
	
	o2->velocity=o1->velocity;
	o2->angularVelocity=o1->angularVelocity;
	o2->forces=o1->forces;
	o2->moment=o1->moment;
	
	o2->energy=o1->energy;
	o2->sleep=o1->sleep;
	
	//temporary
	o2->contactPoints=contactPoints;
	
	memcpy(o2->transformationMatrix,o1->transformationMatrix,sizeof(int32)*9);
	memcpy(o2->invInertiaMatrix,o1->invInertiaMatrix,sizeof(int32)*9);
	memcpy(o2->invWInertiaMatrix,o1->invWInertiaMatrix,sizeof(int32)*9);
	
	memcpy(o2->contactPoints,o1->contactPoints,sizeof(contactPoint_struct)*o2->numContactPoints);
}

bool collideAABB(vect3D o1, vect3D s1, vect3D o2, vect3D s2)
{
	return !(o2.x>o1.x+s1.x || o2.y>o1.y+s1.y || o2.z>o1.z+s1.z
		  || o2.x+s2.x<o1.x || o2.y+s2.y<o1.y || o2.z+s2.z<=o1.z);
}

vect3D projectPointAABB(vect3D size, vect3D p, vect3D* n)
{
	if(!n)return vect(0,0,0);
	vect3D v=p;
	*n=vect(0,0,0);

	/*if(p.x<-size.x){v.x=-size.x;n->x=-1;}
	else if(p.x>size.x){v.x=size.x;n->x=1;}
	
	if(p.y<-size.y){v.y=-size.y;n->y=-1;}
	else if(p.y>size.y){v.y=size.y;n->y=1;}

	if(p.z<-size.z){v.z=-size.z;n->z=-1;}
	else if(p.z>size.z){v.z=size.z;n->z=1;}*/

	if(p.x<-size.x){v.x=-size.x;n->x=-1;}
	else if(p.x>size.x){v.x=size.x;n->x=1;}
	else if(p.y<-size.y){v.y=-size.y;n->y=-1;}
	else if(p.y>size.y){v.y=size.y;n->y=1;}
	else if(p.z<-size.z){v.z=-size.z;n->z=-1;}
	else if(p.z>size.z){v.z=size.z;n->z=1;}

	if(!n->x && !n->y && !n->z)
	{	
		int32 d1=abs(p.x+size.x);int32 d2=abs(p.x-size.x);
		int32 d3=abs(p.y+size.y);int32 d4=abs(p.y-size.y);
		int32 d5=abs(p.z+size.z);int32 d6=abs(p.z-size.z);
		int32 d=min(d1,d2);
		d=min(d,min(d3,d4));
		d=min(d,min(d5,d6));
		if(d==d1){v.x=-size.x;n->x=-1;}
		else if(d==d2){v.x=size.x;n->x=1;}
		else if(d==d3){v.y=-size.y;n->y=-1;}
		else if(d==d4){v.y=size.y;n->y=1;}
		else if(d==d5){v.z=-size.z;n->z=-1;}
		else {v.z=size.z;n->z=1;}
	}

	return v;
}

bool collideLineRectangle(vect3D ro, vect3D ru1, vect3D ru2, vect3D rn, int32 rs1, int32 rs2, vect3D o, vect3D v, int32 d, vect3D* ip)
{
	int32 p1=dotProduct(v,rn);
	if(abs(p1)>10) //margin of error
	{
		int32 p2=dotProduct(vectDifference(ro,o),rn);
		//int32 k=divf32(p2,p1);
		int32 k=divv16(p2,p1);
		if(k<0 || k>d){return false;}
		vect3D i=addVect(o,vectMult(v,k));
		if(ip)*ip=i; //real position at this point
		i=vectDifference(i,ro);
		i=vect(dotProduct(i,ru1),dotProduct(i,ru2),0);
		
		return i.x>=0 && i.x<=rs1 && i.y>=0 && i.y<=rs2;
	}
	return false;
}

bool clipSegmentOBB(int32* ss, vect3D *uu, vect3D* p1, vect3D* p2, vect3D vv, vect3D* uu1, vect3D* uu2, vect3D vv1, vect3D* n1, vect3D* n2, bool* b1, bool* b2, int32* k1, int32* k2)
{
	if(!p1 || !p2 || !uu1 || !uu2 || !n1 || !n2 || !b1 || !b2 || !k1 || !k2)return false;

	if(uu1->x<-ss[0])
	{
		if(uu2->x>-ss[0])
		{
			int32 k=divv(abs(uu1->x+ss[0]),abs(vv1.x));
				*uu1=addVect(*uu1,vectMult(vv1,k));
				*p1=addVect(*p1,vectMult(vv,k));
			*k1=max(*k1,k);
			*n1=vect(-uu[0].x,-uu[0].y,-uu[0].z);
			*b1=true;
		}else return false;
	}else{
		if(uu2->x<-ss[0])
		{
			int32 k=divv(abs(uu1->x+ss[0]),abs(vv1.x));
				*uu2=addVect(*uu1,vectMult(vv1,k));
				*p2=addVect(*p1,vectMult(vv,k));
			*k2=min(*k2,k);
			*n2=vect(-uu[0].x,-uu[0].y,-uu[0].z);
			*b2=true;
		}
	}
				
	if(uu1->x<ss[0])
	{
		if(uu2->x>ss[0])
		{
			int32 k=divv(abs(uu1->x-ss[0]),abs(vv1.x));
				*uu2=addVect(*uu1,vectMult(vv1,k));
				*p2=addVect(*p1,vectMult(vv,k));
			*k2=min(*k2,k);
			*n2=(uu[0]);
			*b2=true;
		}
	}else{
		if(uu2->x<ss[0])
		{
			int32 k=divv(abs(uu1->x-ss[0]),abs(vv1.x));
				*uu1=addVect(*uu1,vectMult(vv1,k));
				*p1=addVect(*p1,vectMult(vv,k));
			*k1=max(*k1,k);
			*n1=(uu[0]);
			*b1=true;
		}else return false;
	}

				
	if(uu1->y<-ss[1])
	{
		if(uu2->y>-ss[1])
		{
			int32 k=divv(abs(uu1->y+ss[1]),abs(vv1.y));
				*uu1=addVect(*uu1,vectMult(vv1,k));
				*p1=addVect(*p1,vectMult(vv,k));
			*k1=max(*k1,k);
			*n1=vect(-uu[1].x,-uu[1].y,-uu[1].z);
			*b1=true;
		}else return false;
	}else{
		if(uu2->y<-ss[1])
		{
			int32 k=divv(abs(uu1->y+ss[1]),abs(vv1.y));
				*uu2=addVect(*uu1,vectMult(vv1,k));
				*p2=addVect(*p1,vectMult(vv,k));
			*k2=min(*k2,k);
			*n2=vect(-uu[1].x,-uu[1].y,-uu[1].z);
			*b2=true;
		}
	}
				
	if(uu1->y<ss[1])
	{
		if(uu2->y>ss[1])
		{
			int32 k=divv(abs(uu1->y-ss[1]),abs(vv1.y));
				*uu2=addVect(*uu1,vectMult(vv1,k));
				*p2=addVect(*p1,vectMult(vv,k));
			*k2=min(*k2,k);
			*n2=(uu[1]);
			*b2=true;
		}
	}else{
		if(uu2->y<ss[1])
		{
			int32 k=divv(abs(uu1->y-ss[1]),abs(vv1.y));
				*uu1=addVect(*uu1,vectMult(vv1,k));
				*p1=addVect(*p1,vectMult(vv,k));
			*k1=max(*k1,k);
			*n1=(uu[1]);
			*b1=true;
		}else return false;
	}

				
	if(uu1->z<-ss[2])
	{
		if(uu2->z>-ss[2])
		{
			int32 k=divv(abs(uu1->z+ss[2]),abs(vv1.z));
				*uu1=addVect(*uu1,vectMult(vv1,k));
				*p1=addVect(*p1,vectMult(vv,k));
			*k1=max(*k1,k);
			*n1=vect(-uu[2].x,-uu[2].y,-uu[2].z);
			*b1=true;
		}else return false;
	}else{
		if(uu2->z<-ss[2])
		{
			int32 k=divv(abs(uu1->z+ss[2]),abs(vv1.z));
				*uu2=addVect(*uu1,vectMult(vv1,k));
				*p2=addVect(*p1,vectMult(vv,k));
			*k2=min(*k2,k);
			*n2=vect(-uu[2].x,-uu[2].y,-uu[2].z);
			*b2=true;
		}
	}
				
	if(uu1->z<ss[2])
	{
		if(uu2->z>ss[2])
		{
			int32 k=divv(abs(uu1->z-ss[2]),abs(vv1.z));
				*uu2=addVect(*uu1,vectMult(vv1,k));
				*p2=addVect(*p1,vectMult(vv,k));
			*k2=min(*k2,k);
			*n2=(uu[2]);
			*b2=true;
		}
	}else{
		if(uu2->z<ss[2])
		{
			int32 k=divv(abs(uu1->z-ss[2]),abs(vv1.z));
				*uu1=addVect(*uu1,vectMult(vv1,k));
				*p1=addVect(*p1,vectMult(vv,k));
			*k1=max(*k1,k);
			*n1=(uu[2]);
			*b1=true;
		}else return false;
	}
	return true;
}

void collideOBBs(OBB_struct* o1, OBB_struct* o2)
{
	if(!o1 || !o2)return;
	if(o1==o2)return;

	if(!collideAABB(vect(o1->AABBo.x-MAXPENETRATIONBOX,o1->AABBo.y-MAXPENETRATIONBOX,o1->AABBo.z-MAXPENETRATIONBOX),
					vect(o1->AABBs.x+2*MAXPENETRATIONBOX,o1->AABBs.y+2*MAXPENETRATIONBOX,o1->AABBs.z+2*MAXPENETRATIONBOX),
					vect(o2->AABBo.x-MAXPENETRATIONBOX,o2->AABBo.y-MAXPENETRATIONBOX,o2->AABBo.z-MAXPENETRATIONBOX),
					vect(o2->AABBs.x+2*MAXPENETRATIONBOX,o2->AABBs.y+2*MAXPENETRATIONBOX,o2->AABBs.z+2*MAXPENETRATIONBOX)))return;

	vect3D v[8],v2[8];
	getOBBVertices(o1,v);
	
	vect3D z1=vect(o1->transformationMatrix[0],o1->transformationMatrix[3],o1->transformationMatrix[6]);
	vect3D z2=vect(o1->transformationMatrix[1],o1->transformationMatrix[4],o1->transformationMatrix[7]);
	vect3D z3=vect(o1->transformationMatrix[2],o1->transformationMatrix[5],o1->transformationMatrix[8]);

	vect3D u1=vect(o2->transformationMatrix[0],o2->transformationMatrix[3],o2->transformationMatrix[6]);
	vect3D u2=vect(o2->transformationMatrix[1],o2->transformationMatrix[4],o2->transformationMatrix[7]);
	vect3D u3=vect(o2->transformationMatrix[2],o2->transformationMatrix[5],o2->transformationMatrix[8]);

	vect3D pp=vectDifference(o1->position,o2->position);

	getVertices(o1->size, vect(dotProduct(pp,u1),dotProduct(pp,u2),dotProduct(pp,u3)), vect(dotProduct(z1,u1),dotProduct(z1,u2),dotProduct(z1,u3)), 
		vect(dotProduct(z2,u1),dotProduct(z2,u2),dotProduct(z2,u3)), vect(dotProduct(z3,u1),dotProduct(z3,u2),dotProduct(z3,u3)), v2);

	int i;
	/*for(i=0;i<8;i++)
	{
		//v2[i]=vectDifference(v[i],o2->position);
		//v2[i]=vect(dotProduct(v2[i],u1),dotProduct(v2[i],u2),dotProduct(v2[i],u3));

		if(v2[i].x>-o2->size.x-MAXPENETRATIONBOX && v2[i].x<o2->size.x+MAXPENETRATIONBOX
		&& v2[i].y>-o2->size.y-MAXPENETRATIONBOX && v2[i].y<o2->size.y+MAXPENETRATIONBOX
		&& v2[i].z>-o2->size.z-MAXPENETRATIONBOX && v2[i].z<o2->size.z+MAXPENETRATIONBOX)
		{
			vect3D n;
			vect3D p=projectPointAABB(o2->size,v2[i],&n);
			o1->contactPoints[o1->numContactPoints].point=v[i];
			o1->contactPoints[o1->numContactPoints].normal=normalize(vect(n.x*u1.x+n.y*u2.x+n.z*u3.x,n.x*u1.y+n.y*u2.y+n.z*u3.y,n.x*u1.z+n.y*u2.z+n.z*u3.z));
			if(abs(n.x)+abs(n.y)+abs(n.z)!=1)
			{
				printf("erf\n");
			}
			o1->contactPoints[o1->numContactPoints].penetration=distance(v2[i],p);
			o1->contactPoints[o1->numContactPoints].penetration=0;
			o1->contactPoints[o1->numContactPoints].target=o2;
			o1->contactPoints[o1->numContactPoints].type=BOXCOLLISION;
			o1->maxPenetration=max(o1->maxPenetration,o1->contactPoints[o1->numContactPoints].penetration);
			o1->numContactPoints++;
		}
	}*/

	//optimize by working in o2 space ?
	vect3D vv2[8];
	getOBBVertices(o2,vv2);
	vect3D u[3];
		u[0]=vect(o1->transformationMatrix[0],o1->transformationMatrix[3],o1->transformationMatrix[6]);
		u[1]=vect(o1->transformationMatrix[1],o1->transformationMatrix[4],o1->transformationMatrix[7]);
		u[2]=vect(o1->transformationMatrix[2],o1->transformationMatrix[5],o1->transformationMatrix[8]);
	vect3D uu[3];
		uu[0]=vect(o2->transformationMatrix[0],o2->transformationMatrix[3],o2->transformationMatrix[6]);
		uu[1]=vect(o2->transformationMatrix[1],o2->transformationMatrix[4],o2->transformationMatrix[7]);
		uu[2]=vect(o2->transformationMatrix[2],o2->transformationMatrix[5],o2->transformationMatrix[8]);
	int32 s[3];
		s[0]=o1->size.x;
		s[1]=o1->size.y;
		s[2]=o1->size.z;
	int32 ss[3];
		ss[0]=o2->size.x;//+MAXPENETRATIONBOX;
		ss[1]=o2->size.y;//+MAXPENETRATIONBOX;
		ss[2]=o2->size.z;//+MAXPENETRATIONBOX;
	for(i=0;i<NUMOBBSEGMENTS;i++)
	{
		vect3D uu1=v2[OBBSegments[i][0]];
		vect3D uu2=v2[OBBSegments[i][1]];
		const bool t=!((uu1.x<-ss[0] && uu2.x<-ss[0]) || (uu1.x>ss[0] && uu2.x>ss[0])
				  || (uu1.y<-ss[1] && uu2.y<-ss[1]) || (uu1.y>ss[1] && uu2.y>ss[1])
				  || (uu1.z<-ss[2] && uu2.z<-ss[2]) || (uu1.z>ss[2] && uu2.z>ss[2]));
		if(t)
		{
			do{
				const vect3D vv=u[OBBSegmentsPD[i][1]];
				const vect3D vv1=vect(dotProduct(vv,uu[0]),dotProduct(vv,uu[1]),dotProduct(vv,uu[2]));
				vect3D p1=v[OBBSegments[i][0]];
				vect3D p2=v[OBBSegments[i][1]];
				vect3D n1=vect(0,0,0);
				vect3D n2=vect(0,0,0);
				int32 k1=0;
				int32 k2=s[OBBSegmentsPD[i][1]]*2;
				bool b1=false;
				bool b2=false;
				if(!clipSegmentOBB(ss,uu,&p1,&p2,vv,&uu1,&uu2,vv1,&n1,&n2,&b1,&b2,&k1,&k2))break;
				
				if(b1&&b2)
				{
					//p1=addVect(p1,vectMult(vv,k1));
					//p2=addVect(p2,vectMult(vv,k2));
					o1->contactPoints[o1->numContactPoints].point=vectDivInt(addVect(p1,p2),2);
					o1->contactPoints[o1->numContactPoints].type=TESTPOINT;
					vect3D n;
					vect3D oo=vectDivInt(addVect(uu1,uu2),2);
					vect3D p=projectPointAABB(o2->size,oo,&n);
					o1->contactPoints[o1->numContactPoints].normal=(vect(n.x*u1.x+n.y*u2.x+n.z*u3.x,n.x*u1.y+n.y*u2.y+n.z*u3.y,n.x*u1.z+n.y*u2.z+n.z*u3.z));
					o1->contactPoints[o1->numContactPoints].penetration=distance(p,oo);
					//o1->contactPoints[o1->numContactPoints].penetration=0;
					o1->contactPoints[o1->numContactPoints].target=o2;
					o1->numContactPoints++;	
				}else{
					if(b1)
					{
						//p1=addVect(p1,vectMult(vv,k1));
						o1->contactPoints[o1->numContactPoints].point=p1;
						o1->contactPoints[o1->numContactPoints].type=TESTPOINT;
						o1->contactPoints[o1->numContactPoints].normal=n1;
						o1->contactPoints[o1->numContactPoints].penetration=0;
						o1->contactPoints[o1->numContactPoints].target=o2;
						o1->numContactPoints++;
					}
					if(b2)
					{
						//p2=addVect(p2,vectMult(vv,k2));
						o1->contactPoints[o1->numContactPoints].point=p2;
						o1->contactPoints[o1->numContactPoints].type=TESTPOINT;
						o1->contactPoints[o1->numContactPoints].normal=n2;
						o1->contactPoints[o1->numContactPoints].penetration=0;
						o1->contactPoints[o1->numContactPoints].target=o2;
						o1->numContactPoints++;
					}
				}
			}while(0);
		}
	}
}

void initTransformationMatrix(int32* m)
{
	if(!m)return;
	int i;
	for(i=0;i<9;i++)m[i]=0;

	m[0]=inttof32(1);
	m[4]=inttof32(1);
	m[8]=inttof32(1);
}

void getVertices(vect3D s, vect3D p, vect3D u1, vect3D u2, vect3D u3, vect3D* v)
{
	int32 m2[9];
	m2[0]=mulf32(u1.x,s.x);m2[3]=mulf32(u1.y,s.x);m2[6]=mulf32(u1.z,s.x);
	m2[1]=mulf32(u2.x,s.y);m2[4]=mulf32(u2.y,s.y);m2[7]=mulf32(u2.z,s.y);
	m2[2]=mulf32(u3.x,s.z);m2[5]=mulf32(u3.y,s.z);m2[8]=mulf32(u3.z,s.z);

	v[0]=vect(-m2[0]-m2[1]-m2[2],-m2[3]-m2[4]-m2[5],-m2[6]-m2[7]-m2[8]);
	v[1]=vect(m2[0]-m2[1]-m2[2],m2[3]-m2[4]-m2[5],m2[6]-m2[7]-m2[8]);
	v[2]=vect(m2[0]-m2[1]+m2[2],m2[3]-m2[4]+m2[5],m2[6]-m2[7]+m2[8]);
	v[3]=vect(-m2[0]-m2[1]+m2[2],-m2[3]-m2[4]+m2[5],-m2[6]-m2[7]+m2[8]);
	
	v[4]=vect(-v[1].x,-v[1].y,-v[1].z);
	v[5]=vect(-v[2].x,-v[2].y,-v[2].z);
	v[6]=vect(-v[3].x,-v[3].y,-v[3].z);
	v[7]=vect(-v[0].x,-v[0].y,-v[0].z);
	
	v[0]=addVect(v[0],p);v[1]=addVect(v[1],p);v[2]=addVect(v[2],p);v[3]=addVect(v[3],p);
	v[4]=addVect(v[4],p);v[5]=addVect(v[5],p);v[6]=addVect(v[6],p);v[7]=addVect(v[7],p);
}

void getOBBVertices(OBB_struct* o, vect3D* v)
{
	if(!o || !v)return;
	int32* m=o->transformationMatrix;
	getVertices(o->size,o->position,vect(m[0],m[3],m[6]),vect(m[1],m[4],m[7]),vect(m[2],m[5],m[8]),v);

	int i;
	vect3D mm=o->position;
	vect3D M=o->position;
	for(i=0;i<8;i++)
	{
		//v[i]=addVect(v[i],o->position);
		if(v[i].x<mm.x)mm.x=v[i].x;
		if(v[i].y<mm.y)mm.y=v[i].y;
		if(v[i].z<mm.z)mm.z=v[i].z;
		if(v[i].x>M.x)M.x=v[i].x;
		if(v[i].y>M.y)M.y=v[i].y;
		if(v[i].z>M.z)M.z=v[i].z;
	}
	o->AABBo=mm;
	o->AABBs=vectDifference(M,mm);
}

void applyOBBForce(OBB_struct* o, vect3D p, vect3D f)
{
	if(!o)return;
	o->forces=addVect(o->forces,f);
	o->moment=addVect(o->moment,vectProduct(vectDifference(p,o->position),f));
}

void applyOBBImpulsePlane(OBB_struct* o, u8 pID)
{
	if(!o || pID>=o->numContactPoints)return;
	contactPoint_struct* cp=&o->contactPoints[pID];    
	vect3D r=vectDifference(cp->point,o->position);
	vect3D v=addVect(o->velocity,vectProduct(o->angularVelocity,r));

	const int32 CoefficientOfRestitution=floattof32(0.2f);
    
	int32 iN=-mulf32((floattof32(1)+CoefficientOfRestitution),dotProduct(v,cp->normal));
	//int32 invMass=divf32(inttof32(1),o->mass);
	int32 invMass=divv16(inttof32(1),o->mass);
	int32 iD=invMass+dotProduct(vectProduct(evalVectMatrix33(o->invWInertiaMatrix,vectProduct(r,cp->normal)),r),cp->normal);
    //iN=divf32(iN,iD);
    iN=divv16(iN,iD);
	if(iN<0)iN=0;
	//vect3D imp=vectMult(cp->normal,iN);
	vect3D imp=vectMult(cp->normal,iN+cp->penetration/2); //added bias adds jitter, but prevents sinking.

	//printf("imp : %d",iN);
    
    // apply impulse to primary quantities
	o->velocity=addVect(o->velocity,vectMult(imp,invMass));
	o->angularMomentum=addVect(o->angularMomentum,vectProduct(r,imp));
    
    // compute affected auxiliary quantities
	o->angularVelocity=evalVectMatrix33(o->invWInertiaMatrix,o->angularMomentum);
	
	{
		vect3D tangent=vect(0,0,0);
		tangent=vectDifference(v,(vectMult(cp->normal,dotProduct(v, cp->normal))));
		if(magnitude(tangent)<1)return;
		tangent=normalize(tangent);

		int32 kTangent=invMass+dotProduct(tangent,vectProduct(evalVectMatrix33(o->invWInertiaMatrix,(vectProduct(r, tangent))), r));			

		int32 vt = dotProduct(v, tangent);
		//int32 dPt = divf32((-vt),kTangent);
		int32 dPt = divv16((-vt),kTangent);

		const int32 frictionCONST=floattof32(1.0f);

		int32 maxPt=abs(mulf32(frictionCONST,iN));
		if(dPt<-maxPt)dPt=-maxPt;
		else if(dPt>maxPt)dPt=maxPt;

		// Apply contact impulse
		vect3D P = vectMult(tangent,dPt);

		o->velocity=addVect(o->velocity,vectMult(P,invMass));
		o->angularMomentum=addVect(o->angularMomentum,vectProduct(r,P));
    
		// compute affected auxiliary quantities
		o->angularVelocity=evalVectMatrix33(o->invWInertiaMatrix,o->angularMomentum);
	}
}

void applyOBBImpulseOBB(OBB_struct* o, u8 pID)
{
	if(!o || pID>=o->numContactPoints)return;
	contactPoint_struct* cp=&o->contactPoints[pID];
	OBB_struct* o2=(OBB_struct*)cp->target;
	if(!o2)return;
	vect3D r1=vectDifference(cp->point,o->position);
	vect3D r2=vectDifference(cp->point,o2->position);
	vect3D v1=addVect(o->velocity,vectProduct(o->angularVelocity,r1));
	vect3D v2=addVect(o2->velocity,vectProduct(o2->angularVelocity,r2));
	vect3D dv=vectDifference(v1,v2);

	const int32 CoefficientOfRestitution=floattof32(0.5f);
    
	int32 iN=-mulf32((floattof32(1)+CoefficientOfRestitution),dotProduct(dv,cp->normal));
	//int32 invMass1=divf32(inttof32(1),o->mass);
	//int32 invMass2=divf32(inttof32(1),o2->mass);
	int32 invMass1=divv16(inttof32(1),o->mass);
	int32 invMass2=divv16(inttof32(1),o2->mass);
	int32 iD=invMass1+invMass2+dotProduct(addVect(vectProduct(evalVectMatrix33(o->invWInertiaMatrix,vectProduct(r1,cp->normal)),r1),vectProduct(evalVectMatrix33(o2->invWInertiaMatrix,vectProduct(r2,cp->normal)),r2)),cp->normal);
	//iN=divf32(iN,iD);
	iN=divv16(iN,iD);
	if(iN<0)iN=0;
	//vect3D imp=vectMult(cp->normal,iN);
	vect3D imp=vectMult(cp->normal,iN+cp->penetration); //added bias adds jitter, but prevents sinking.

	//printf("norm : %d %d %d\n",cp->normal.x,cp->normal.y,cp->normal.z);
    
    // apply impulse to primary quantities
	o->velocity=addVect(o->velocity,vectMult(imp,invMass1));
	o->angularMomentum=addVect(o->angularMomentum,vectProduct(r1,imp));

	o2->velocity=vectDifference(o2->velocity,vectMult(imp,invMass2));
	o2->angularMomentum=vectDifference(o2->angularMomentum,vectProduct(r2,imp));
    
    // compute affected auxiliary quantities
	o->angularVelocity=evalVectMatrix33(o->invWInertiaMatrix,o->angularMomentum);

	o2->angularVelocity=evalVectMatrix33(o2->invWInertiaMatrix,o2->angularMomentum);
	
	{
		vect3D tangent=vect(0,0,0);
		tangent=vectDifference(dv,(vectMult(cp->normal,dotProduct(dv, cp->normal))));
		if(magnitude(tangent)<1)return;
		tangent=normalize(tangent);

		int32 kTangent=invMass1+invMass2+dotProduct(tangent,addVect(vectProduct(evalVectMatrix33(o->invWInertiaMatrix,(vectProduct(r1, tangent))), r1),
																	vectProduct(evalVectMatrix33(o->invWInertiaMatrix,(vectProduct(r2, tangent))), r2)));

		int32 vt = dotProduct(dv, tangent);
		//int32 dPt = divf32((-vt),kTangent);
		int32 dPt = divv16((-vt),kTangent);

		const int32 frictionCONST=floattof32(0.5f);

		int32 maxPt=abs(mulf32(frictionCONST,iN));
		if(dPt<-maxPt)dPt=-maxPt;
		else if(dPt>maxPt)dPt=maxPt;

		// Apply contact impulse
		vect3D P = vectMult(tangent,dPt);
		
		o->velocity=addVect(o->velocity,vectMult(P,invMass1));
		o->angularMomentum=addVect(o->angularMomentum,vectProduct(r1,P));

		o2->velocity=vectDifference(o2->velocity,vectMult(P,invMass2));
		o2->angularMomentum=vectDifference(o2->angularMomentum,vectProduct(r2,P));
    
		// compute affected auxiliary quantities
		o->angularVelocity=evalVectMatrix33(o->invWInertiaMatrix,o->angularMomentum);
		o2->angularVelocity=evalVectMatrix33(o2->invWInertiaMatrix,o2->angularMomentum);
	}
}

void applyOBBImpulses(OBB_struct* o)
{
	if(!o)return;
	int i;
	for(i=0;i<o->numContactPoints;i++)
	{
		switch(o->contactPoints[i].type)
		{
			case PLANECOLLISION:
				applyOBBImpulsePlane(o,i);
				break;
			case AARCOLLISION:
				applyOBBImpulsePlane(o,i);
				break;
			case BOXCOLLISION:
				// printf("collision ! %d",i);
				//applyOBBImpulseOBB(o,i);
				break;
			case TESTPOINT:
				// printf("test collision ! %d",i);
				applyOBBImpulseOBB(o,i);
				break;
		}
	}
}

void integrate(OBB_struct* o, float dt)
{
	if(!o)return;
	//o->position=addVect(o->position,vectMult(o->velocity,dt));
	//o->position=addVect(o->position,vect(mulf32(o->velocity.x,dt)>>TIMEPREC,mulf32(o->velocity.y,dt)>>TIMEPREC,mulf32(o->velocity.z,dt)>>TIMEPREC));
	//o->position=addVect(o->position,vect((mulf32((o->velocity.x+o->oldVelocity.x)/2,dt)>>TIMEPREC),
	//									(mulf32((o->velocity.y+o->oldVelocity.y)/2,dt)>>TIMEPREC),
	//									(mulf32((o->velocity.z+o->oldVelocity.z)/2,dt)>>TIMEPREC)));
	o->position=addVect(o->position,vect(o->velocity.x*dt,o->velocity.y*dt,o->velocity.z*dt));
	
	int32 m[9], m2[9];
	//m[0]=0;m[1]=-(mulf32(dt,o->angularVelocity.z));m[2]=(mulf32(dt,o->angularVelocity.y));
    //m[3]=-m[1];m[4]=0;m[5]=-(mulf32(dt,o->angularVelocity.x));
	//m[0]=0;m[1]=-(mulf32(dt,o->angularVelocity.z)>>TIMEPREC);m[2]=(mulf32(dt,o->angularVelocity.y)>>TIMEPREC);
    //m[3]=-m[1];m[4]=0;m[5]=-(mulf32(dt,o->angularVelocity.x)>>TIMEPREC);
	//m[0]=0;m[1]=-((mulf32(dt,(o->angularVelocity.z+o->oldAngularVelocity.z)/2)>>TIMEPREC));m[2]=((mulf32(dt,(o->angularVelocity.y+o->oldAngularVelocity.y)/2)>>TIMEPREC));
    //m[3]=-m[1];m[4]=0;m[5]=-((mulf32(dt,(o->angularVelocity.x+o->oldAngularVelocity.x)/2)>>TIMEPREC));
	m[0]=0;m[1]=-((dt*o->angularVelocity.z));m[2]=((dt*o->angularVelocity.y));
    m[3]=-m[1];m[4]=0;m[5]=-((dt*o->angularVelocity.x));
    m[6]=-m[2];m[7]=-m[5];m[8]=0;
	multMatrix33(m,o->transformationMatrix,m2);
	addMatrix33(o->transformationMatrix,m2,o->transformationMatrix);
	
	//o->velocity=addVect(o->velocity,divideVect(vectMult(o->forces,dt),o->mass));
	//o->velocity=addVect(o->velocity,divideVect(vect(mulf32(o->forces.x,dt)>>TIMEPREC,mulf32(o->forces.y,dt)>>TIMEPREC,mulf32(o->forces.z,dt)>>TIMEPREC),o->mass));
	//o->velocity=addVect(o->velocity,divideVect(vect((mulf32((o->forces.x+o->oldForces.x)/2,dt)>>TIMEPREC),
	//											(mulf32((o->forces.y+o->oldForces.y)/2,dt)>>TIMEPREC),
	//											(mulf32((o->forces.z+o->oldForces.z)/2,dt)>>TIMEPREC)),o->mass));
	o->velocity=addVect(o->velocity,divideVect(vect(o->forces.x*dt,o->forces.y*dt,o->forces.z*dt),o->mass));
	
	//o->angularMomentum=addVect(o->angularMomentum,vectMult(o->moment,dt));
	//o->angularMomentum=addVect(o->angularMomentum,vect(mulf32(o->moment.x,dt)>>TIMEPREC,mulf32(o->moment.y,dt)>>TIMEPREC,mulf32(o->moment.z,dt)>>TIMEPREC));
	//o->angularMomentum=addVect(o->angularMomentum,vect((mulf32((o->moment.x+o->oldMoment.x)/2,dt)>>TIMEPREC),
	//												(mulf32((o->moment.y+o->oldMoment.y)/2,dt)>>TIMEPREC),
	//												(mulf32((o->moment.z+o->oldMoment.z)/2,dt)>>TIMEPREC)));
	o->angularMomentum=addVect(o->angularMomentum,vect(o->moment.x*dt,o->moment.y*dt,o->moment.z*dt));
	
	fixMatrix(o->transformationMatrix);

    // compute auxiliary quantities
	transposeMatrix33(o->transformationMatrix,m2);
	multMatrix33(m2,o->invInertiaMatrix,m);
	multMatrix33(m,o->transformationMatrix,o->invWInertiaMatrix);
	
	o->angularVelocity=evalVectMatrix33(o->invWInertiaMatrix,o->angularMomentum);
}

extern plane_struct testPlane;
extern OBB_struct *testOBB, *testOBB2;

void checkOBBCollisions(OBB_struct* o, bool sleep)
{
	if(!o)return;
	int i;
	o->numContactPoints=0;
	// planeOBBContacts(&testPlane,o);
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used && o!=&objects[i] && (!sleep || !objects[i].sleep))
		{
			collideOBBs(o,&objects[i]);
		}
	}
	AARsOBBContacts(o, sleep);
}

void wakeOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			objects[i].counter=0;
			objects[i].sleep=false;
		}
	}
}

void calculateOBBEnergy(OBB_struct* o)
{
	if(!o)return;
	
	u32 tmp=dotProduct(o->velocity,o->velocity)+dotProduct(o->angularVelocity,o->angularVelocity);
	// o->energy=(o->energy*9+tmp)/10;
	o->energy=tmp;
}

u8 sleeping;

void simulate(OBB_struct* o, float dt2)
{
	if(!o)return;
	//int32 dt=(dt2)<<TIMEPREC;
	float dt=f32tofloat(dt2);
    float currentTime=0;
    float targetTime=dt;
	
	applyOBBForce(o,o->position,vect(0,-inttof32(2),0)); //gravity
	o->forces=addVect(o->forces,vectDivInt(o->velocity,-25));
	o->moment=addVect(o->moment,vectDivInt(o->angularVelocity,-20));
	
	if(!o->sleep)
	{
		while(currentTime<dt)
		{
			OBB_struct bkp;
			copyOBB(o,&bkp);

			// cpuStartTiming(0);
			integrate(o,targetTime-currentTime);
			// integ+=cpuEndTiming();

			// cpuStartTiming(0);
			checkOBBCollisions(o, false);
			// coll+=cpuEndTiming();
			
			// cpuStartTiming(0);
			if(o->numContactPoints && o->maxPenetration>PENETRATIONTHRESHOLD)
			{
				targetTime=(currentTime+targetTime)/2;
				copyOBB(&bkp,o);
				if(targetTime-currentTime<=0.000001f)
				{
					// printf("desp impulse\n");
					checkOBBCollisions(o, false);
					applyOBBImpulses(o);
					currentTime=targetTime;
					targetTime=dt;
				}
			}else if(o->numContactPoints)
			{
				// printf("impulse\n");
				applyOBBImpulses(o);
				currentTime=targetTime;
				targetTime=dt;
			}else{
				currentTime=targetTime;
				targetTime=dt;
			}
			// impul+=cpuEndTiming();
		}
		// collideSpherePlatforms(&o->position,o->size.x-8);
	}else sleeping++;

	calculateOBBEnergy(o);
	bool oldSleep=o->sleep;
	if(o->energy>=SLEEPTHRESHOLD*10)o->sleep=false;
	else if(o->energy<=SLEEPTHRESHOLD)
	{
		o->counter++;
		if(o->counter>=SLEEPTIMETHRESHOLD)o->sleep=true;
	}else o->counter=0;

	if(o->sleep)
	{
		if(oldSleep)checkOBBCollisions(o, true); //make it so sleeping collisions don't have to check vs AARs and other sleeping OBBs
		{
			int i;
			bool canSleep=oldSleep;
			for(i=0;i<o->numContactPoints;i++)
			{
				switch(o->contactPoints[i].type)
				{
					case AARCOLLISION:
						canSleep=true;
						break;
					case PLANECOLLISION:
						canSleep=true;
						break;
					case TESTPOINT:
					case BOXCOLLISION:
						{
							OBB_struct* o2=(OBB_struct*)o->contactPoints[i].target;
							if(o2->energy<=SLEEPTHRESHOLD && o2->counter>=SLEEPTIMETHRESHOLD && o->energy<=SLEEPTHRESHOLD)
							{
								canSleep=true;
								o->sleep=true;
								o2->sleep=true;
								// i=o->numContactPoints; //get a similar system to work with more than 2 boxes in contact ? (only make it for !o2->sleep ?)
							}else{
								canSleep=false;
								o2->sleep=false;
								i=o->numContactPoints;
							}
						}
						break;
				}
			}
			if(!canSleep)o->sleep=false;
		}
	}
		
	o->forces=vect(0,0,0);
	o->moment=vect(0,0,0);
}

bool pointInFrontOfPortal(portal_struct* p, vect3D pos, int32* z) //assuming correct normal
{
	if(!p)return false;
	const vect3D v2=vectDifference(pos, p->position); //then, project onto portal base
	vect3D v=vect(dotProduct(p->plane[0],v2),dotProduct(p->plane[1],v2),0);
	*z=dotProduct(p->normal,v2);
	return (v.y>-PORTALSIZEY*4 && v.y<PORTALSIZEY*4 && v.x>-PORTALSIZEX*4 && v.x<PORTALSIZEX*4);
}

void updateOBBPortals(OBB_struct* o, u8 id, bool init)
{
	if(!o&&id<2)return;

	int32 z;
	o->oldPortal[id]=o->portal[id];
	o->portal[id]=((dotProduct(vectDifference(o->position,portal[id].position),portal[id].normal)>0)&1)|(((pointInFrontOfPortal(&portal[id],o->position,&z))&1)<<1);
	
	switch(init)
	{
		case false:
			if(((o->oldPortal[id]&1) && !(o->portal[id]&1) && (o->oldPortal[id]&2 || o->portal[id]&2)) || (o->portal[id]&2 && z<=0 && z>-16))
			{
				o->position=addVect(portal[id].targetPortal->position,warpVector(&portal[id],vectDifference(o->position,portal[id].position)));
				o->velocity=warpVector(&portal[id],o->velocity);
				o->forces=warpVector(&portal[id],o->forces);
				o->angularVelocity=warpVector(&portal[id],o->angularVelocity);
				o->moment=warpVector(&portal[id],o->moment);
				
				warpMatrix(&portal[id], o->transformationMatrix);
				warpMatrix(&portal[id], o->invWInertiaMatrix);
			}
			break;
		default:
			o->oldPortal[id]=o->portal[id];
			break;
	}
}

void updateOBB(OBB_struct* o)
{
	if(!o)return;
	
	simulate(o,20);
	
	if(portal[0].used&&portal[1].used)
	{
		updateOBBPortals(o,0,false);
		updateOBBPortals(o,1,false);
	}
}

void updateOBBs(void)
{
	int i;
	sleeping=0;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			updateOBB(&objects[i]);
		}
	}
}

OBB_struct* createOBB(u8 id, vect3D size, vect3D position, int32 mass)
{
	int i=id;
		// if(!objects[i].used)
		{
			initOBB(&objects[i],size,position,mass);
			return &objects[i];
		}
	return NULL;
}

void drawOBBs(void)
{
	int i;
	for(i=0;i<NUMOBJECTS;i++)
	{
		if(objects[i].used)
		{
			drawOBB(&objects[i]);
		}
	}
}

void drawOBB(OBB_struct* o)
{

}
