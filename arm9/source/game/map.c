#include "game/game_main.h"

room_struct gameRoom;

u32* testDL=NULL;

//DEBUG
bool debugWireframe=false;

void initRectangleList(rectangleList_struct* p)
{
	p->first=NULL;
	p->num=0;
}

rectangle_struct* addRectangle(rectangle_struct r, rectangleList_struct* p)
{
	listCell_struct* pc=(listCell_struct*)malloc(sizeof(listCell_struct));
	pc->next=p->first;
	pc->data=r;
	p->first=pc;
	p->num++;
	return &pc->data;
}

void popRectangle(rectangleList_struct* p)
{
	p->num--;
	if(p->first)
	{
		listCell_struct* pc=p->first;
		p->first=pc->next;
		free(pc);
	}
}

void initRectangle(rectangle_struct* rec, vect3D pos, vect3D size)
{
	s8 sign=1;
	rec->position=pos;
	rec->size=size;
	rec->normal=vect(0,0,0);
	rec->touched=false;
	rec->collides=true;
	if(!size.x)rec->normal.x=inttof32(1);
	else if(!size.y)rec->normal.y=inttof32(-1);
	else rec->normal.z=inttof32(1);
	sign*=(size.x>=0)?1:-1;
	sign*=(size.y>=0)?1:-1;
	sign*=(size.z>=0)?1:-1;
	rec->normal=vect(rec->normal.x*sign,rec->normal.y*sign,rec->normal.z*sign);
	rec->material=NULL;
	// NOGBA("n : %d %d %d",rec->normal.x,rec->normal.y,rec->normal.z);
	rec->hide=false;
}

rectangle_struct createRectangle(vect3D pos, vect3D size, bool portalable)
{
	rectangle_struct rec;
	initRectangle(&rec, pos, size);
	rec.portalable=portalable;
	return rec;
}

vect3D getUnitVect(rectangle_struct* rec)
{
	vect3D u=vect(0,0,0);
	vect3D size=rec->size;	
	if(size.x>0)u.x=(TILESIZE*2)/LIGHTMAPRESOLUTION;
	else if(size.x)u.x=-(TILESIZE*2)/LIGHTMAPRESOLUTION;
	if(size.y>0)u.y=(TILESIZE*2)/LIGHTMAPRESOLUTION;
	else if(size.y)u.y=-(TILESIZE*2)/LIGHTMAPRESOLUTION;
	if(size.z>0)u.z=(TILESIZE*2)/LIGHTMAPRESOLUTION;
	else if(size.z)u.z=-(TILESIZE*2)/LIGHTMAPRESOLUTION;
	return u;
}

bool collideLineMap(room_struct* r, rectangle_struct* rec, vect3D l, vect3D u, int32 d, vect3D* i, vect3D* n)
{
	listCell_struct *lc=r->rectangles.first;
	vect3D v;
	while(lc)
	{
		if(&lc->data!=rec)
		{
			// NOGBA("%p vs %p",rec,&lc->data);
			if(collideLineRectangle(&lc->data,l,u,d,NULL,&v)){if(i)*i=v;if(n)*n=lc->data.normal;return true;}
		}
		lc=lc->next;
	}
	return false;
}

rectangle_struct* collideGridCell(gridCell_struct* gc, rectangle_struct* rec, vect3D l, vect3D u, int32 d, vect3D* i, vect3D* n)
{
	if(!gc)return NULL;
	vect3D v;
	int j;
	for(j=0;j<gc->numRectangles;j++)
	{
		if(gc->rectangles[j]!=rec)
		{
			if(collideLineRectangle(gc->rectangles[j],l,u,d,NULL,&v)){if(i)*i=v;if(n)*n=gc->rectangles[j]->normal;return gc->rectangles[j];}
		}
	}
	return NULL;
}

