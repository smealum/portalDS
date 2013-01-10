#include "game/game_main.h"

particle_struct particles[NUMPARTICLES];

void initParticles(void)
{
	int i;
	for(i=0;i<NUMPARTICLES;i++)
	{
		particles[i].used=false;
	}
}

void createParticles(vect3D position, vect3D speed, u16 life, u16 color)
{
	int i;
	for(i=0;i<NUMPARTICLES;i++)
	{
		if(!particles[i].used)
		{
			particle_struct* p=&particles[i];
			p->used=true;
			p->position=position;
			p->speed=speed;
			p->life=life;
			p->color=color;
			p->alpha=31;
			p->timer=0;
			return;
		}
	}
}

void drawParticle(particle_struct* p)
{
	if(!p)return;
	glPushMatrix();
		// glPolyFmt(POLY_ALPHA(p->alpha) | POLY_CULL_NONE);
		glPolyFmt(POLY_ALPHA(p->alpha) | POLY_CULL_NONE | POLY_ID(48));
		GFX_COLOR=p->color;
		glTranslate3f32(p->position.x,p->position.y,p->position.z);
		glScalef32(1<<8,1<<8,1<<8);
		glBegin(GL_QUADS);
			const u16 size=1;
			GFX_VERTEX10=NORMAL_PACK(-size, size, 0);
			GFX_VERTEX10=NORMAL_PACK(size, size, 0);
			GFX_VERTEX10=NORMAL_PACK(size, -size, 0);
			GFX_VERTEX10=NORMAL_PACK(-size, -size, 0);
			
			GFX_VERTEX10=NORMAL_PACK(0, size, -size);
			GFX_VERTEX10=NORMAL_PACK(0, size, size);
			GFX_VERTEX10=NORMAL_PACK(0, -size, size);
			GFX_VERTEX10=NORMAL_PACK(0, -size, -size);
	glPopMatrix(1);
}

void drawParticles(void)
{
	int i;
	unbindMtl();
	for(i=0;i<NUMPARTICLES;i++)
	{
		if(particles[i].used)drawParticle(&particles[i]);
	}
}

void updateParticle(particle_struct* p)
{
	if(!p)return;
	p->position.y-=GRAVITY/4;
	p->timer++;
	p->alpha=31-(p->timer*30)/p->life;
	if(p->timer>p->life)p->used=false;
	p->position=addVect(p->position,p->speed);
}

void updateParticles(void)
{
	int i;
	unbindMtl();
	for(i=0;i<NUMPARTICLES;i++)
	{
		if(particles[i].used)updateParticle(&particles[i]);
	}
}

void particleExplosion(vect3D p, int number, u16 color)
{
	int i;
	const u16 speed=64;
	for(i=0;i<number;i++)
	{
		createParticles(p, vect((rand()%speed)-speed/2,(rand()%speed)-speed/2,(rand()%speed)-speed/2), 20, color);
	}
}

void particleExplosionDir(vect3D p, vect3D dir, int number, u16 color)
{
	int i;
	const u16 speed=8;
	for(i=0;i<number;i++)
	{
		createParticles(p, addVect(dir,vect((rand()%speed)-speed/2,(rand()%speed)-speed/2,(rand()%speed)-speed/2)), 90, color);
	}
}