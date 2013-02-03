#include "stdafx.h"

#define ALLOCATORSIZE (8*1024)

u8 allocatorPool[ALLOCATORSIZE];
u16 allocatorCounter;

AAR_struct aaRectangles[NUMAARS];
grid_struct AARgrid;

static const u8 AARSegments[NUMAARSEGMENTS][2]={{0,1},{1,2},{3,2},{0,3}};
static const u8 AARSegmentsPD[NUMAARSEGMENTS][2]={{0,0},{1,1},{3,0},{0,1}};

extern portal_struct portal[2];

void initAllocator(void)
{
	allocatorCounter=0;
}

void* allocateData(u16 size)
{
	if(allocatorCounter+size>ALLOCATORSIZE){return NULL;}
	allocatorCounter+=size;
	return &allocatorPool[allocatorCounter-size];
}

void initAARs(void)
{
	int i;
	for(i=0;i<NUMAARS;i++)
	{
		aaRectangles[i].used=false;
	}
	
	AARgrid.nodes=NULL;
	AARgrid.width=AARgrid.height=0;
	
	initAllocator();
}

void freeGrid(grid_struct* g)
{
	if(!g || !g->nodes)return;
	g->nodes=NULL;
	initAllocator();
}

void generateGrid(grid_struct* g)
{
	if(!g)g=&AARgrid;
	
	freeGrid(g);
	
	int i;
	bool b=false;
	vect3D m, M;
	m=vect((1<<29),(1<<29),(1<<29));
	M=vect(-(1<<29),-(1<<29),-(1<<29));
	for(i=0;i<NUMAARS;i++)
	{
		if(aaRectangles[i].used)
		{
			if(aaRectangles[i].position.x<m.x)m.x=aaRectangles[i].position.x;
			if(aaRectangles[i].position.z<m.z)m.z=aaRectangles[i].position.z;
			
			if(aaRectangles[i].position.x+aaRectangles[i].size.x>M.x)M.x=aaRectangles[i].position.x+aaRectangles[i].size.x;
			if(aaRectangles[i].position.z+aaRectangles[i].size.z>M.z)M.z=aaRectangles[i].position.z+aaRectangles[i].size.z;
			b=true;
		}
	}
	if(!b)return;
	g->width=(M.x-m.x)/NODESIZE+1;
	g->height=(M.z-m.z)/NODESIZE+1;
	g->m=m;
	g->M=M;
	
	g->nodes=allocateData(sizeof(node_struct)*g->width*g->height);
	
	fifoSendValue32(FIFO_USER_08,allocatorCounter);
	
	static u16 temp[NUMAARS];
	
	int j, k;
	for(i=0;i<g->width;i++)
	{
		for(j=0;j<g->height;j++)
		{
			node_struct* n=&g->nodes[i+j*g->width];
			n->length=0;
			for(k=0;k<NUMAARS;k++)
			{
				if(aaRectangles[k].used)
				{
					const int32 mX=g->m.x+i*NODESIZE, MX=g->m.x+(i+1)*NODESIZE;
					const int32 mZ=g->m.z+j*NODESIZE, MZ=g->m.z+(j+1)*NODESIZE;
					if(!((aaRectangles[k].position.x<mX && aaRectangles[k].position.x+aaRectangles[k].size.x<mX)
					||   (aaRectangles[k].position.x>MX && aaRectangles[k].position.x+aaRectangles[k].size.x>MX)
					||   (aaRectangles[k].position.z<mZ && aaRectangles[k].position.z+aaRectangles[k].size.z<mZ)
					||   (aaRectangles[k].position.z>MZ && aaRectangles[k].position.z+aaRectangles[k].size.z>MZ)))
					{
						temp[n->length]=k;
						n->length++;
					}
				}
			}
			if(n->length){n->data=allocateData(sizeof(u16)*n->length);memcpy(n->data,temp,n->length*sizeof(u16));}
			else n->data=NULL;
		}
	}
	fifoSendValue32(FIFO_USER_08,allocatorCounter);
}