rectangle_struct* collideLineMapClosest(room_struct* r, rectangle_struct* rec, vect3D l, vect3D u, int32 d, vect3D* i)
{
	if(!r)return NULL;
	listCell_struct *lc=r->rectangles.first;
	vect3D v;
	int32 lowestK=d;
	rectangle_struct* hit=NULL;
	while(lc)
	{
		if(&lc->data!=rec)
		{
			// NOGBA("%p vs %p",rec,&lc->data);
			int32 k;
			// if(collideLineRectangle(&lc->data,l,u,d,&k,&v)){if(k<lowestK){*i=v;lowestK=k;}}
			if(collideLineRectangle(&lc->data,l,u,lowestK,&k,&v)){if(k<lowestK){*i=v;lowestK=k;hit=&lc->data;}}
		}
		lc=lc->next;
	}
	return hit;
}

rectangle_struct* addRoomRectangle(room_struct* r, rectangle_struct rec, material_struct* mat, bool portalable)
{
	if((!rec.size.x && (!rec.size.z || !rec.size.y)) || (!rec.size.y && !rec.size.z))return NULL;
	initRectangle(&rec, rec.position, rec.size);
	if(mat)rec.material=mat;
	else{
		u16 x=rec.position.x;
		u16 y=rec.position.z;
		if(rec.size.x<0)x+=rec.size.x;
		if(rec.size.z<0)y+=rec.size.z;
		if(x>=r->width)x=r->width-1;
		if(y>=r->height)y=r->height-1;
		rec.material=r->materials[x+y*r->width];
	}
	// NOGBA("mat : %d",getMaterialID(rec.material));
	rec.portalable=portalable;
	return addRectangle(rec, &r->rectangles);
}

void removeRectangles(room_struct* r)
{
	while(r->rectangles.num)popRectangle(&r->rectangles);
}

vect3D convertNormal(vect3D n, int32 v)
{
	if(n.x>0)n.x=v;
	else if(n.x<0)n.x=-v;
	if(n.y>0)n.y=v;
	else if(n.y<0)n.y=-v;
	if(n.z>0)n.z=v;
	else if(n.z<0)n.z=-v;
	return n;
}

vect3D getOverlayRectanglePosition(rectangle_struct rec)
{
	int32 v=LIGHTCONST;
	vect3D u=addVect(convertVect(rec.position),convertNormal(rec.normal,-LIGHTCONST));
	if(rec.size.x>0)u.x-=v;
	else if(rec.size.x)u.x+=v;
	if(rec.size.y>0)u.y-=v;
	else if(rec.size.y)u.y+=v;
	if(rec.size.z>0)u.z-=v;
	else if(rec.size.z)u.z+=v;
	return u;
}

vect3D getOverlayRectangleSize(rectangle_struct rec)
{
	int32 v=LIGHTCONST;
	vect3D u=convertSize(rec.size);
	if(u.x>0)u.x+=2*v;
	else if(u.x)u.x-=2*v;
	if(u.y>0)u.y+=2*v;
	else if(u.y)u.y-=2*v;
	if(u.z>0)u.z+=2*v;
	else if(u.z)u.z-=2*v;
	return u;
}

