#include "common/general.h"

#define MAX_ANGLE   512
#define ABS(x)   ((x) < 0 ? -(x) : (x))

const u32 arctan2_tab[129] =
{
	0x00000000, 0x0000000c, 0x00000019, 0x00000025, 0x00000032, 0x0000003e, 0x0000004b, 0x00000058,
	0x00000064, 0x00000071, 0x0000007e, 0x0000008b, 0x00000097, 0x000000a4, 0x000000b1, 0x000000be,
	0x000000cb, 0x000000d8, 0x000000e5, 0x000000f3, 0x00000100, 0x0000010d, 0x0000011b, 0x00000128,
	0x00000136, 0x00000144, 0x00000152, 0x00000160, 0x0000016e, 0x0000017c, 0x0000018b, 0x00000199,
	0x000001a8, 0x000001b6, 0x000001c5, 0x000001d5, 0x000001e4, 0x000001f3, 0x00000203, 0x00000213,
	0x00000223, 0x00000233, 0x00000244, 0x00000254, 0x00000265, 0x00000276, 0x00000288, 0x0000029a,
	0x000002ac, 0x000002be, 0x000002d1, 0x000002e4, 0x000002f7, 0x0000030b, 0x0000031f, 0x00000333,
	0x00000348, 0x0000035d, 0x00000373, 0x00000389, 0x000003a0, 0x000003b7, 0x000003ce, 0x000003e7,
	0x00000400, 0x00000419, 0x00000433, 0x0000044e, 0x00000469, 0x00000486, 0x000004a3, 0x000004c0,
	0x000004df, 0x000004ff, 0x00000520, 0x00000541, 0x00000564, 0x00000588, 0x000005ad, 0x000005d4,
	0x000005fc, 0x00000626, 0x00000651, 0x0000067d, 0x000006ac, 0x000006dc, 0x0000070f, 0x00000744,
	0x0000077b, 0x000007b5, 0x000007f2, 0x00000832, 0x00000875, 0x000008bb, 0x00000906, 0x00000954,
	0x000009a8, 0x00000a00, 0x00000a5e, 0x00000ac2, 0x00000b2d, 0x00000ba0, 0x00000c1b, 0x00000ca0,
	0x00000d2f, 0x00000dcb, 0x00000e74, 0x00000f2d, 0x00000ff8, 0x000010d7, 0x000011d0, 0x000012e4,
	0x0000141b, 0x0000157b, 0x0000170d, 0x000018dc, 0x00001af7, 0x00001d73, 0x0000206e, 0x00002411,
	0x0000289c, 0x00002e73, 0x0000363a, 0x0000411b, 0x0000516b, 0x00006c99, 0x0000a2f1, 0x000145ee,
	0x7fffffff
};


// ArcTan, renvoie l'angle de 0 à 128 (dx et dy doivent être positifs !)
int ArcTan(int dx, int dy)
{
    // Secure...
    if (dx == 0) return 128;
    if (dy == 0) return 0;

    u32 tan = (dy << 10) / dx;
    int start = 0;
    int end = MAX_ANGLE/4;
    int mid;

    // Dichotomie time !
    while (start < end)
    {
        //printf("%i %i => %i %i\n", start, end, tab[start], tab[end]);
        mid = (start + end) >> 1;

        if (tan == arctan2_tab[mid]) return mid;
        if (tan < arctan2_tab[mid]) end = mid-1;
        else start = mid+1;
    }

    return mid;
}

// Récupère l'angle de 0 à 90 et corrige pour obtenir
int ArcTan2(int dx, int dy)
{
    int angle = ArcTan(ABS(dx), ABS(dy));

	//iprintf("\nTEST2 : %d, %d, ",dx,dy);
	
    if (dx < 0) angle = 256 - angle;
    if (dy < 0) angle = 512 - angle;
	
	//iprintf("%d  \n",angle);
	
    return angle;
}

u64 Math_FakeDistance(s32 x1, s32 y1, s32 x2, s32 y2)
{
	return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}

u16 Math_AdjustAngle(u16 angle, s16 anglerot, s32 startx, s32 starty, s32 targetx, s32 targety) {
u64 distances[3];

	startx = startx << 8; // Fixed point...
	starty = starty << 8; // Fixed point...
	targetx = targetx << 8; // Fixed point...
	targety = targety << 8; // Fixed point...

      u16 tempangle = (angle - anglerot) & 511;


      // Calcul des distances en fonction des angles
      distances[0] = Math_FakeDistance(startx + Math_Cos(tempangle), starty - Math_Sin(tempangle), targetx, targety);
      tempangle += anglerot;
	  tempangle &= 511;
      distances[1] = Math_FakeDistance(startx + Math_Cos(tempangle), starty - Math_Sin(tempangle), targetx, targety);
      tempangle += anglerot;
	  tempangle &= 511;
      distances[2] = Math_FakeDistance(startx + Math_Cos(tempangle), starty - Math_Sin(tempangle), targetx, targety);
                                 
                                 

      // On regarde si l'angle est optimal. Si ce n'est pas le cas,
      // on fait tourner toujours dans le meme sens...
      if (distances[0] < distances[1])  angle -= anglerot;
      else if (distances[2] < distances[1])  angle += anglerot;
           
      return (angle&511);    
}

