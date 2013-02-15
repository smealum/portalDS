#ifndef __GENERAL9__
#define __GENERAL9__

#define GAMEVERSION "0.01"

#include <nds.h>
#include <fat.h>
#include <filesystem.h>
#include <maxmod9.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <unistd.h>

// #define FATONLY

#include "soundbank.h"
#include "soundbank_bin.h"

#include "common/compress.h"
#include "common/iniparser.h"
#include "common/math.h"
#include "common/files.h"
#include "common/audio.h"
#include "dual3D.h"
#include "common/pcx.h"
#include "common/textures.h"
#include "common/md2.h"
#include "common/font.h"
#include "common/simplegui.h"
#include "game/displaylist.h"

#include "game/game_ex.h"
#include "menu/menu_ex.h"
#include "editor/editor_ex.h"

#include "debug/xmem.h"
#include "engine/state.h"
#include "engine/debug.h"
#include "engine/memory.h"

extern state_struct gameState;
extern state_struct editorState;
extern state_struct ACOTMenuState;

#endif