void drawRect(rectangle_struct rec, vect3D pos, vect3D size, bool c) //TEMP ? (clean up, switch to v10 ?)
{
	if(rec.hide)return;

	vect3D v[4];
	pos=vectMultInt(pos,32);
	size=vectMultInt(size,32);
	if(!rec.size.x)
	{
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y, pos.z+size.z);
		v[2]=vect(pos.x, pos.y+size.y, pos.z+size.z);
		v[3]=vect(pos.x, pos.y+size.y, pos.z);
	}else if(rec.size.y){
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y+size.y, pos.z);
		v[2]=vect(pos.x+size.x, pos.y+size.y, pos.z);
		v[3]=vect(pos.x+size.x, pos.y, pos.z);
	}else{
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y, pos.z+size.z);
		v[2]=vect(pos.x+size.x, pos.y, pos.z+size.z);
		v[3]=vect(pos.x+size.x, pos.y, pos.z);
	}

	vect3D vt[4];
	int32 t[4];
	if(c)
	{
		GFX_COLOR=RGB15(31,31,31);		
		bindMaterial(rec.material,&rec,t,vt,false);
	}else if(rec.lightData.lightMap){
		vect3D lmPos=rec.lightData.lightMap->lmPos;
		vect3D lmSize=rec.lightData.lightMap->lmSize;
		vect3D p1=vect(inttot16(lmPos.x),inttot16(lmPos.y),0);
		vect3D p2=vect(inttot16(lmPos.x+lmSize.x-1),inttot16(lmPos.y+lmSize.y-1),0);
		if(rec.lightData.lightMap->rot)
		{
			p2=vect(inttot16(lmPos.x+lmSize.y-1),inttot16(lmPos.y+lmSize.x-1),0);
			t[0]=TEXTURE_PACK(p1.x, p1.y);
			t[3]=TEXTURE_PACK(p1.x, p2.y);
			t[2]=TEXTURE_PACK(p2.x, p2.y);
			t[1]=TEXTURE_PACK(p2.x, p1.y);
		}else{
			t[0]=TEXTURE_PACK(p1.x, p1.y);
			t[1]=TEXTURE_PACK(p1.x, p2.y);
			t[2]=TEXTURE_PACK(p2.x, p2.y);
			t[3]=TEXTURE_PACK(p2.x, p1.y);
		}
	}else return;

	if(rec.lightData.vertex)
	{
		u8* vc=rec.lightData.vertex->values;
		const u8 num=rec.lightData.vertex->width*rec.lightData.vertex->height;

		if(num>4)
		{
			vertexLightingData_struct* vld=rec.lightData.vertex;
			vect3D o=vect(pos.x,pos.y,pos.z);
			vect3D v1=vect(size.x/(vld->width-1),0,0);
			vect3D v2=vect(0,0,size.z/(vld->height-1));

			if(!size.x)v1=vect(0,size.y/(vld->width-1),0);
			if(!size.z)v2=vect(0,size.y/(vld->height-1),0);

			vect3D to=vt[0];
			vect3D vt1=vectDivInt(vectDifference(vt[1],vt[0]),vld->height-1);
			vect3D vt2=vectDivInt(vectDifference(vt[3],vt[0]),vld->width-1);

			// NOGBA("%d",vld->height);

			vect3D p=o;
			vect3D tp=to;
			int i, j;
			int k=0;
			for(j=0;j<rec.lightData.vertex->width-1;j++)
			{
				p=o;
				tp=to;
				glBegin(GL_QUAD_STRIP);
				for(i=0;i<rec.lightData.vertex->height;i++)
				{
					// vc=&rec.lightData.vertex->values[(k++)+rec.lightData.vertex->height];
					// u8 vb=computeVertexLightings(convertVect(vectDivInt(vect(p.x+v1.x,p.y+v1.y,p.z+v1.z),32)), rec.normal);
					u8 vb=rec.lightData.vertex->values[k+rec.lightData.vertex->height];
					if(!debugWireframe)GFX_COLOR=RGB15(vb,vb,vb);
					GFX_TEX_COORD=TEXTURE_PACK(tp.x+vt2.x,tp.y+vt2.y);
					glVertex3v16(p.x+v1.x,p.y+v1.y,p.z+v1.z);
					// vc=&rec.lightData.vertex->values[k];
					// vb=computeVertexLightings(convertVect(vectDivInt(vect(p.x,p.y,p.z),32)), rec.normal);
					vb=rec.lightData.vertex->values[k++];
					if(!debugWireframe)GFX_COLOR=RGB15(vb,vb,vb);
					GFX_TEX_COORD=TEXTURE_PACK(tp.x,tp.y);
					glVertex3v16(p.x,p.y,p.z);

					p=addVect(p,v2);
					tp=addVect(tp,vt1);
				}
				o=addVect(o,v1);
				to=addVect(to,vt2);
			}
			// NOGBA("%d %d vs %d %d",tp.x,tp.y,vt[3].x,vt[3].y);
		}else{
			glBegin(GL_QUAD);
				GFX_TEX_COORD = t[0];
				if(!debugWireframe)GFX_COLOR=RGB15(*vc,*vc,*vc);
				glVertex3v16(v[0].x,v[0].y,v[0].z);
		
				GFX_TEX_COORD = t[1];
				vc=&rec.lightData.vertex->values[1];
				if(!debugWireframe)GFX_COLOR=RGB15(*vc,*vc,*vc);
				glVertex3v16(v[1].x,v[1].y,v[1].z);

				GFX_TEX_COORD = t[2];
				vc=&rec.lightData.vertex->values[3];
				if(!debugWireframe)GFX_COLOR=RGB15(*vc,*vc,*vc);
				glVertex3v16(v[2].x,v[2].y,v[2].z);
		
				GFX_TEX_COORD = t[3];
				vc=&rec.lightData.vertex->values[2];
				if(!debugWireframe)GFX_COLOR=RGB15(*vc,*vc,*vc);
				glVertex3v16(v[3].x,v[3].y,v[3].z);
		}
	}else{
		glBegin(GL_QUAD);
			GFX_TEX_COORD = t[0];
			glVertex3v16(v[0].x,v[0].y,v[0].z);
	
			GFX_TEX_COORD = t[1];
			glVertex3v16(v[1].x,v[1].y,v[1].z);
	
			GFX_TEX_COORD = t[2];
			glVertex3v16(v[2].x,v[2].y,v[2].z);
	
			GFX_TEX_COORD = t[3];
			glVertex3v16(v[3].x,v[3].y,v[3].z);
	}
}