void getOBBNodes(grid_struct* g, OBB_struct* o, u16* x, u16* X, u16* z, u16* Z)
{
	if(!o)return;
	if(!g)g=&AARgrid;
	
	const vect3D m=vectDifference(o->AABBo,g->m);
	const vect3D M=addVect(m,o->AABBs);
	
	*x=m.x/NODESIZE;*z=m.z/NODESIZE;
	*X=M.x/NODESIZE;*Z=M.z/NODESIZE;
}

AAR_struct* createAAR(u16 id, vect3D position, vect3D size, vect3D normal)
{
	int i=id;
	// for(i=0;i<NUMAARS;i++)
	// {
		if(!aaRectangles[i].used)
		{
			aaRectangles[i].used=true;
			aaRectangles[i].position=position;
			aaRectangles[i].size=size;
			aaRectangles[i].normal=normal;
			// fifoSendValue32(FIFO_USER_08,i);
			return &aaRectangles[i];
		}
	// }
	return NULL;
}

void updateAAR(u16 id, vect3D position)
{
	int i=id;
	// for(i=0;i<NUMAARS;i++)
	// {
		if(aaRectangles[i].used)
		{
			aaRectangles[i].position=position;
		}
	// }
}

bool pointInPortal(portal_struct* p, vect3D pos) //assuming correct normal
{
	if(!p)return false;
	const vect3D v2=vectDifference(pos, p->position); //then, project onto portal base
	vect3D v=vect(dotProduct(p->plane[0],v2),dotProduct(p->plane[1],v2),dotProduct(p->normal,v2));
	return (abs(v.z)<16 && v.y>-PORTALSIZEY*4 && v.y<PORTALSIZEY*4 && v.x>-PORTALSIZEX*4 && v.x<PORTALSIZEX*4);
}

