#ifndef PIC_H
#define PIC_H

#define PISIGNALDATA (6)
#define PISIGNALMASK ((1<<PISIGNALDATA)-1)

typedef enum
{
	PI_START=1,  	    //ARG : 0
	PI_PAUSE=2,  	    //ARG : 0
	PI_STOP=3,   	    //ARG : 0
	PI_RESET=4,  	    //ARG : 0
	PI_ADDBOX=5,  	    //ARG : 5 (id;[sizex|sizey][sizez|mass][posx][posy][posz])
	PI_APPLYFORCE=6,    //ARG : 5 (id;[posx|posy][posz][vx][vy][vz])
	PI_ADDAAR=7,  	    //ARG : 5 (id;[sizex|sizey][sizez|normal][posx][posy][posz])
	PI_MAKEGRID=8,      //ARG : 0
	PI_SETVELOCITY=9,   //ARG : 3 (id;[vx][vy][vz])
	PI_UPDATEPLAYER=10, //ARG : 3 ([vx][vy][vz])
	PI_UPDATEPORTAL=11, //ARG : 4 (id;[px][py][pz][n])
}message_type;

#ifdef ARM7
	static inline void computePortalPlane(portal_struct* p)
	{
		if(!p)return;
		
		if(p->normal.x)
		{
			if(p->normal.x>0)
			{
				p->plane[0]=vect(0,p->sin,-p->cos);
			}else{
				p->plane[0]=vect(0,p->sin,p->cos);
			}
		}else if(p->normal.y)
		{
			if(p->normal.y>0)
			{
				p->plane[0]=vect(p->cos,0,p->sin);
			}else{
				p->plane[0]=vect(p->cos,0,p->sin);
			}
		}else{
			if(p->normal.z>0)
			{
				p->plane[0]=vect(p->cos,p->sin,0);
			}else{
				p->plane[0]=vect(-p->cos,p->sin,0);
			}
		}
		
		p->plane[1]=vectProduct(p->normal,p->plane[0]);
	}
#else
	static inline void computePortalPlane(portal_struct* p)
	{
		if(!p)return;
		
		if(p->normal.x)
		{
			if(p->normal.x>0)
			{
				p->plane[0]=vect(0,sinLerp(p->angle),-cosLerp(p->angle));
			}else{
				p->plane[0]=vect(0,sinLerp(p->angle),cosLerp(p->angle));
			}
		}else if(p->normal.y)
		{
			if(p->normal.y>0)
			{
				p->plane[0]=vect(cosLerp(p->angle),0,sinLerp(p->angle));
			}else{
				p->plane[0]=vect(cosLerp(p->angle),0,sinLerp(p->angle));
			}
		}else{
			if(p->normal.z>0)
			{
				p->plane[0]=vect(cosLerp(p->angle),sinLerp(p->angle),0);
			}else{
				p->plane[0]=vect(-cosLerp(p->angle),sinLerp(p->angle),0);
			}
		}
		
		p->plane[1]=vectProduct(p->normal,p->plane[0]);
	}
#endif

static inline vect3D warpVector(portal_struct* p, vect3D v)
{
	if(!p)return vect(0,0,0);
	portal_struct* p2=p->targetPortal;
	if(!p2)return vect(0,0,0);
	
	// computePortalPlane(p2);
	
	int32 x=dotProduct(v,p->plane[0]);
	int32 y=dotProduct(v,p->plane[1]);
	int32 z=dotProduct(v,p->normal);
	
	return addVect(vectMult(p2->normal,-z),addVect(vectMult(p2->plane[0],-x),vectMult(p2->plane[1],y)));
}

#endif