void drawRectDL(rectangle_struct rec, vect3D pos, vect3D size, bool c, vect3D cpos, vect3D cnormal, bool cull) //TEMP ?
{
	if(rec.hide)return;
	
	vect3D v[4];
	vect3D rpos=pos, rsize=size;
	pos=convertVect(pos);
	size=convertSize(size);
	if(!rec.size.x)
	{
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y, pos.z+size.z);
		v[2]=vect(pos.x, pos.y+size.y, pos.z+size.z);
		v[3]=vect(pos.x, pos.y+size.y, pos.z);
	}else if(rec.size.y){
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y+size.y, pos.z);
		v[2]=vect(pos.x+size.x, pos.y+size.y, pos.z);
		v[3]=vect(pos.x+size.x, pos.y, pos.z);
	}else{
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y, pos.z+size.z);
		v[2]=vect(pos.x+size.x, pos.y, pos.z+size.z);
		v[3]=vect(pos.x+size.x, pos.y, pos.z);
	}
	
	if(cull)
	{
		int i;bool pass=false;
		for(i=0;i<4;i++)
		{
			vect3D v1=vectDifference(v[i],cpos);
			if(dotProduct(v1,cnormal)>0)pass=true;
		}
		if(!pass)return;
	}
	
	vect3D vt[4];
	int32 t[4];
	if(c)
	{
		glColorDL(RGB15(31,31,31));
		bindMaterial(rec.material,&rec,t,vt,true);
	}else if(rec.lightData.lightMap){
		vect3D lmPos=rec.lightData.lightMap->lmPos;
		vect3D lmSize=rec.lightData.lightMap->lmSize;
		vect3D p1=vect(inttot16(lmPos.x),inttot16(lmPos.y),0);
		vect3D p2=vect(inttot16(lmPos.x+lmSize.x-1),inttot16(lmPos.y+lmSize.y-1),0);
		if(rec.lightData.lightMap->rot)
		{
			p2=vect(inttot16(lmPos.x+lmSize.y-1),inttot16(lmPos.y+lmSize.x-1),0);
			t[0]=TEXTURE_PACK(p1.x, p1.y);
			t[3]=TEXTURE_PACK(p1.x, p2.y);
			t[2]=TEXTURE_PACK(p2.x, p2.y);
			t[1]=TEXTURE_PACK(p2.x, p1.y);
		}else{
			t[0]=TEXTURE_PACK(p1.x, p1.y);
			t[1]=TEXTURE_PACK(p1.x, p2.y);
			t[2]=TEXTURE_PACK(p2.x, p2.y);
			t[3]=TEXTURE_PACK(p2.x, p1.y);
		}
	}else return;

	pos=vectMultInt(rpos,32);
	size=vectMultInt(rsize,32);
	if(!rec.size.x)
	{
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y, pos.z+size.z);
		v[2]=vect(pos.x, pos.y+size.y, pos.z+size.z);
		v[3]=vect(pos.x, pos.y+size.y, pos.z);
	}else if(rec.size.y){
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y+size.y, pos.z);
		v[2]=vect(pos.x+size.x, pos.y+size.y, pos.z);
		v[3]=vect(pos.x+size.x, pos.y, pos.z);
	}else{
		v[0]=vect(pos.x, pos.y, pos.z);
		v[1]=vect(pos.x, pos.y, pos.z+size.z);
		v[2]=vect(pos.x+size.x, pos.y, pos.z+size.z);
		v[3]=vect(pos.x+size.x, pos.y, pos.z);
	}

	if(rec.lightData.vertex)
	{
		u8* vc=rec.lightData.vertex->values;
		const u8 num=rec.lightData.vertex->width*rec.lightData.vertex->height;

		if(num>4)
		{
			vertexLightingData_struct* vld=rec.lightData.vertex;
			vect3D o=vect(pos.x,pos.y,pos.z);
			vect3D v1=vect(size.x/(vld->width-1),0,0);
			vect3D v2=vect(0,0,size.z/(vld->height-1));

			if(!size.x)v1=vect(0,size.y/(vld->width-1),0);
			if(!size.z)v2=vect(0,size.y/(vld->height-1),0);

			vect3D to=vt[0];
			vect3D vt1=vectDivInt(vectDifference(vt[1],vt[0]),vld->height-1);
			vect3D vt2=vectDivInt(vectDifference(vt[3],vt[0]),vld->width-1);

			// NOGBA("%d",vld->height);

			vect3D p=o;
			vect3D tp=to;
			int i, j;
			int k=0;
			for(j=0;j<rec.lightData.vertex->width-1;j++)
			{
				p=o;
				tp=to;
				glBeginDL(GL_QUAD_STRIP);
				for(i=0;i<rec.lightData.vertex->height;i++)
				{
					u8 vb=rec.lightData.vertex->values[k+rec.lightData.vertex->height];
					if(!debugWireframe)glColorDL(RGB15(vb,vb,vb));
					glTexCoordPACKED(TEXTURE_PACK(tp.x+vt2.x,tp.y+vt2.y));
					glVertex3v16DL(p.x+v1.x,p.y+v1.y,p.z+v1.z);

					vb=rec.lightData.vertex->values[k++];
					if(!debugWireframe)glColorDL(RGB15(vb,vb,vb));
					glTexCoordPACKED(TEXTURE_PACK(tp.x,tp.y));
					glVertex3v16DL(p.x,p.y,p.z);

					p=addVect(p,v2);
					tp=addVect(tp,vt1);
				}
				o=addVect(o,v1);
				to=addVect(to,vt2);
			}
			// NOGBA("%d %d vs %d %d",tp.x,tp.y,vt[3].x,vt[3].y);
		}else{
			glBeginDL(GL_QUAD);
				glTexCoordPACKED(t[0]);
				if(!debugWireframe)glColorDL(RGB15(*vc,*vc,*vc));
				glVertex3v16DL(v[0].x,v[0].y,v[0].z);
		
				glTexCoordPACKED(t[1]);
				vc=&rec.lightData.vertex->values[1];
				if(!debugWireframe)glColorDL(RGB15(*vc,*vc,*vc));
				glVertex3v16DL(v[1].x,v[1].y,v[1].z);

				glTexCoordPACKED(t[2]);
				vc=&rec.lightData.vertex->values[3];
				if(!debugWireframe)glColorDL(RGB15(*vc,*vc,*vc));
				glVertex3v16DL(v[2].x,v[2].y,v[2].z);
		
				glTexCoordPACKED(t[3]);
				vc=&rec.lightData.vertex->values[2];
				if(!debugWireframe)glColorDL(RGB15(*vc,*vc,*vc));
				glVertex3v16DL(v[3].x,v[3].y,v[3].z);
		}
	}else{
		glBeginDL(GL_QUAD);
			glTexCoordPACKED(t[0]);
			glVertex3v16DL(v[0].x, v[0].y, v[0].z);

			glTexCoordPACKED(t[1]);
			glVertex3v16DL(v[1].x, v[1].y, v[1].z);

			glTexCoordPACKED(t[2]);
			glVertex3v16DL(v[2].x, v[2].y, v[2].z);

			glTexCoordPACKED(t[3]);
			glVertex3v16DL(v[3].x, v[3].y, v[3].z);
	}
}

