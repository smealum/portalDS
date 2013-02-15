#include "editor/editor_main.h"

selection_struct editorSelection;

contextButton_struct entitySelectionButtonArray[]={(contextButton_struct){"set target", NULL}, (contextButton_struct){"portalable", NULL}};
contextButton_struct planarSelectionButtonArray[]={(contextButton_struct){"unportalable", NULL}, (contextButton_struct){"portalable", NULL}};
contextButton_struct nonplanarSelectionButtonArray[]={(contextButton_struct){"unportalable", NULL}, (contextButton_struct){"portalable", NULL}, (contextButton_struct){"fill", NULL}, (contextButton_struct){"delete", NULL}};

void initSelection(selection_struct* s)
{
	if(!s)s=&editorSelection;
	
	s->firstFace=s->secondFace=NULL;
	s->origin=s->size=vect(0,0,0);
	s->active=false;
}

void adjustSelection(editorRoom_struct* er, selection_struct* s, blockFace_struct of, blockFace_struct os, blockFace_struct oc, vect3D v)
{
	if(!s)return;

	of.x+=v.x;of.y+=v.y;of.z+=v.z;
	os.x+=v.x;os.y+=v.y;os.z+=v.z;
	oc.x+=v.x;oc.y+=v.y;oc.z+=v.z;

	s->firstFace=findBlockFace(er->blockFaceList, of.x, of.y, of.z, of.direction);
	s->secondFace=findBlockFace(er->blockFaceList, os.x, os.y, os.z, os.direction);
	s->currentFace=findBlockFace(er->blockFaceList, oc.x, oc.y, oc.z, oc.direction);

	if(!s->firstFace || !s->secondFace || !s->currentFace)s->active=false;
}

void undoSelection(selection_struct* s)
{
	if(!s)return;

	cleanUpContextButtons();
	s->entity=NULL;
	s->active=false;
	s->selecting=false;
}

void updateSelection(selection_struct* s)
{
	if(!s)s=&editorSelection;
	if(!s->active || (!s->entity && (!s->firstFace || !s->secondFace)))return;
	
	if(s->entity)
	{
		s->planar=false;
		s->origin=s->entity->position;
		s->size=vect(1,1,1);
	}else{
		s->planar=(s->firstFace->direction==s->secondFace->direction) && 
				   (((s->firstFace->direction==0 || s->firstFace->direction==1) && s->firstFace->x==s->secondFace->x)
				 || ((s->firstFace->direction==2 || s->firstFace->direction==3) && s->firstFace->y==s->secondFace->y)
				 || ((s->firstFace->direction==4 || s->firstFace->direction==5) && s->firstFace->z==s->secondFace->z));
		
		if(s->planar)
		{
			s->origin=vect(s->firstFace->x,s->firstFace->y,s->firstFace->z);
			s->size=vectDifference(vect(s->secondFace->x,s->secondFace->y,s->secondFace->z),s->origin);
		}else{
			s->origin=vect(s->firstFace->x,s->firstFace->y,s->firstFace->z);
			switch(s->firstFace->direction)
			{
				case 0: if(s->secondFace->x>s->origin.x)s->origin.x++; break;
				case 1: if(s->secondFace->x<s->origin.x)s->origin.x--; break;
				case 2: if(s->secondFace->y>s->origin.y)s->origin.y++; break;
				case 3: if(s->secondFace->y<s->origin.y)s->origin.y--; break;
				case 4: if(s->secondFace->z>s->origin.z)s->origin.z++; break;
				case 5: if(s->secondFace->z<s->origin.z)s->origin.z--; break;
			}
			s->size=vectDifference(vect(s->secondFace->x,s->secondFace->y,s->secondFace->z),s->origin);
			switch(s->secondFace->direction)
			{
				case 0: if(s->firstFace->x>s->secondFace->x)s->size.x++; break;
				case 1: if(s->firstFace->x<s->secondFace->x)s->size.x--; break;
				case 2: if(s->firstFace->y>s->secondFace->y)s->size.y++; break;
				case 3: if(s->firstFace->y<s->secondFace->y)s->size.y--; break;
				case 4: if(s->firstFace->z>s->secondFace->z)s->size.z++; break;
				case 5: if(s->firstFace->z<s->secondFace->z)s->size.z--; break;
			}
		}
		if(s->size.x<0){s->origin.x+=s->size.x;s->size.x=-s->size.x;}
		if(s->size.y<0){s->origin.y+=s->size.y;s->size.y=-s->size.y;}
		if(s->size.z<0){s->origin.z+=s->size.z;s->size.z=-s->size.z;}
		s->size.x++;s->size.y++;s->size.z++;
	}
}

