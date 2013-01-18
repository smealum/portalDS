/*
 * md2.c -- md2 model loader
 * last modification: aug. 14, 2007
 *
 * Copyright (c) 2005-2007 David HENRY
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * gcc -Wall -ansi -lGL -lGLU -lglut md2.c -o md2
 
 (modified for fixed point and probably other stuff by smealum, 2012)
 */

 #include "common/general.h"
 
 #define getInterpData(i,j,n) interpTable[(i)+(((j)+((n)<<2))<<8)]

/* Table of precalculated normals */
u32 anorms_table[162] =
 #include "common/anorms.h"
;
vect3D anorms_table2[162] =
 #include "common/anorms2.h"
;

void packFrameData(md2Model_struct *mdl, md2_frame_t* f)
{
	int i;
	f->min=vect(f->verts[0].v[0],f->verts[0].v[1],f->verts[0].v[2]);
	f->max=vect(f->verts[0].v[0],f->verts[0].v[1],f->verts[0].v[2]);
	for(i=0;i<mdl->header.num_vertices;i++)
	{
	    md2_vertex_t* pvert = &f->verts[i];
		// vect3D scale=f->scale;
		// f->packedVerts[i]=vect(((scale.x*pvert->v[0])/8),((scale.y*pvert->v[1])/8),((scale.z*pvert->v[2])/8));
		f->packedv10[i]=NORMAL_PACK(pvert->v[0],pvert->v[1],pvert->v[2]);
		
		if(pvert->v[0]<f->min.x)f->min.x=pvert->v[0];
		else if(pvert->v[0]>f->max.x)f->max.x=pvert->v[0];
		if(pvert->v[1]<f->min.y)f->min.y=pvert->v[1];
		else if(pvert->v[1]>f->max.y)f->max.y=pvert->v[1];
		if(pvert->v[2]<f->min.z)f->min.z=pvert->v[2];
		else if(pvert->v[2]>f->max.z)f->max.z=pvert->v[2];
	}
	f->min=vect(mulf32(f->min.x,f->scale.x)*128*32,mulf32(f->min.y,f->scale.y)*128*32,mulf32(f->min.z,f->scale.z)*128*32);
	f->max=vect(mulf32(f->max.x,f->scale.x)*128*32,mulf32(f->max.y,f->scale.y)*128*32,mulf32(f->max.z,f->scale.z)*128*32);
	f->min=addVect(f->min,f->translate);
	f->max=addVect(f->max,f->translate);
}

void packTexcoordData(md2Model_struct *mdl)
{
	int i;
	for(i=0;i<mdl->header.num_st;i++)
	{
		mdl->packedTexcoords[i]=TEXTURE_PACK(inttot16(mdl->texcoords[i].s),inttot16(mdl->texcoords[i].t));
	}
}

void trimName(char* name) //destructive
{
	int i=0;
	while(name[i]){if(name[i]>='0' && name[i]<='9'){name[i]=0;return;}i++;}
}

void getAnimations(md2Model_struct *mdl)
{
	if(!mdl || !mdl->frames)return;
	int i;
	char* oldstr=NULL;
	int n=0;
	for(i=0;i<mdl->header.num_frames;i++)
	{
		md2_frame_t *pframe=&mdl->frames[i];
		trimName(pframe->name);
		if(strcmp(pframe->name,oldstr))n++;
		oldstr=pframe->name;
	}
	mdl->numAnim=n;
	mdl->animations=malloc(sizeof(md2Anim_struct)*mdl->numAnim);
	n=0;
	mdl->animations[0].start=0;
	oldstr=mdl->frames[0].name;
	for(i=0;i<mdl->header.num_frames;i++)
	{
		md2_frame_t *pframe=&mdl->frames[i];
		if(strcmp(pframe->name,oldstr))
		{
			mdl->animations[n++].end=i-1;
			mdl->animations[n].start=i;
		}
		oldstr=pframe->name;
	}
	mdl->animations[n].end=i-1;
}

