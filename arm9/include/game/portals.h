#ifndef __PORTALS9__
#define __PORTALS9__

#define PORTALMARGIN (32)

typedef struct portal_struct
{
	camera_struct camera;
	vect3D position;
	u16 viewPoint[256*192];
	u16 color, innerOutlineColor, outlineColor;
	u16 animCNT;
	vect3D normal, plane[2];
	int32 oldZ;
	u32* displayList;
	polygon_struct *outline, *unprojectedOutline;
	polygon_struct *polygon, *unprojectedPolygon;
	struct portal_struct* targetPortal;
	bool used;
}portal_struct;

extern portal_struct portal1, portal2;
extern portal_struct* currentPortal;

void initPortals(void);
void freePortals(void);
void resetPortals(void);
void updatePortals(void);
void initPortal(portal_struct* p, vect3D pos, vect3D normal, bool color);
void drawPortal(portal_struct* p);
void movePortal(portal_struct* p, vect3D pos, vect3D normal, vect3D plane0, bool actualMove);
void updatePortalCamera(portal_struct* p, camera_struct* c);

void drawPortalRoom(portal_struct* p);

void collidePortal(room_struct* r, rectangle_struct* rec, portal_struct* p, vect3D* point);

u16 getCurrentPortalColor(vect3D o);

bool isPortalOnWall(room_struct* r, portal_struct* p, bool fix);

/**
 * Determines if the two portals do not collide.
 *
 * \param[in] p one portal
 *
 * \param[in] p the other portal
 *
 * \return true if the two portal do not collides, false otherwise
 *
 * \warning Does not work if two portals are on the ceiling.
*/
bool portalToPortalIntersection(const  portal_struct* p, const portal_struct* p2);
bool isPointInPortal(portal_struct* p, vect3D o, vect3D *v, int32* x, int32* y, int32* z);

#endif