s16 transferRectangle(vect3D pos, vect3D size, vect3D normal)
{
	if(size.x<0){pos.x+=size.x;size.x=-size.x;}
	if(size.y<0){pos.y+=size.y;size.y=-size.y;}
	if(size.z<0){pos.z+=size.z;size.z=-size.z;}
	return createAAR(vectMultInt(size,4), vectMultInt(pos,4), vectMultInt(normal,-1));
}

void transferRectangles(room_struct* r)
{
	listCell_struct *lc=r->rectangles.first;
	int i=0;
	while(lc)
	{
		lc->data.AARid=transferRectangle(addVect(convertSize(vect(r->position.x,0,r->position.y)),convertVect(lc->data.position)),convertSize(lc->data.size),lc->data.normal);
		lc=lc->next;
		i++;
		if(!(i%8))swiWaitForVBlank();
	}
}

void translateRectangles(room_struct* r, vect3D v)
{
	listCell_struct *lc=r->rectangles.first;
	while(lc)
	{
		lc->data.position=addVect(lc->data.position,v);
		lc=lc->next;
	}
}

void drawRectangleList(rectangleList_struct* rl)
{
	if(!rl)return;

	listCell_struct *lc=rl->first;
	unbindMtl();
	GFX_COLOR=RGB15(31,31,31);

	glPushMatrix();
		glTranslate3f32(-TILESIZE,0,-TILESIZE);
		glScalef32((TILESIZE*2)<<7,(HEIGHTUNIT)<<7,(TILESIZE*2)<<7);
		while(lc)
		{
			drawRect(lc->data,(lc->data.position),(lc->data.size),true);
			lc=lc->next;
		}
	glPopMatrix(1);
}

