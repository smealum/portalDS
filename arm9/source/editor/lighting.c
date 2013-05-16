#include "editor/editor_main.h"

#define A5I3

#define MAXTESSELATION (5)
#define LIGHTGAP (3)

void initLightData(lightingData_struct* ld)
{
	if(!ld)return;

	ld->type=LIGHTMAP_DATA;
	ld->size=0;
	ld->data.lightMap.buffer=NULL;
	ld->data.lightMap.coords=NULL;
}

void freeLightData(lightingData_struct* ld)
{
	if(!ld)return;

	switch(ld->type)
	{
		case LIGHTMAP_DATA:
			if(ld->data.lightMap.coords)free(ld->data.lightMap.coords);
			if(ld->data.lightMap.buffer)free(ld->data.lightMap.buffer);
			ld->data.lightMap.buffer=NULL;
			ld->data.lightMap.coords=NULL;
			if(ld->data.lightMap.texture)ld->data.lightMap.texture->used=false;
			ld->size=0;
			break;
		default:
			break;
	}
}

//LIGHTMAP

void initLightDataLM(lightingData_struct* ld, u16 n)
{
	if(!ld)return;

	ld->type=LIGHTMAP_DATA;
	ld->size=n;
	ld->data.lightMap.lmSize=vect(0,0,0);
	ld->data.lightMap.buffer=NULL;
	ld->data.lightMap.coords=(lightMapCoordinates_struct*)malloc(sizeof(lightMapCoordinates_struct)*n);
	ld->data.lightMap.texture=NULL;
}

u8 computeLighting(vect3D l, int32 intensity, vect3D p, rectangle_struct* rec, room_struct* r)
{
	int32 dist=sqDistance(l,p);
	int32 rdist=sqrtf32(dist);
	dist=mulf32(dist,dist);
	// int32 dist=distance(l,p);
	if(dist<intensity)
	{
		vect3D u=vectDifference(p,l);
		u=divideVect(u,rdist);
		if(collideLineMap(r, rec, l, u, rdist, NULL, NULL))return 0;
		int32 v=dotProduct(u,rec->normal);
		v=max(0,v);
		v*=3;
		v/=4;
		v+=inttof32(1)/4;
		// int32 v=inttof32(1);
		return mulf32(v,(31-((dist*31)/intensity)));
	}else return 0;
}

u8 computeLightings(vect3D p, rectangle_struct* rec, room_struct* r)
{
	int v=AMBIENTLIGHT;
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		if(lights[i].used)
		{
			light_struct* l=&lights[i];
			v+=computeLighting(vect(l->position.x*TILESIZE*2,l->position.y*HEIGHTUNIT,l->position.z*TILESIZE*2), l->intensity, p, rec, r);
		}
	}
	return (u8)(31-min(max(v,0),31));
}

void fillBuffer(u8* buffer, vect2D p, vect2D s, u8* v, bool rot, int w)
{
	if(!buffer || !v)return;
	int i;
	// u8 vt=(rand()%31)<<3;
	if(!rot)
	{
		NOGBA("bounds %d %d",p.x+s.x,p.y+s.y);
		for(i=0;i<s.x;i++)
		{
			int j;
			for(j=0;j<s.y;j++)
			{
				buffer[p.x+i+(p.y+j)*w]=v[i+j*s.x];
				// buffer[p.x+i+(p.y+j)*w]=vt;
			}
		}
	}else{
		NOGBA("bounds %d %d",p.x+s.y,p.y+s.x);
		for(i=0;i<s.x;i++)
		{
			int j;
			for(j=0;j<s.y;j++)
			{
				buffer[p.x+j+(p.y+i)*w]=v[i+j*s.x];
				// buffer[p.x+j+(p.y+i)*w]=vt;
			}
		}
	}
}