bool isFaceInSelection(blockFace_struct* bf, selection_struct* s)
{
	if(!bf || !s || !s->firstFace || !s->secondFace)return false;

	if(s->planar)
	{
		if(bf->direction!=s->firstFace->direction)return false;
		switch(bf->direction)
		{
			case 0: case 1: return (bf->x==s->origin.x && bf->y>=s->origin.y && bf->y<s->origin.y+s->size.y && bf->z>=s->origin.z && bf->z<s->origin.z+s->size.z);
			case 2: case 3: return (bf->y==s->origin.y && bf->x>=s->origin.x && bf->x<s->origin.x+s->size.x && bf->z>=s->origin.z && bf->z<s->origin.z+s->size.z);
			default:		return (bf->z==s->origin.z && bf->y>=s->origin.y && bf->y<s->origin.y+s->size.y && bf->x>=s->origin.x && bf->x<s->origin.x+s->size.x);
		}
	}else{
		return (bf->x>=s->origin.x && bf->x<s->origin.x+s->size.x && bf->y>=s->origin.y && bf->y<s->origin.y+s->size.y && bf->z>=s->origin.z && bf->z<s->origin.z+s->size.z);
	}
}

void drawSelection(selection_struct* s)
{
	if(!s)s=&editorSelection;
	if(!s->active || (!s->entity && (!s->firstFace || !s->secondFace)))return;
	
	blockFace_struct* bf=s->firstFace;

	GFX_COLOR=(s->error)?RGB15(31,0,0):(RGB15(29,15,3));
	
	if(s->planar && !s->entity)
	{
		u32* vtxPtr=packedVertex[bf->direction];
		vect3D n=faceNormals[bf->direction];
		
		unbindMtl();
		
		glPolyFmt(POLY_ALPHA(15) | POLY_CULL_NONE | POLY_ID(8));
		
		glPushMatrix();
			editorRoomTransform();
			glTranslate3f32(-inttof32(1)/2,-inttof32(1)/2,-inttof32(1)/2);
			glTranslate3f32(inttof32(s->origin.x),inttof32(s->origin.y),inttof32(s->origin.z));
			glTranslate3f32(n.x/16, n.y/16, n.z/16);
			glScalef32(inttof32(s->size.x),inttof32(s->size.y),inttof32(s->size.z));
			glTranslate3f32(inttof32(1)/2,inttof32(1)/2,inttof32(1)/2);
				
			GFX_BEGIN=GL_QUADS;
			
			GFX_VERTEX10=*vtxPtr++;
			GFX_VERTEX10=*vtxPtr++;
			GFX_VERTEX10=*vtxPtr++;
			GFX_VERTEX10=*vtxPtr++;		
		glPopMatrix(1);
	}else if(!(s->entity && !s->entity->placed))
	{
		u32* vtxPtr=(u32*)packedVertex;
		
		unbindMtl();
		
		glPolyFmt(POLY_ALPHA(15) | POLY_CULL_NONE);
		
		glPushMatrix();		
			glScalef32((BLOCKSIZEX),(BLOCKSIZEY),(BLOCKSIZEZ));
			glTranslate3f32(-inttof32(1)/2,-inttof32(1)/2,-inttof32(1)/2);
			glTranslate3f32(inttof32(s->origin.x-ROOMARRAYSIZEX/2),inttof32(s->origin.y-ROOMARRAYSIZEY/2),inttof32(s->origin.z-ROOMARRAYSIZEZ/2));
			glScalef32(inttof32(s->size.x),inttof32(s->size.y),inttof32(s->size.z));
			glTranslate3f32(inttof32(1)/2,inttof32(1)/2,inttof32(1)/2);
				
			GFX_BEGIN=GL_QUADS;
			int i;for(i=0;i<6*4;i++)GFX_VERTEX10=*vtxPtr++;	
		glPopMatrix(1);
	}
	
	s->error=false;
}