int loadMd2Model(const char *filename, char *texname, md2Model_struct *mdl)
{
	FILE *fp;
	int i;
	
	mdl->texture=createTexture(texname,"textures");

	fp = fopen (filename, "rb");
	if (!fp)
	{
		fprintf (stderr, "Error: couldn't open \"%s\"!\n", filename);
		return 0;
	}

	/* Read header */
	fread (&mdl->header, 1, sizeof (md2_header_t), fp);

	if ((mdl->header.ident != 844121161) || (mdl->header.version != 8))
	{
		/* Error! */
		fprintf (stderr, "Error: bad version or identifier\n");
		fclose (fp);
		return 0;
	}

	/* Memory allocations */
	mdl->skins = (md2_skin_t *)malloc (sizeof (md2_skin_t) * mdl->header.num_skins);
	mdl->texcoords = (md2_texCoord_t *)malloc (sizeof (md2_texCoord_t) * mdl->header.num_st);
	mdl->triangles = (md2_triangle_t *)malloc (sizeof (md2_triangle_t) * mdl->header.num_tris);
	mdl->frames = (md2_frame_t *)malloc (sizeof (md2_frame_t) * mdl->header.num_frames);

	/* Read model data */
	fseek (fp, mdl->header.offset_skins, SEEK_SET);
	fread (mdl->skins, sizeof (md2_skin_t), mdl->header.num_skins, fp);

	fseek (fp, mdl->header.offset_st, SEEK_SET);
	fread (mdl->texcoords, sizeof (md2_texCoord_t), mdl->header.num_st, fp);
	
	mdl->packedTexcoords=malloc(sizeof(u32)*mdl->header.num_st);
	packTexcoordData(mdl);

	fseek (fp, mdl->header.offset_tris, SEEK_SET);
	fread (mdl->triangles, sizeof (md2_triangle_t), mdl->header.num_tris, fp);

	/* Read frames */
	fseek (fp, mdl->header.offset_frames, SEEK_SET);
	for (i = 0; i < mdl->header.num_frames; ++i)
	{
		/* Memory allocation for vertices of this frame */
		mdl->frames[i].verts = (md2_vertex_t *) malloc (sizeof (md2_vertex_t) * mdl->header.num_vertices);
		mdl->frames[i].packedv10 = (u32 *) malloc (sizeof (u32) * mdl->header.num_vertices);
		mdl->frames[i].displayList = NULL;
		// mdl->frames[i].faceNormals = (vect3D *) malloc (sizeof (vect3D) * mdl->header.num_tris);		
		
	    vec3_t scale,trans;
		
		fread (scale, sizeof (vec3_t), 1, fp);
		fread (trans, sizeof (vec3_t), 1, fp);
		
		mdl->frames[i].scale=vect(floattof32(scale[0]),floattof32(scale[1]),floattof32(scale[2]));
		mdl->frames[i].translate=vect(floattof32(trans[0]),floattof32(trans[1]),floattof32(trans[2]));
		
		fread (mdl->frames[i].name, sizeof (char), 16, fp);
		fread (mdl->frames[i].verts, sizeof (md2_vertex_t), mdl->header.num_vertices, fp);
		
		packFrameData(mdl,&mdl->frames[i]);
	}
	
	getAnimations(mdl);

	fclose(fp);
	return 1;
}

void freeMd2Model(md2Model_struct *mdl)
{
	int i;

	if (mdl->skins)
	{
		free (mdl->skins);
		mdl->skins = NULL;
	}

	if (mdl->texcoords)
	{
		free (mdl->texcoords);
		free (mdl->packedTexcoords);
		mdl->texcoords = NULL;
		mdl->packedTexcoords = NULL;
	}

	if (mdl->triangles)
	{
		free (mdl->triangles);
		mdl->triangles = NULL;
	}

	if (mdl->frames)
	{
		for (i = 0; i < mdl->header.num_frames; ++i)
		{
			free(mdl->frames[i].verts);
			// free (mdl->frames[i].packedVerts);
			free(mdl->frames[i].packedv10);
			if(mdl->frames[i].displayList)free(mdl->frames[i].displayList);
			mdl->frames[i].verts = NULL;
			mdl->frames[i].packedVerts = NULL;
			mdl->frames[i].packedv10 = NULL;
		}

		free (mdl->frames);
		mdl->frames = NULL;
	}
	
	if(mdl->animations)free(mdl->animations);
	mdl->animations=NULL;
}