void generateLightmap(rectangle_struct* rec, room_struct* r, lightMapData_struct* lmd, u8* b, lightMapCoordinates_struct* lmc)
{
	if(rec && b)
	{
		u16 x=lmc->lmSize.x, y=lmc->lmSize.y;
		u8* data=malloc(x*y);
		if(!data)return;
		vect3D p=vect(rec->position.x*TILESIZE*2-TILESIZE,rec->position.y*HEIGHTUNIT,rec->position.z*TILESIZE*2-TILESIZE);
		NOGBA("p : %d, %d, %d",p.x,p.y,p.z);
		int i;
		vect3D u=getUnitVect(rec);
		for(i=0;i<x;i++)
		{
			int j;
			for(j=0;j<y;j++)
			{
				#ifdef A5I3
					if(!rec->size.x)data[i+j*x]=computeLightings(addVect(p,vect(0,i*u.y+u.y/2,j*u.z+u.z/2)),rec,r)<<3;
					else if(rec->size.y)data[i+j*x]=computeLightings(addVect(p,vect(i*u.x+u.x/2,j*u.y+u.y/2,0)),rec,r)<<3;
					else data[i+j*x]=computeLightings(addVect(p,vect(i*u.x+u.x/2,0,j*u.z+u.z/2)),rec,r)<<3;
				#else
					if(!rec->size.x)data[i+j*x]=computeLightings(addVect(p,vect(0,i*u.y+u.y/2,j*u.z+u.z/2)),rec,r);//<<3;
					else if(rec->size.y)data[i+j*x]=computeLightings(addVect(p,vect(i*u.x+u.x/2,j*u.y+u.y/2,0)),rec,r);//<<3;
					else data[i+j*x]=computeLightings(addVect(p,vect(i*u.x+u.x/2,0,j*u.z+u.z/2)),rec,r);//<<3;
				#endif
			}
		}
		fillBuffer(b, vect2(lmc->lmPos.x,lmc->lmPos.y), vect2(lmc->lmSize.x,lmc->lmSize.y), data, lmc->rot, lmd->lmSize.x);
		free(data);
	}else NOGBA("NOTHING?");
}

void generateLightmaps(room_struct* r, lightingData_struct* ld)
{
	if(!r)return;
	listCell_struct *lc=r->rectangles.first;
	rectangle2DList_struct rl;
	initRectangle2DList(&rl);
	int i=0;
	initLightDataLM(ld, r->rectangles.num);

	while(lc)
	{
		insertRectangle2DList(&rl,(rectangle2D_struct){vect2(0,0),vect2(abs(lc->data.size.x?(lc->data.size.x*LIGHTMAPRESOLUTION):(lc->data.size.y*LIGHTMAPRESOLUTION*HEIGHTUNIT/(TILESIZE*2))),
																		abs((lc->data.size.y&&lc->data.size.x)?(lc->data.size.y*LIGHTMAPRESOLUTION*HEIGHTUNIT/(TILESIZE*2)):(lc->data.size.z*LIGHTMAPRESOLUTION))),
																		&ld->data.lightMap.coords[i++], false});
		lc=lc->next;
	}
	short w=32, h=32;

	bool rr=packRectanglesSize(&rl, &w, &h);
	ld->data.lightMap.lmSize=vect(w,h,0);
	NOGBA("done : %d %dx%d",(int)rr,w,h);

	if(!rr){freeLightData(ld);return;} //TEMP
	ld->data.lightMap.buffer=malloc(w*h);
	if(!ld->data.lightMap.buffer){freeLightData(ld);return;}

	lc=r->rectangles.first;
	i=0;
	while(lc)
	{
		generateLightmap(&lc->data, r, &ld->data.lightMap, ld->data.lightMap.buffer, &ld->data.lightMap.coords[i++]);
		lc=lc->next;
	}
		
	freeRectangle2DList(&rl);
	NOGBA("freed.");
}

// VERTEX LIGHTING

void initLightDataVL(lightingData_struct* ld, u16 n)
{
	if(!ld)return;

	ld->type=VERTEXLIGHT_DATA;
	ld->size=n;
	ld->data.vertexLighting=(vertexLightingData_struct*)malloc(sizeof(vertexLightingData_struct)*n);
}

