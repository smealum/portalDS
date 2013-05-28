#ifndef __PARTICLES9__
#define __PARTICLES9__

#define NUMPARTICLES 256

typedef struct
{
	vect3D position, speed;
	u16 life, timer, color;
	u8 alpha;
	bool used;
}particle_struct;

void initParticles(void);
void drawParticles(void);
void updateParticles(void);
void particleExplosion(vect3D p, int number, u16 color);
void particleExplosionDir(vect3D p, vect3D dir, int number, u16 color);
void createParticles(vect3D position, vect3D speed, u16 life, u16 color);

#endif