void OBBAARContacts(AAR_struct* a, OBB_struct* o, bool port)
{
	if(!a || !o || !o->used || !a->used)return;
	
	vect3D u[3];
		u[0]=vect(o->transformationMatrix[0],o->transformationMatrix[3],o->transformationMatrix[6]);
		u[1]=vect(o->transformationMatrix[1],o->transformationMatrix[4],o->transformationMatrix[7]);
		u[2]=vect(o->transformationMatrix[2],o->transformationMatrix[5],o->transformationMatrix[8]);
		
	vect3D v[4], vv[4];
	vect3D uu[2], uuu[2];
		v[0]=vectDifference(a->position,o->position);
		v[2]=addVect(v[0],a->size);
		if(a->normal.x)
		{
			uu[0]=vect(0,inttof32(1),0);
			uu[1]=vect(0,0,inttof32(1));
			
			v[1]=addVect(v[0],vect(0,a->size.y,0));
			v[3]=addVect(v[0],vect(0,0,a->size.z));
		}else if(a->normal.y)
		{
			uu[0]=vect(inttof32(1),0,0);
			uu[1]=vect(0,0,inttof32(1));
			
			v[1]=addVect(v[0],vect(a->size.x,0,0));
			v[3]=addVect(v[0],vect(0,0,a->size.z));
		}else{
			uu[0]=vect(inttof32(1),0,0);
			uu[1]=vect(0,inttof32(1),0);
			
			v[1]=addVect(v[0],vect(a->size.x,0,0));
			v[3]=addVect(v[0],vect(0,a->size.y,0));
		}
	int i;
	for(i=0;i<4;i++) //optimizeable
	{
		vv[i]=vect(dotProduct(v[i],u[0]),dotProduct(v[i],u[1]),dotProduct(v[i],u[2]));
		v[i]=addVect(v[i],o->position);
	}
	uuu[0]=vect(dotProduct(uu[0],u[0]),dotProduct(uu[0],u[1]),dotProduct(uu[0],u[2]));
	uuu[1]=vect(dotProduct(uu[1],u[0]),dotProduct(uu[1],u[1]),dotProduct(uu[1],u[2]));

	int32 ss[3];
		ss[0]=o->size.x;//+MAXPENETRATIONBOX;
		ss[1]=o->size.y;//+MAXPENETRATIONBOX;
		ss[2]=o->size.z;//+MAXPENETRATIONBOX;
	
	for(i=0;i<NUMAARSEGMENTS;i++)
	{
		vect3D p1=v[AARSegments[i][0]];
		vect3D p2=v[AARSegments[i][1]];
		vect3D uu1=vv[AARSegments[i][0]];
		vect3D uu2=vv[AARSegments[i][1]];
		vect3D n1, n2;
		int32 k1, k2;
		bool b1, b2;
		if(clipSegmentOBB(ss, u, &p1, &p2, uu[AARSegmentsPD[i][1]], &uu1, &uu2, uuu[AARSegmentsPD[i][1]], &n1, &n2, &b1, &b2, &k1, &k2))
		{
			if(b1)
			{
				bool b=false;
				if(port)
				{
					if((portal[0].normal.x&&a->normal.x)||(portal[0].normal.y&&a->normal.y)||(portal[0].normal.z&&a->normal.z))b=pointInPortal(&portal[0],p1);
					if(!b&&((portal[1].normal.x&&a->normal.x)||(portal[1].normal.y&&a->normal.y)||(portal[1].normal.z&&a->normal.z)))b=pointInPortal(&portal[1],p1);
				}
				if(!b)
				{
					//p1=addVect(p1,vectMult(vv,k1));
					o->contactPoints[o->numContactPoints].point=p1;
					o->contactPoints[o->numContactPoints].type=PLANECOLLISION;
					o->contactPoints[o->numContactPoints].normal=a->normal;
					o->contactPoints[o->numContactPoints].penetration=0;
					o->contactPoints[o->numContactPoints].target=NULL;
					o->numContactPoints++;
				}
			}
			if(b2)
			{
				bool b=false;
				if(port)
				{
					if((portal[0].normal.x&&a->normal.x)||(portal[0].normal.y&&a->normal.y)||(portal[0].normal.z&&a->normal.z))b=pointInPortal(&portal[0],p2);
					if(!b&&((portal[1].normal.x&&a->normal.x)||(portal[1].normal.y&&a->normal.y)||(portal[1].normal.z&&a->normal.z)))b=pointInPortal(&portal[1],p2);
				}
				if(!b)
				{
					//p2=addVect(p2,vectMult(vv,k2));
					o->contactPoints[o->numContactPoints].point=p2;
					o->contactPoints[o->numContactPoints].type=PLANECOLLISION;
					o->contactPoints[o->numContactPoints].normal=a->normal;
					o->contactPoints[o->numContactPoints].penetration=0;
					o->contactPoints[o->numContactPoints].target=NULL;
					o->numContactPoints++;
				}
			}
		}
	}
}