u8 computeVertexLighting(vect3D l, int32 intensity, vect3D p, vect3D n)
{
	// NOGBA("L %d %d %d",l.x,l.y,l.z);
	int32 dist=sqDistance(l,p);
	int32 rdist=sqrtf32(dist);
	dist=mulf32(dist,dist);
	if(dist<intensity)
	{
		vect3D u=vectDifference(p,l);
		u=divideVect(u,rdist);
		int32 v=dotProduct(u,n);
		v=max(0,v);
		// v*=3;
		// v/=4;
		// v+=inttof32(1)/4;
		return mulf32(v,(31-((dist*31)/intensity)));
	}else return 0;
}

u8 computeVertexLightings(vect3D p, vect3D n)
{
	int v=AMBIENTLIGHT;
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		if(lights[i].used)
		{
			light_struct* l=&lights[i];
			v+=computeVertexLighting(vect(l->position.x*TILESIZE*2,l->position.y*HEIGHTUNIT,l->position.z*TILESIZE*2), l->intensity, p, n);
		}
	}
	return (u8)(min(max(v,0),31));
}

void generateRectangleVertexLightingWH(room_struct* r, rectangle_struct* rec, vertexLightingData_struct* vld, u8* wgap, u8* hgap)
{
	if(!r || !rec || !vld)return;

	u8 num=vld->width*vld->height;

	if(vld->values)free(vld->values);
	vld->values=malloc(sizeof(u8)*num);
	if(!vld->values)return;

	vect3D o=convertVect(rec->position);
	vect3D v1=vect((rec->size.x*TILESIZE*2)/(vld->width-1),0,0);
	vect3D v2=vect(0,0,(rec->size.z*TILESIZE*2)/(vld->height-1));

	if(!rec->size.x)v1=vect(0,(rec->size.y*HEIGHTUNIT)/(vld->width-1),0);
	if(!rec->size.z)v2=vect(0,(rec->size.y*HEIGHTUNIT)/(vld->height-1),0);

	vect3D p=o;

	u8 wgap1=0;
	u8 hgap1=0;

	int i, j;
	int k=0;
	for(j=0;j<vld->width;j++)
	{
		p=o;
		for(i=0;i<vld->height;i++)
		{
			vld->values[k]=computeVertexLightings(p, rec->normal);

			u8 v=abs(vld->values[k-1]-vld->values[k]);
			if(i && hgap1<v)hgap1=v;

			p=addVect(p,v2);
			k++;
		}
		u8 v=abs(vld->values[k-1]-vld->values[k-1-vld->height]);
		if(j && wgap1<v)wgap1=v;
		o=addVect(o,v1);
	}

	if(wgap)*wgap=wgap1;
	if(hgap)*hgap=hgap1;
}

void generateRectangleVertexLighting(room_struct* r, rectangle_struct* rec, vertexLightingData_struct* vld)
{
	if(!r || !rec || !vld)return;

	vld->width=2;
	vld->height=2;

	vld->values=NULL;

	u8 wgap=0, hgap=0;

	do
	{
		generateRectangleVertexLightingWH(r, rec, vld, &wgap, &hgap);

		if(vld->width>=MAXTESSELATION || vld->height>=MAXTESSELATION)break;

		if(wgap>LIGHTGAP)
		{
			if(vld->width<=2)vld->width++;
			else vld->width+=2;
		}
		if(hgap>LIGHTGAP)
		{
			if(vld->height<=2)vld->height++;
			else vld->height+=2;
		}
	}while((hgap>LIGHTGAP || wgap>LIGHTGAP));
}

void generateVertexLighting(room_struct* r, lightingData_struct* ld)
{
	if(!r || !ld)return;
	listCell_struct *lc=r->rectangles.first;
	initLightDataVL(ld, r->rectangles.num);

	int i=0;
	while(lc)
	{
		generateRectangleVertexLighting(r, &lc->data, &ld->data.vertexLighting[i++]);
		lc=lc->next;
	}
}
