#include "editor/editor_main.h"

#include <dirent.h>

#define TOOLBUTTONS 9
#define ACTIONBUTTONS 16

#define MAXLISTBUTTONS 8

extern char* basePath;

typedef struct
{
	vect2D size;
	guiEntity_struct* window;
	guiEntity_struct* label;
	guiEntity_struct* slider;
	guiEntity_struct* button[MAXLISTBUTTONS];
	guiFunction function;
	char** nameList;
	u16 numElements;
	u8 numButtons;
	u16 cursor;
}listWindow_struct;

listWindow_struct listWindow;

guiEntity_struct* newLevelWindow;

guiEntity_struct* fileToolWindow;
guiEntity_struct* roomToolWindow;
guiEntity_struct* selectedRoomToolWindow;
guiEntity_struct* entityToolWindow;
guiEntity_struct* entityPropertyWindow;
guiEntity_struct* tileToolWindow;
guiEntity_struct* materialToolWindow;
guiEntity_struct* materialWindowLabel;
guiEntity_struct* toolButtons[TOOLBUTTONS];
guiEntity_struct* actionButtons[ACTIONBUTTONS];
extern roomEdit_struct *selectedRoom;
extern roomEdit_struct *oldSelectedRoom;
extern entity_struct *currentEntity;
extern u8 roomEditorMode;

int* correspondanceList=NULL;
int selectedMaterial=0;

void updateListWindowButtons(listWindow_struct* lw);

void listWindowSlider(guiEntity_struct* e)
{
	listWindow_struct* lw=NULL;
	if(!lw)lw=&listWindow;
	guiSliderData_struct* d=(guiSliderData_struct*)e->data;
	lw->cursor=(d->position*(lw->numElements-lw->numButtons))/d->size;
	if(d->position!=d->oldpos)updateListWindowButtons(lw);
}

int getListWindowButtonPosition(listWindow_struct* lw, char* text)
{
	if(!lw)lw=&listWindow;
	int w=strlen(text)*8;
	return (lw->size.x-w)/2;
}

