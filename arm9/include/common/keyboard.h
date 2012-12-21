#ifndef __KEYBOARD9__
#define __KEYBOARD9__

#define KEYBOARDBUTTONS 39

guiEntity_struct* keyboardWindow;
guiEntity_struct* keyboardButton[KEYBOARDBUTTONS];

static const char keyboardButtons[] = { '1','2','3','4','5','6','7','8','9','0',
										'q','w','e','r','t','y','u','i','o','p',
										'a','s','d','f','g','h','j','k','l',
										'z','x','c','v','b','n','m',' '};
						
static const u8 keyboardRows[] = {10,10,9,7};
char* keyboardString;
int keyboardCursor, keyboardStrlen;
guiDrawFunction keyboardReturn;
bool keyboardLock;


void setupKeyboard(char* string, u8 stringlen, guiDrawFunction r);
void keyboardButtonPressed(guiEntity_struct *e);
void showKeyboard();
void hideKeyboard();
void lockKeyboard();
void initKeyboard(u8 t);
void appearKeyboard(u8 t);
void disappearKeyboard(u8 t);

#endif