void generateFrameDisplayList(int n, const md2Model_struct *mdl)
{
	int i, j;
	if(!mdl)return;
	if ((n < 0) || (n > mdl->header.num_frames - 1))return;

	md2_frame_t *pframe=&mdl->frames[n];
	
	u32* ptr=glBeginListDL();
	glBeginDL(GL_TRIANGLES);
	for (i = 0; i < mdl->header.num_tris; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			glTexCoordPACKED(mdl->packedTexcoords[mdl->triangles[i].st[j]]);
			// glNormalDL(anorms_table[pframe->verts[mdl->triangles[i].vertex[j]].normalIndex]);
			// glVertexPackedDL(pframe->packedv10[mdl->triangles[i].vertex[j]]);
			glVertexPackedDL(NORMAL_PACK(pframe->verts[mdl->triangles[i].vertex[j]].v[0]*2,pframe->verts[mdl->triangles[i].vertex[j]].v[1]*2,pframe->verts[mdl->triangles[i].vertex[j]].v[2]*2));
		}
	}
	// glEndDL();
	u32 size=glEndListDL();
	pframe->displayList=malloc((size+1)*4);
	if(pframe->displayList)memcpy(pframe->displayList,ptr,(size+1)*4);
}

void generateModelDisplayLists(md2Model_struct *mdl)
{
	if(!mdl)return;
	
	int i;
	for(i=0;i<mdl->header.num_tris;i++)
	{
		generateFrameDisplayList(i, mdl);
	}
}

void renderModelFrame(int n, const md2Model_struct *mdl)
{
	int i, j;
	
	glPolyFmt(POLY_ALPHA(31) | POLY_FORMAT_LIGHT0 | POLY_CULL_FRONT);

	n%=mdl->header.num_frames;
	if ((n < 0) || (n > mdl->header.num_frames - 1))return;

	md2_frame_t *pframe=&mdl->frames[n];
		
	applyMTL(mdl->texture);

	glPushMatrix();
	
	glRotateXi(-(1<<13));
	
	// vect3D u=vect(inttof32(1),0,0);
	
	glTranslate3f32(pframe->translate.x,pframe->translate.y,pframe->translate.z);
	glScalef32((pframe->scale.x),(pframe->scale.y),(pframe->scale.z));
	glScalef32(inttof32(64),inttof32(64),inttof32(64)); // necessary for v10
	glBegin (GL_TRIANGLES);
	GFX_COLOR=RGB15(31,31,31);
	for (i = 0; i < mdl->header.num_tris; ++i)
	{
		// if(fakeDotProduct(anorms_table2[pframe->verts[mdl->triangles[i].vertex[0]].normalIndex],u)>0)
		{
			for (j = 0; j < 3; ++j)
			{
				GFX_TEX_COORD=mdl->packedTexcoords[mdl->triangles[i].st[j]];

				GFX_NORMAL=anorms_table[pframe->verts[mdl->triangles[i].vertex[j]].normalIndex];

				//v16
				// vect3D v=pframe->packedVerts[mdl->triangles[i].vertex[j]];
				// glVertex3v16(v.x,v.y,v.z);
				
				//v10
				GFX_VERTEX10=pframe->packedv10[mdl->triangles[i].vertex[j]];
			}
		}
	}
	glEnd();
	glPopMatrix(1);
}