void initListWindow(listWindow_struct* lw)
{
	if(!lw)lw=&listWindow;
	int i;
	lw->size=vect2(180,120);
	lw->window=createWindow(200,8,lw->size.x,lw->size.y,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	lw->window->prio=512;
	lw->label=createLabelFather(60, 6, RGB15(31,31,31), lw->window, "filler", false);
	lw->slider=createVSliderFather(lw->size.x-20, 10, 100, listWindowSlider, lw->window, "", false);
	for(i=0;i<MAXLISTBUTTONS;i++)
	{
		lw->button[i]=createButtonFather(3, 20+20*i, RGB15(31,31,31), NULL, lw->window, "filler", "", false);
		lw->button[i]->outline=1;
	}
	lw->nameList=NULL;
	lw->numElements=0;
	lw->cursor=0;
	lw->numButtons=MAXLISTBUTTONS;
}

void openListWindow(listWindow_struct* lw, char* t, char** nl, u16 ne, u8 nb, guiFunction f)
{
	if(!lw)lw=&listWindow;
	lw->cursor=0;
	updateLabelText(lw->label,t);
	setPositionX(lw->label,getListWindowButtonPosition(lw,t));
	lw->function=f;
	lw->nameList=nl;
	lw->numButtons=nb;
	lw->numElements=ne;
	setPositionX(lw->window,-lw->size.x/2);
	lw->size.y=20+20*lw->numButtons;
	setSizeA(lw->window,lw->size.x,lw->size.y);
	if(lw->numElements<lw->numButtons)lw->numButtons=lw->numElements;
	updateListWindowButtons(lw);
}

void closeListWindow(listWindow_struct* lw)
{
	if(!lw)lw=&listWindow;
	setPositionX(lw->window,200);
}

void updateListWindowButtons(listWindow_struct* lw)
{
	if(!lw)lw=&listWindow;
	if(lw->cursor+lw->numButtons>lw->numElements)lw->cursor=lw->numElements-lw->numButtons;
	if(lw->cursor<0)lw->cursor=0;
	int i;
	for(i=0;i<lw->numButtons;i++)
	{
		updateButtonText(lw->button[i],lw->nameList[lw->cursor+i]);
		((guiButtonData_struct*)(lw->button[i]->data))->function=lw->function;
		setPositionX(lw->button[i],getListWindowButtonPosition(lw,lw->nameList[lw->cursor+i]));
	}
	for(;i<MAXLISTBUTTONS;i++)setPositionX(lw->button[i],300);
}

char** listFiles(char* directory, int *k)
{
	DIR *dir;
	NOGBA("%s ?", directory);
	struct dirent *ent;
	dir=opendir(directory);
	int n=0;
	if(dir)
	{
		while((ent=readdir(dir)))if(ent->d_name[0]!='.')n++;
		closedir(dir);
	}else return NULL;
	NOGBA("%d",n);
	NOGBA("1");
	u8* strLengths=malloc(n);
	if(!strLengths)return NULL;
	n=0;
	dir=opendir(directory);
	if(dir)
	{
		while((ent=readdir(dir)))
		{
			if(ent->d_name[0]!='.')
			{
				strLengths[n]=strlen(ent->d_name);
				n++;
			}
		}
		closedir(dir);
	}else {free(strLengths);return NULL;}
	NOGBA("%d",n);
	NOGBA("1");
	int i;
	char** list=malloc(sizeof(char*)*(n+1));
	if(!list){free(strLengths);return NULL;}
	for(i=0;i<n;i++){list[i]=malloc(strLengths[i]+1);}
	free(strLengths);
	n=0;
	dir=opendir(directory);
	if(dir)
	{
		while((ent=readdir(dir)))
		{
			if(ent->d_name[0]!='.')
			{
				strcpy(list[n],ent->d_name);
				NOGBA("%s",list[n]);
				n++;
			}
		}
		closedir(dir);
	}else {free(list);return NULL;}
	NOGBA("%d",n);
	*k=n;
	list[n]=NULL;
	return list;
}

void freeLevelList(char** l)
{
	char** ll=l;
	while(*ll){free(*(ll));ll++;}
	free(l);
}

void callNewLevelWindow(guiEntity_struct* e)
{
	setPositionX(newLevelWindow, -90);
}

void hideNewLevelWindow(void)
{
	setPositionX(newLevelWindow, -400);
}

void newLevelYesButton(guiEntity_struct* e)
{
	selectedRoom=NULL;
	currentEntity=NULL;
	wipeMapEdit();
	hideNewLevelWindow();
}

void newLevelNoButton(guiEntity_struct* e)
{
	hideNewLevelWindow();
}

void initNewLevelWindow(void)
{
	u16 w=180;
	// newLevelWindow=createWindow(-w/2,16,w,78,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	newLevelWindow=createWindow(-400,16,w,78,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	newLevelWindow->prio+=64;
	u16 k=w/2-strlen("Unsaved data will be")*4;
	createLabelFather(w/2-3*8-4, 8, RGB15(31,31,31), newLevelWindow, "New Map", false);
	createLabelFather(k, 24, RGB15(31,31,31), newLevelWindow,    "Unsaved data will be", false);
	createLabelFather(k, 24+12, RGB15(31,31,31), newLevelWindow, "lost. Are you sure ?", false);
	createButtonFather(w/2-38, 24*2+4, RGB15(31,31,31), newLevelYesButton, newLevelWindow, "Yes", "", false)->outline=1;
	createButtonFather(w/2+38-16-8, 24*2+4, RGB15(31,31,31), newLevelNoButton, newLevelWindow, "No", "", false)->outline=1;
}

void resetRoomButtons(void)
{
	int i;
	for(i=0;i<TOOLBUTTONS;i++)if(toolButtons[i])((guiButtonData_struct*)toolButtons[i]->data)->colorTakeover=false;
}

void commonToolButton(guiEntity_struct* e)
{
	guiButtonData_struct* data=(guiButtonData_struct*)e->data;
	resetRoomButtons();
	data->colorTakeover=true;
	e->outline_color=RGB15(31,0,0);
	e->color=RGB15(31,20,27);
	NOGBA("LALALA"); 
}

void buttonCreateRoom(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(ROOMCREATION);
}

void buttonSelectRoom(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(ROOMSELECTION);
}

void buttonResizeRoom(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(ROOMSELECTRESIZE);
}

void buttonMoveRoom(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(ROOMSELECTMOVE);
}

void buttonSelectMaterialFunction(guiEntity_struct* e)
{
	closeListWindow(NULL);
	if(!correspondanceList)return;
	int i;
	for(i=0;i<listWindow.numButtons;i++)
	{
		if(e==listWindow.button[i])break;
	}
	if(i<listWindow.numButtons)
	{
		selectedMaterial=correspondanceList[i+listWindow.cursor];
		updateLabelText(materialWindowLabel,listWindow.nameList[i+listWindow.cursor]);
	}else {selectedMaterial=0;updateLabelText(materialWindowLabel,listWindow.nameList[0]);}
	freeMaterialList(listWindow.nameList);
	free(correspondanceList);
	correspondanceList=NULL;
}

void buttonSelectMaterial(guiEntity_struct* e)
{
	int n;
	char** l=getMaterialList(&n, &correspondanceList);
	openListWindow(NULL,"Select material", l,n,6,&buttonSelectMaterialFunction);
}

void buttonApplyMaterial(guiEntity_struct* e)
{
	applyMaterial(getMaterial(selectedMaterial), NULL, NULL, NULL);
}

extern u8 selectionMode;
extern u8 oldSelectionMode;

void buttonSelectTiles(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(TILESELECTION);
	selectionMode=1;
}

void buttonUpTiles(guiEntity_struct* e)
{
	moveData(1, NULL, NULL, NULL);
}

void buttonDownTiles(guiEntity_struct* e)
{
	moveData(-1, NULL, NULL, NULL);
}

void buttonSwapData(guiEntity_struct* e)
{
	swapData(NULL);
}

void buttonDeleteRoom(guiEntity_struct* e)
{
	deleteRoomEdit(NULL);
	selectionMode=0;
}

void buttonRenderRoom(guiEntity_struct* e)
{
	optimizeRoom(selectedRoom);
}

void buttonMakeWall(guiEntity_struct* e)
{
	makeWall(NULL, NULL, NULL);
}

void buttonSelectEntityTool(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(ENTITYSELECTION);
	selectionMode=0;
}

void buttonCreateLightTool(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(LIGHTCREATION);
}

void buttonCreateEnemyTool(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(ENEMYCREATION);
}

void buttonCreateDoorTool(guiEntity_struct* e)
{
	commonToolButton(e);
	setRoomEditorMode(DOORCREATION);
}

void buttonUpEntity(guiEntity_struct* e)
{
	if(currentEntity)
	{
		currentEntity->position.z++;
		changeRoom(selectedRoom,false);
	}
}

void buttonDownEntity(guiEntity_struct* e)
{
	if(currentEntity)
	{
		currentEntity->position.z--;
		changeRoom(selectedRoom,false);
	}
}

void buttonUpLight(guiEntity_struct* e)
{
	if(currentEntity)
	{
		if(currentEntity->type==lightEntity)
		{
			lightData_struct* d=currentEntity->data;
			d->intensity=min(d->intensity+TILESIZE,TILESIZE*2*60);
			changeRoom(selectedRoom,false);
		}
	}
}

void buttonDownLight(guiEntity_struct* e)
{
	if(currentEntity)
	{
		if(currentEntity->type==lightEntity)
		{
			lightData_struct* d=currentEntity->data;
			d->intensity=max(d->intensity-TILESIZE,128);
			changeRoom(selectedRoom,false);
		}
	}
}

void buttonDeleteEntity(guiEntity_struct* e)
{
	if(selectedRoom && currentEntity)
	{
		if(currentEntity->type==lightEntity)changeRoom(selectedRoom,false);
		removeEntity(&selectedRoom->entityCollection,currentEntity);
		currentEntity=NULL;
	}
}

char** levelList=NULL;

void actuallyLoadLevelButton(guiEntity_struct* e)
{
	int i;
	for(i=0;i<listWindow.numButtons;i++)
	{
		if(e==listWindow.button[i])break;
	}
	if(i<listWindow.numButtons)
	{
		i+=listWindow.cursor;
	}
	wipeMapEdit();
	readMap(levelList[i], false);
	freeLevelList(levelList);
	levelList=NULL;
	closeListWindow(NULL);
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void loadLevelButton(guiEntity_struct* e)
{
	int k=0;
	char str[255];
	sprintf(str,"%sfpsm/maps/",basePath);
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	levelList=listFiles(str, &k);
	
	openListWindow(NULL, "Load Map", levelList, k, 6, actuallyLoadLevelButton);
}

void saveLevelButton(guiEntity_struct* e)
{
	writeMap("lalala.map");
}

void updateSelectedRoomToolWindow(void)
{
	if(!selectedRoom)
	{
		if(oldSelectedRoom)
		{
			setPosition(selectedRoomToolWindow, -200, f32toint(selectedRoomToolWindow->Position.y));
			if(roomEditorMode>ROOMSELECTION)buttonCreateRoom(toolButtons[0]);
		}
	}else{
		if(!oldSelectedRoom)setPosition(selectedRoomToolWindow, -120, f32toint(selectedRoomToolWindow->Position.y));
	}
}

void updateEntityToolWindow(void)
{
	if(!selectedRoom)
	{
		if(oldSelectedRoom)
		{
			setPosition(entityToolWindow, -260, f32toint(entityToolWindow->Position.y));
			if(roomEditorMode>ROOMSELECTION)buttonCreateRoom(toolButtons[0]);
		}
	}else{
		if(!oldSelectedRoom)setPosition(entityToolWindow, -120+24+8+8+64, f32toint(entityToolWindow->Position.y));
	}
}

void updateTileToolWindow(void)
{
	if(!selectionMode)
	{
		if(oldSelectionMode)setPosition(tileToolWindow, 200, f32toint(tileToolWindow->Position.y));
	}else if(selectionMode==2){
		if(oldSelectionMode<=1)setPosition(tileToolWindow, 96, f32toint(tileToolWindow->Position.y));
	}
}

void updateEntityPropertyWindow(void)
{
	if(!selectionMode)
	{
		if(oldSelectionMode)setPosition(entityPropertyWindow, 200, f32toint(entityPropertyWindow->Position.y));
	}else if(selectionMode==3){
		if(oldSelectionMode<=1)setPosition(entityPropertyWindow, 96, f32toint(entityPropertyWindow->Position.y));
	}
}

void initEditor(void)
{
	// NOGBA("initializing...");
	// videoSetMode(MODE_0_3D);
	
	// glInit();
	
	// initD3D();
	
	// glEnable(GL_TEXTURE_2D);
	// glEnable(GL_BLEND);
	
	// glClearColor(0,0,0,31);
	// glClearPolyID(63);
	// glClearDepth(0x7FFF);

	// glViewport(0,0,255,191);
	
	// initVramBanks(2);
	// initVramBanks(1);
	// initTextures();
	
	// initCamera(NULL);

	// initText();
	// initGrid();
	// initRoomEditor();
	// initGui();
	
	// initMaterials();
	// initDoorCollection(NULL);
	
	// fileToolWindow=createWindow(-120+24+8,8,64,24,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// actionButtons[13]=createButtonFather(4, 3, RGB15(31,31,31), callNewLevelWindow, fileToolWindow, "", "icon21.pcx", false);
	// actionButtons[14]=createButtonFather(4+(16+4), 3, RGB15(31,31,31), loadLevelButton, fileToolWindow, "", "icon22.pcx", false);
	// actionButtons[15]=createButtonFather(4+(16+4)*2, 3, RGB15(31,31,31), saveLevelButton, fileToolWindow, "", "icon23.pcx", false);
	
	
	// roomToolWindow=createWindow(-120,8,24,88,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// toolButtons[0]=createButtonFather(3, 4, RGB15(31,31,31), buttonCreateRoom, roomToolWindow, "", "icon1.pcx", false);
	// toolButtons[1]=createButtonFather(3, 4+(16+4), RGB15(31,31,31), buttonSelectRoom, roomToolWindow, "", "icon2.pcx", false);
	// toolButtons[2]=createButtonFather(3, 4+(16+4)*2, RGB15(31,31,31), buttonResizeRoom, roomToolWindow, "", "icon3.pcx", false);
	// toolButtons[3]=createButtonFather(3, 4+(16+4)*3, RGB15(31,31,31), buttonMoveRoom, roomToolWindow, "", "icon4.pcx", false);
	
	// resetRoomButtons();
	// buttonCreateRoom(toolButtons[0]);
	
	// selectedRoomToolWindow=createWindow(-200,f32toint(roomToolWindow->Position.y+roomToolWindow->Size.y)+8,24,88,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// toolButtons[4]=createButtonFather(3, 4, RGB15(31,31,31), buttonSelectTiles, selectedRoomToolWindow, "", "icon5.pcx", false);
	// actionButtons[0]=createButtonFather(3, 4+(16+4), RGB15(31,31,31), buttonDeleteRoom, selectedRoomToolWindow, "", "icon6.pcx", false);
	// actionButtons[1]=createButtonFather(3, 4+(16+4)*2, RGB15(31,31,31), buttonSwapData, selectedRoomToolWindow, "", "icon7.pcx", false);
	// actionButtons[8]=createButtonFather(3, 4+(16+4)*3, RGB15(31,31,31), buttonRenderRoom, selectedRoomToolWindow, "", "icon14.pcx", false);

	// tileToolWindow=createWindow(200,8,24,64,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// actionButtons[2]=createButtonFather(3, 4, RGB15(31,31,31), buttonUpTiles, tileToolWindow, "", "icon8.pcx", false);
	// actionButtons[3]=createButtonFather(3, 4+(16+4), RGB15(31,31,31), buttonDownTiles, tileToolWindow, "", "icon9.pcx", false);
	// actionButtons[4]=createButtonFather(3, 4+(16+4)*2, RGB15(31,31,31), buttonMakeWall, tileToolWindow, "", "icon10.pcx", false);

	// entityToolWindow=createWindow(-260,8,86,24,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// toolButtons[5]=createButtonFather(4, 3, RGB15(31,31,31), buttonSelectEntityTool, entityToolWindow, "", "icon13.pcx", false);
	// toolButtons[6]=createButtonFather(4+(16+4), 3, RGB15(31,31,31), buttonCreateLightTool, entityToolWindow, "", "icon11.pcx", false);
	// toolButtons[7]=createButtonFather(4+(16+4)*2, 3, RGB15(31,31,31), buttonCreateEnemyTool, entityToolWindow, "", "icon12.pcx", false);
	// toolButtons[8]=createButtonFather(4+(16+4)*3, 3, RGB15(31,31,31), buttonCreateDoorTool, entityToolWindow, "", "icon20.pcx", false);

	// entityPropertyWindow=createWindow(200,8,24,104,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// actionButtons[5]=createButtonFather(3, 4, RGB15(31,31,31), buttonUpEntity, entityPropertyWindow, "", "icon8.pcx", false);
	// actionButtons[6]=createButtonFather(3, 4+(16+4), RGB15(31,31,31), buttonDownEntity, entityPropertyWindow, "", "icon9.pcx", false);
	// actionButtons[7]=createButtonFather(3, 4+(16+4)*2, RGB15(31,31,31), buttonUpLight, entityPropertyWindow, "", "icon17.pcx", false);
	// actionButtons[9]=createButtonFather(3, 4+(16+4)*3, RGB15(31,31,31), buttonDownLight, entityPropertyWindow, "", "icon18.pcx", false);
	// actionButtons[10]=createButtonFather(3, 4+(16+4)*4, RGB15(31,31,31), buttonDeleteEntity, entityPropertyWindow, "", "icon19.pcx", false);

	// materialToolWindow=createWindow(-120+24+8,191-24,140,24,31,RGB15(31,31,31),1,RGB15(0,0,0),"");
	// actionButtons[11]=createButtonFather(4, 3, RGB15(31,31,31), buttonSelectMaterial, materialToolWindow, "", "icon15.pcx", false);
	// actionButtons[12]=createButtonFather(4+(16+4), 3, RGB15(31,31,31), buttonApplyMaterial, materialToolWindow, "", "icon16.pcx", false);
	// createLabelFather(4+(16+4)*2, 2, RGB15(31,31,31), materialToolWindow, "selected :", false);
	// materialWindowLabel=createLabelFather(4+(16+4)*2+4, 2+12, RGB15(31,31,31), materialToolWindow, "material0", false);
	
	// initListWindow(NULL);
	// initNewLevelWindow();
	
	// loadMaterialSlices("slices.ini");
	// loadMaterials("materials.ini");
	
	// getVramStatus();
	// NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	// fadeIn();
}

int frm=0;
int frm2=0;
int testTime=0;

void editorFrame(void)
{
	/*touchPosition touchPos;
	switch(d3dScreen)
	{
		case true:
			projectGrid();
			
			glPushMatrix();
				scanKeys();
				touchRead(&touchPos);
				
				if(keysHeld()&(KEY_RIGHT))translateGrid(vect(-(1<<7),0,0));
				if(keysHeld()&(KEY_LEFT))translateGrid(vect((1<<7),0,0));
				if(keysHeld()&(KEY_DOWN))translateGrid(vect(0,-(1<<7),0));
				if(keysHeld()&(KEY_UP))translateGrid(vect(0,(1<<7),0));
				
				if(keysHeld()&(KEY_R))scaleGrid(1<<6);
				if(keysHeld()&(KEY_L))scaleGrid(-(1<<6));
				
				transformGrid();
				
				updateRoomEditor(touchPos.px,touchPos.py);
				
				char testStr[32];
				sprintf(testStr,"%d",testTime);
				// drawString("lalala",RGB15(31,31,31),inttof32(1)/32,0,0);
				drawString(testStr,RGB15(31,31,31),inttof32(1)/32,0,0);
				
				drawGrid();
				
				drawRoomEdits();
			glPopMatrix(1);
			
			guiCall();
			
			projectGui();
			updateTileToolWindow();
			updateEntityPropertyWindow();
			updateSelectedRoomToolWindow();
			updateEntityToolWindow();
			glPushMatrix();
				updateGui(NULL,&touchPos);
			glPopMatrix(1);
			
			glFlush(0);
			// glFlush(GL_TRANS_MANUALSORT|GL_WBUFFERING);
			// glFlush(GL_WBUFFERING);
			break;
		default:		
			projectCamera(NULL);
				
			if(keysHeld()&(KEY_A))rotateCamera(NULL, vect(0,-(1<<8),0));
			if(keysHeld()&(KEY_Y))rotateCamera(NULL, vect(0,1<<8,0));
			if(keysHeld()&(KEY_B))moveCamera(NULL, vect(0,0,inttof32(1)>>6));
			if(keysHeld()&(KEY_X))moveCamera(NULL, vect(0,0,-(inttof32(1)>>6)));
			// if(keysHeld()&(KEY_SELECT))moveCamera(NULL, vect(0,-(inttof32(1)>>6),0));
			// if(keysHeld()&(KEY_START))moveCamera(NULL, vect(0,inttof32(1)>>6,0));
			// if(keysDown()&(KEY_SELECT))writeMap("lalala.map");
			// if(keysDown()&(KEY_SELECT))changeState(&editorState);
			// if(keysDown()&(KEY_START))readMap("lalala.map", false);
			
			// if(selectedRoom)updateCameraPreview(&selectedRoom->data, NULL);
			// else 
			updateCameraPreview(NULL, NULL);
		
			glPushMatrix();
				glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);
				
				transformCamera(NULL);				
				
				frm2++;
				if(frm2>=4){frm++;frm2=0;}
				
				drawRoomsPreview();
								
			glPopMatrix(1);
			
			glFlush(0);
			// glFlush(GL_TRANS_MANUALSORT|GL_WBUFFERING);
			// glFlush(GL_WBUFFERING);
			break;
	}
	
	swiWaitForVBlank();
	updateD3D();*/
}

void killEditor(void)
{
	// fadeOut();
	// NOGBA("KILLING IT");
	// cleanUpGui();
	// wipeMapEdit();
	// freeState(NULL);
	// NOGBA("END mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void editorVBL(void)
{

}
