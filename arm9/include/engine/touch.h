#ifndef TOUCH_H
#define TOUCH_H

#include <nds/system.h>
#include <nds/input.h>
#include <nds/touch.h>

/*!
	\brief Obtains the current touchpad state.
	\param data a touchPosition ptr which will be filled by the function.
*/
inline void touchReadFix(touchPosition *data)
{

    touchRead(data);

    // Copied and paste from nds library, as it seems that data->px and data->py are set to zero from touchRead (using latest devkitPro libnds).
    // The issue only affects this project, libnds examples are fine.
    // In the meantime, this workaround seems to work.
    s32 xscale, yscale;
    s32 xoffset, yoffset;

    xscale = ((PersonalData->calX2px - PersonalData->calX1px) << 19) / ((PersonalData->calX2) - (PersonalData->calX1));
    yscale = ((PersonalData->calY2px - PersonalData->calY1px) << 19) / ((PersonalData->calY2) - (PersonalData->calY1));

    xoffset = ((PersonalData->calX1 + PersonalData->calX2) * xscale  - ((PersonalData->calX1px + PersonalData->calX2px) << 19) ) / 2;
    yoffset = ((PersonalData->calY1 + PersonalData->calY2) * yscale  - ((PersonalData->calY1px + PersonalData->calY2px) << 19) ) / 2;

    s16 px = ( data->rawx * xscale - xoffset + xscale/2 ) >>19;
    s16 py = ( data->rawy * yscale - yoffset + yscale/2 ) >>19;

    if ( px < 0) px = 0;
    if ( py < 0) py = 0;
    if ( px > (SCREEN_WIDTH -1)) px = SCREEN_WIDTH -1;
    if ( py > (SCREEN_HEIGHT -1)) py = SCREEN_HEIGHT -1;

    data->px = px;
    data->py = py;
}
#endif