void renderModelFrameInterp(int n, int n2, int m, const md2Model_struct *mdl, u32 params, bool center, u32* pal)
{
	int i, j;
	
	glPolyFmt(params);

	n%=mdl->header.num_frames;
	n2%=mdl->header.num_frames;
	
	if ((n < 0) || (n > mdl->header.num_frames - 1))return;

	md2_frame_t *pframe=&mdl->frames[n];
	md2_frame_t *pframe2=&mdl->frames[n2];
		
	applyMTL(mdl->texture);
	if(pal)bindPaletteAddr(pal);

	glPushMatrix();
	
		glRotateXi(-(1<<13));

		glScalef32(1<<5,1<<5,1<<5); //?
		
		glTranslate3f32(pframe->translate.x+((pframe2->translate.x-pframe->translate.x)*m)/4,pframe->translate.y+((pframe2->translate.y-pframe->translate.y)*m)/4,pframe->translate.z+((pframe2->translate.z-pframe->translate.z)*m)/4);
		if(center){md2_frame_t *frm=&mdl->frames[0];glTranslate3f32(-(frm->min.x+frm->max.x)/2,-(frm->min.y+frm->max.y)/2,-(frm->min.z+frm->max.z)/2);} //TEMP? ("fake" center)
		
		glScalef32((pframe->scale.x+((pframe2->scale.x-pframe->scale.x)*m)/4),(pframe->scale.y+((pframe2->scale.y-pframe->scale.y)*m)/4),(pframe->scale.z+((pframe2->scale.z-pframe->scale.z)*m)/4));

		glScalef32(inttof32(32),inttof32(32),inttof32(32)); // necessary for v10
		GFX_COLOR=RGB15(31,31,31);
		
		if(pframe->displayList && (!m || n==n2))
		{
			glCallList(pframe->displayList);
		}else{		
			glBegin (GL_TRIANGLES);
			for (i = 0; i < mdl->header.num_tris; ++i)
			{
				// if(fakeDotProduct(pframe->faceNormals[i],u)>0)
				{
					for (j = 0; j < 3; ++j)
					{
						md2_vertex_t* pvert = &pframe->verts[mdl->triangles[i].vertex[j]];
						md2_vertex_t* pvert2 = &pframe2->verts[mdl->triangles[i].vertex[j]];
						
						GFX_TEX_COORD=mdl->packedTexcoords[mdl->triangles[i].st[j]];

						// GFX_NORMAL=anorms_table[pframe->verts[mdl->triangles[i].vertex[j]].normalIndex];
									
						vect3D v=vect((pvert->v[0]*2+((pvert2->v[0]-pvert->v[0])*m)/2),(pvert->v[1]*2+((pvert2->v[1]-pvert->v[1])*m)/2),(pvert->v[2]*2+((pvert2->v[2]-pvert->v[2])*m)/2));
						
						GFX_VERTEX10=NORMAL_PACK(v.x,v.y,v.z);
					}
				}
			}
			glEnd();
		}
	glPopMatrix(1);
}

void initModelInstance(modelInstance_struct* mi, md2Model_struct* mdl)
{
	if(!mi || !mdl)return;
	mi->currentAnim=0;
	mi->interpCounter=0;
	mi->currentFrame=0;
	mi->nextFrame=0;
	mi->palette=NULL;
	mi->model=mdl;
}

void changeAnimation(modelInstance_struct* mi, u16 newAnim, bool oneshot)
{
	if(!mi || mi->currentAnim==newAnim || newAnim>=mi->model->numAnim)return;
	if(!oneshot && mi->oneshot){mi->oldAnim=newAnim;return;}
	mi->oneshot=oneshot;
	mi->oldAnim=mi->currentAnim;
	mi->currentAnim=newAnim;
	mi->nextFrame=mi->model->animations[mi->currentAnim].start;
}

void updateAnimation(modelInstance_struct* mi)
{
	if(!mi)return;
	mi->interpCounter++;
	if(mi->interpCounter>=4)
	{
		mi->interpCounter=0;
		mi->currentFrame=mi->nextFrame;
		if(mi->currentFrame>=mi->model->animations[mi->currentAnim].end)
		{
			if(mi->oneshot){mi->currentAnim=mi->oldAnim;mi->oneshot=false;}
			mi->nextFrame=mi->model->animations[mi->currentAnim].start;
		}else mi->nextFrame++;
	}
}