bool AAROBBContacts(AAR_struct* a, OBB_struct* o, vect3D* v, bool port)
{
	if(!a || !o || !o->used || !a->used)return false;
	
	u16 oldnum=o->numContactPoints;

	vect3D u[3];
		u[0]=vect(o->transformationMatrix[0],o->transformationMatrix[3],o->transformationMatrix[6]);
		u[1]=vect(o->transformationMatrix[1],o->transformationMatrix[4],o->transformationMatrix[7]);
		u[2]=vect(o->transformationMatrix[2],o->transformationMatrix[5],o->transformationMatrix[8]);

	if(a->normal.x)
	{
		if(a->position.x>o->AABBo.x+o->AABBs.x || a->position.x<o->AABBo.x)return false;
		int i;
		bool vb[8];
		for(i=0;i<8;i++)vb[i]=v[i].x>a->position.x;
		for(i=0;i<NUMOBBSEGMENTS;i++) //possible to only check half !
		{
			if(vb[OBBSegments[i][0]]!=vb[OBBSegments[i][1]])
			{
				const vect3D uu=v[OBBSegmentsPD[i][0]];
				const vect3D vv=u[OBBSegmentsPD[i][1]];
				const int32 k=divv16(abs(uu.x-a->position.x),abs(vv.x));
				const vect3D p=addVect(uu,vectMult(vv,k));
				if(p.y>a->position.y && p.y<a->position.y+a->size.y && p.z>a->position.z && p.z<a->position.z+a->size.z)
				{
					bool b=false;
					if(port)
					{
						if(portal[0].normal.x)b=pointInPortal(&portal[0],p);
						if(!b&&portal[1].normal.x)b=pointInPortal(&portal[1],p);
					}
					if(!b)
					{
						o->contactPoints[o->numContactPoints].point=p;
						o->contactPoints[o->numContactPoints].type=AARCOLLISION;
						o->contactPoints[o->numContactPoints].normal=a->normal;
						o->contactPoints[o->numContactPoints].penetration=0;
						o->contactPoints[o->numContactPoints].target=NULL;
						o->numContactPoints++;
					}
				}
			}
		}
	}else if(a->normal.y)
	{
		if(a->position.y>o->AABBo.y+o->AABBs.y || a->position.y<o->AABBo.y)return false;
		int i;
		bool vb[8];
		for(i=0;i<8;i++)vb[i]=v[i].y>a->position.y;
		for(i=0;i<NUMOBBSEGMENTS;i++)
		{
			if(vb[OBBSegments[i][0]]!=vb[OBBSegments[i][1]])
			{
				const vect3D uu=v[OBBSegmentsPD[i][0]];
				const vect3D vv=u[OBBSegmentsPD[i][1]];
				const int32 k=divv16(abs(uu.y-a->position.y),abs(vv.y));
				const vect3D p=addVect(uu,vectMult(vv,k));
				if(p.x>a->position.x && p.x<a->position.x+a->size.x && p.z>a->position.z && p.z<a->position.z+a->size.z)
				{
					bool b=false;
					if(port)
					{
						if(portal[0].normal.y)b=pointInPortal(&portal[0],p);
						if(!b&&portal[1].normal.y)b=pointInPortal(&portal[1],p);
					}
					if(!b)
					{
						o->contactPoints[o->numContactPoints].point=p;
						o->contactPoints[o->numContactPoints].type=AARCOLLISION;
						o->contactPoints[o->numContactPoints].normal=a->normal;
						o->contactPoints[o->numContactPoints].penetration=0;
						o->contactPoints[o->numContactPoints].target=NULL;
						o->numContactPoints++;
					}
				}
			}
		}
	}else{
		if(a->position.z>o->AABBo.z+o->AABBs.z || a->position.z<o->AABBo.z)return false;
		int i;
		bool vb[8];
		for(i=0;i<8;i++)vb[i]=v[i].z>a->position.z;
		for(i=0;i<NUMOBBSEGMENTS;i++)
		{
			if(vb[OBBSegments[i][0]]!=vb[OBBSegments[i][1]])
			{
				const vect3D uu=v[OBBSegmentsPD[i][0]];
				const vect3D vv=u[OBBSegmentsPD[i][1]];
				const int32 k=divv16(abs(uu.z-a->position.z),abs(vv.z));
				const vect3D p=addVect(uu,vectMult(vv,k));
				if(p.x>a->position.x && p.x<a->position.x+a->size.x && p.y>a->position.y && p.y<a->position.y+a->size.y)
				{
					bool b=false;
					if(port)
					{
						if(portal[0].normal.z)b=pointInPortal(&portal[0],p);
						if(!b&&portal[1].normal.z)b=pointInPortal(&portal[1],p);
					}
					if(!b)
					{
						o->contactPoints[o->numContactPoints].point=p;
						o->contactPoints[o->numContactPoints].type=AARCOLLISION;
						o->contactPoints[o->numContactPoints].normal=a->normal;
						o->contactPoints[o->numContactPoints].penetration=0;
						o->contactPoints[o->numContactPoints].target=NULL;
						o->numContactPoints++;
					}
				}
			}
		}
	}

	OBBAARContacts(a, o, port);
	
	return o->numContactPoints>oldnum;
}

