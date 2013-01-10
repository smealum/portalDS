#ifndef __PORTALS9__
#define __PORTALS9__

#define PORTALMARGIN (32)

typedef struct portal_struct
{
	camera_struct camera;
	mtlImg_struct* texture;
	vect3D position;
	u16 viewPoint[256*192];
	u16 color;
	vect3D normal, plane[2];
	int32 angle, oldZ;
	polygon_struct *polygon, *unprojectedPolygon;
	struct portal_struct* targetPortal;
}portal_struct;

extern portal_struct portal1, portal2;
extern portal_struct* currentPortal;

void initPortals(void);
void updatePortals(void);
void initPortal(portal_struct* p, vect3D pos, vect3D normal, bool color);
void drawPortal(portal_struct* p);
void movePortal(portal_struct* p, vect3D pos, vect3D normal, int32 angle, bool send);
void updatePortalCamera(portal_struct* p, camera_struct* c);

void collidePortal(room_struct* r, rectangle_struct* rec, portal_struct* p, vect3D* point);

bool isPortalOnWall(room_struct* r, portal_struct* p, bool fix);

#endif
