/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include "stdafx.h"

u32 cnt=0;

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	// Wifi_Update();
}


//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
	// fifoSendValue32(FIFO_USER_05,cnt); //debug
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}


OBB_struct *testOBB, *testOBB2;
plane_struct testPlane;
extern u32 coll, integ, impul;
extern u8 sleeping;

int32 sqrtv(int32 x)
{
	const u16 a=f32toint(x);
	const u16 b=x&4095;
	return (a<SQRTRANGE)?((a)?mulf32(sqrtLUT1[a],sqrtLUT2[b/(2*a)]):(sqrtLUT3[b])):(0);
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	readUserSettings();

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();

	// mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

	// installWifiFIFO();
	// installSoundFIFO();

	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);
	
	setPowerButtonCB(powerButtonCB);
	
	initPI7();
	
	// initPlane(&testPlane, 0, inttof32(1), 0, inttof32(3)/2);
	
	// createAAR(0, vect(-inttof32(0),-inttof32(1)/2,-inttof32(3)), vect(inttof32(6),0,inttof32(6)), vect(0,inttof32(1),0));
	// createAAR(1, vect(-inttof32(0),-inttof32(1)/2-inttof32(6),-inttof32(3)), vect(0,inttof32(6),inttof32(6)), vect(-inttof32(1),0,0));

	u32 tm=0;
	u32 tm2=0;
	while(!exitflag)
	{
		if(getPI7Status())
		{
			cpuStartTiming(0);
	
			coll=impul=integ=0;
				updateOBBs();
				listenPI7();
				updateOBBs();
				listenPI7();
				updateOBBs();
				listenPI7();
				updateOBBs();
				listenPI7();
				updateOBBs();
				// updateOBBs();
				// updateOBBs();
				// updateOBBs();
				// updateOBBs();
				// updateOBBs();
			// fifoSendValue32(FIFO_USER_08,integ);
			// fifoSendValue32(FIFO_USER_08,coll);
			// fifoSendValue32(FIFO_USER_08,impul);
			fifoSendValue32(FIFO_USER_08,cpuEndTiming());
			fifoSendValue32(FIFO_USER_08,sleeping);
			fifoSendValue32(FIFO_USER_08,objects[0].energy);
			fifoSendValue32(FIFO_USER_08,objects[1].energy);
			
			sendDataPI7();
			// u32 key=REG_KEYINPUT;
			// if(0==(key&(KEY_B)))applyOBBForce(&objects[0],addVect(testOBB->position,vect(-inttof32(1),0,0)),vect(0,inttof32(100),0));
		}
		
		listenPI7();
		
		swiWaitForVBlank();
		cnt++;
	}
	return 0;
}