void AARsOBBContacts(OBB_struct* o, bool sleep)
{
	int i, j, k;
	bool port=portal[0].used&&portal[1].used;
	vect3D v[8];
	getOBBVertices(o,v);
	if(!sleep)
	{
		u16 x, X, z, Z;
		getOBBNodes(NULL, o, &x, &X, &z, &Z);
		bool lalala[NUMAARS];
		for(i=0;i<NUMAARS;i++)lalala[i]=0;
		o->groundID=-1;
		for(i=x;i<=X;i++)
		{
			for(j=z;j<=Z;j++)
			{
				node_struct* n=&AARgrid.nodes[i+j*AARgrid.width];
				for(k=0;k<n->length;k++)
				{
					u16 old=o->numContactPoints;
					
					if(!lalala[n->data[k]])
					{
						AAROBBContacts(&aaRectangles[n->data[k]], o, v, port);
					}
					if(o->groundID<0 && o->numContactPoints>old && aaRectangles[n->data[k]].normal.y>0)o->groundID=n->data[k];
					lalala[n->data[k]]=1;
				}
			}
		}
		if(port)
		{
			AAROBBContacts(&portal[0].guideAAR[0], o, v, false);
			AAROBBContacts(&portal[0].guideAAR[1], o, v, false);
			AAROBBContacts(&portal[0].guideAAR[2], o, v, false);
			AAROBBContacts(&portal[0].guideAAR[3], o, v, false);

			AAROBBContacts(&portal[1].guideAAR[0], o, v, false);
			AAROBBContacts(&portal[1].guideAAR[1], o, v, false);
			AAROBBContacts(&portal[1].guideAAR[2], o, v, false);
			AAROBBContacts(&portal[1].guideAAR[3], o, v, false);
		}
	}
	collideOBBPlatforms(o, v);
}

void fixAAR(AAR_struct* a)
{
	if(!a)return;
	
	if(a->size.x<0){a->position.x+=a->size.x;a->size.x=-a->size.x;}
	if(a->size.y<0){a->position.y+=a->size.y;a->size.y=-a->size.y;}
	if(a->size.z<0){a->position.z+=a->size.z;a->size.z=-a->size.z;}
}

void generateGuidAAR(portal_struct* p)
{
	if(!p)return;
	
	p->guideAAR[0].used=true;
	p->guideAAR[0].position=vectDifference(p->position,vectMultInt(addVect(vectDivInt(p->plane[0],PORTALFRACTIONX),vectDivInt(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[0].size=vectMultInt(addVect(vectDivInt(vectMultInt(p->plane[0],2),PORTALFRACTIONX),vectDivInt(p->normal,-8)),4);
	p->guideAAR[0].normal=p->plane[1];
	fixAAR(&p->guideAAR[0]);
	
	p->guideAAR[1].used=true;
	p->guideAAR[1].position=vectDifference(p->position,vectMultInt(addVect(vectDivInt(p->plane[0],PORTALFRACTIONX),vectDivInt(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[1].size=vectMultInt(addVect(vectDivInt(vectMultInt(p->plane[1],2),PORTALFRACTIONX),vectDivInt(p->normal,-8)),4);
	p->guideAAR[1].normal=p->plane[0];
	fixAAR(&p->guideAAR[1]);
	
	p->guideAAR[2].used=true;
	p->guideAAR[2].position=addVect(p->position,vectMultInt(addVect(vectDivInt(p->plane[0],PORTALFRACTIONX),vectDivInt(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[2].size=vectMultInt(addVect(vectDivInt(vectMultInt(p->plane[0],2),PORTALFRACTIONX),vectDivInt(p->normal,-8)),4);
	p->guideAAR[2].normal=vectMultInt(p->plane[1],-1);
	fixAAR(&p->guideAAR[2]);
	
	p->guideAAR[3].used=true;
	p->guideAAR[3].position=vectDifference(p->position,vectMultInt(addVect(vectDivInt(p->plane[0],PORTALFRACTIONX),vectDivInt(p->plane[1],PORTALFRACTIONY)),4));
	p->guideAAR[3].size=vectMultInt(addVect(vectDivInt(vectMultInt(p->plane[1],2),PORTALFRACTIONX),vectDivInt(p->normal,-8)),4);
	p->guideAAR[3].normal=vectMultInt(p->plane[0],-1);
	fixAAR(&p->guideAAR[3]);
}
	