void drawRectangles(room_struct* r, u8 mode, u16 color)
{
	// glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	if(keysDown() & KEY_X)debugWireframe^=1;
	if(debugWireframe)glPolyFmt(POLY_ALPHA(0) | POLY_CULL_BACK); //DEBUG
	else glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	
	drawRectangleList(&r->rectangles);
	if(mode&6 && r->lightingData.type==LIGHTMAP_DATA)
	{
		listCell_struct *lc=r->rectangles.first;
		glPolyFmt(POLY_ALPHA(31) | (1<<14) | POLY_CULL_BACK);
		applyMTL(r->lightingData.data.lightMap.texture);
		GFX_COLOR=RGB15(31,31,31);
		glPushMatrix();
			glTranslate3f32(-TILESIZE,0,-TILESIZE);
			glScalef32((TILESIZE*2)<<7,(HEIGHTUNIT)<<7,(TILESIZE*2)<<7);
			while(lc)
			{
				drawRect(lc->data,(lc->data.position),(lc->data.size),false);
				lc=lc->next;
			}
		glPopMatrix(1);
	}
	if(mode&128 && color)
	{
		listCell_struct *lc=r->rectangles.first;
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_FRONT);
		unbindMtl();
		GFX_COLOR=color;
		glPushMatrix();
			glTranslate3f32(-TILESIZE,0,-TILESIZE);
			glScalef32((TILESIZE*2)<<7,(HEIGHTUNIT)<<7,(TILESIZE*2)<<7);
			while(lc)
			{
				drawRect(lc->data,(lc->data.position),(lc->data.size),false);
				lc=lc->next;
			}
		glPopMatrix(1);
	}
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
}

void initRoomGrid(room_struct* r)
{
	if(!r || !r->width || !r->height)return;
	
	r->rectangleGridSize.x=r->width/CELLSIZE+1;
	r->rectangleGridSize.y=r->height/CELLSIZE+1;
	
	r->rectangleGrid=malloc(sizeof(gridCell_struct)*r->rectangleGridSize.x*r->rectangleGridSize.y);
	
	int i;
	for(i=0;i<r->rectangleGridSize.x*r->rectangleGridSize.y;i++)
	{
		r->rectangleGrid[i].rectangles=NULL;
	}
}

void initRoom(room_struct* r, u16 w, u16 h, vect3D p)
{
	if(!r)return;
	
	r->width=w;
	r->height=h;
	r->position=p;
	
	initRectangleList(&r->rectangles);
	
	if(r->height && r->width)
	{
		r->materials=malloc(r->height*r->width*sizeof(material_struct*));
		int i;for(i=0;i<r->height*r->width;i++){r->materials[i]=NULL;}
	}else r->materials=NULL;

	initLightData(&r->lightingData);
	r->rectangleGrid=NULL;
		
	initRoomGrid(r);
}

gridCell_struct* getCurrentCell(room_struct* r, vect3D o)
{
	if(!r)return NULL;
	
	o=vectDifference(o,convertSize(vect(r->position.x,0,r->position.y)));
	o=vectDivInt(o,CELLSIZE*TILESIZE*2);
	
	if(o.x>=0 && o.x<r->rectangleGridSize.x && o.z>=0 && o.z<r->rectangleGridSize.y)return &r->rectangleGrid[o.x+o.z*r->rectangleGridSize.x];
	return NULL;
}

void drawCell(gridCell_struct* gc) //debug
{
	if(!gc)return;
	
	room_struct* r=getPlayer()->currentRoom;
	if(!r)return;
	glPushMatrix();
	glTranslate3f32(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	int i;
	for(i=0;i<gc->numRectangles;i++)
	{
		drawRect(*gc->rectangles[i],convertVect(gc->rectangles[i]->position),convertSize(gc->rectangles[i]->size),true);
	}
	glPopMatrix(1);
}

void generateGridCell(room_struct* r, gridCell_struct* gc, u16 x, u16 y)
{
	if(!r || !gc)return;
	
	if(gc->rectangles)free(gc->rectangles);
	gc->numRectangles=0;
	gc->lights[0]=NULL;gc->lights[1]=NULL;gc->lights[2]=NULL;
	
	x*=CELLSIZE*2;y*=CELLSIZE*2; //so getting the center isn't a problem
	x+=CELLSIZE;y+=CELLSIZE; //center
	
	listCell_struct *lc=r->rectangles.first;
	while(lc)
	{
		if((abs(x-(lc->data.position.x*2+lc->data.size.x))<=(CELLSIZE+abs(lc->data.size.x))) && (abs(y-(lc->data.position.z*2+lc->data.size.z))<=(CELLSIZE+abs(lc->data.size.z))))
		{
			gc->numRectangles++;
		}
		lc=lc->next;
	}
	gc->rectangles=malloc(sizeof(rectangle_struct*)*gc->numRectangles);
	gc->numRectangles=0;
	lc=r->rectangles.first;
	while(lc)
	{
		if((abs(x-(lc->data.position.x*2+lc->data.size.x))<=(CELLSIZE+abs(lc->data.size.x))) && (abs(y-(lc->data.position.z*2+lc->data.size.z))<=(CELLSIZE+abs(lc->data.size.z))))
		{
			gc->rectangles[gc->numRectangles++]=&lc->data;
		}
		lc=lc->next;
	}
	
	getClosestLights(vect(x/2,0,y/2), &gc->lights[0], &gc->lights[1], &gc->lights[2], &gc->lightDistances[0], &gc->lightDistances[1], &gc->lightDistances[2]);
}

void generateRoomGrid(room_struct* r)
{
	if(!r)return;
	
	int i, j;
	for(i=0;i<r->rectangleGridSize.x;i++)
	{
		for(j=0;j<r->rectangleGridSize.y;j++)
		{
			generateGridCell(r,&r->rectangleGrid[i+j*r->rectangleGridSize.x],i,j);
		}
	}
}

void drawRoom(room_struct* r, u8 mode, u16 color) //obviously temp
{
	if(!r)return;
	glPushMatrix();
		glTranslate3f32(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y);
		if(mode&1)drawRectangles(r, mode, color);
	glPopMatrix(1);
}

u32* generateRoomDisplayList(room_struct* r, vect3D pos, vect3D normal, bool cull)
{	
	if(!r)r=getPlayer()->currentRoom;
	if(!r)return NULL;
	u32* ptr=glBeginListDL();
	
	glPolyFmtDL(POLY_ALPHA(31) | POLY_CULL_BACK);
	listCell_struct *lc=r->rectangles.first;
	while(lc)
	{
		drawRectDL(lc->data,(lc->data.position),(lc->data.size),true,vectDifference(pos,vect(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y)),normal,cull);
		lc=lc->next;
	}
	
	if(r->lightingData.type==LIGHTMAP_DATA)
	{
		lc=r->rectangles.first;
		glPolyFmtDL(POLY_ALPHA(31) | (1<<14) | POLY_CULL_BACK);
		applyMTLDL(r->lightingData.data.lightMap.texture);
		while(lc)
		{
			drawRectDL(lc->data,(lc->data.position),(lc->data.size),false,vectDifference(pos,vect(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y)),normal,cull);
			lc=lc->next;
		}
	}

	u32 size=glEndListDL();
	u32* displayList=malloc((size+1)*4);
	if(displayList)memcpy(displayList,ptr,(size+1)*4);
	return displayList;
}

vect3D getVector(vect3D pos, light_struct* l, int32* d)
{
	room_struct* r=&gameRoom;
	vect3D p=l->position;
	vect3D v=vectDifference(pos,vect((r->position.x+p.x)*(TILESIZE*2),p.y*HEIGHTUNIT,(r->position.y+p.z)*(TILESIZE*2)));
	v=vectMultInt(v,8);
	int32 dist=magnitude(v);
	if(d)*d=dist/8;
	v=divideVect(v,dist);
	return vect(f32tov10(v.x),f32tov10(v.y),f32tov10(v.z));
}

void setupObjectLighting(room_struct* r, vect3D pos, u32* params)
{
	if(!r)r=getPlayer()->currentRoom;
	if(!r)return;
	light_struct *l1, *l2, *l3;
	// vect3D tilepos=reverseConvertVect(vectDifference(pos,convertVect(vect(r->position.x,0,r->position.y))));
	// getClosestLights(r->entityCollection, tilepos, &l1, &l2, &l3, &d1, &d2, &d3);
	gridCell_struct* gc=getCurrentCell(r, pos);
	if(!gc)return;
	l1=gc->lights[0];
	l2=gc->lights[1];
	l3=gc->lights[2];
	// *params=POLY_ALPHA(31) | POLY_CULL_FRONT;
	
	glMaterialf(GL_AMBIENT, RGB15(5,5,5));
	glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
	glMaterialf(GL_SPECULAR, RGB15(0,0,0));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));
	
	if(l1)
	{
		*params|=POLY_FORMAT_LIGHT0;
		int32 d;
		vect3D v=getVector(pos, l1, &d);
		int32 v2=31-((31*d)/(l1->intensity));
		glLight(0, RGB15(v2,v2,v2), v.x, v.y, v.z);
		if(l2)
		{
			*params|=POLY_FORMAT_LIGHT1;
			vect3D v=getVector(pos, l2, &d);
			int32 v2=31-((31*d)/(l2->intensity));
			glLight(1, RGB15(v2,v2,v2), v.x, v.y, v.z);
			if(l3)
			{
				*params|=POLY_FORMAT_LIGHT2;
				vect3D v=getVector(pos, l3, &d);
				int32 v2=31-((31*d)/(l3->intensity));
				glLight(2, RGB15(v2,v2,v2), v.x, v.y, v.z);
			}
		}
	}
}

void freeRoom(room_struct* r)
{
	if(r)
	{
		if(r->materials)free(r->materials);
		if(r->rectangleGrid)
		{
			int i;
			for(i=0;i<r->rectangleGridSize.x*r->rectangleGridSize.y;i++)
			{
				if(r->rectangleGrid[i].rectangles)free(r->rectangleGrid[i].rectangles);
			}
			free(r->rectangleGrid);
			r->rectangleGrid=NULL;
		}
		r->materials=NULL;
		freeLightData(&r->lightingData);
		removeRectangles(r);
	}
}
