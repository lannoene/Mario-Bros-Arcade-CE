#include "level.h"


#include <keypadc.h>
#include <graphx.h>

#include "screens.h"
#include "draw.h"
#include "player.h"
#include "platforms.h"

unsigned int gameFrame = 0;
player_t mario1;
gameData_t game_data = {};

bool LevelLoop(void) {
	mario1.moving = false;
	// check for button presses
	if (kb_Data[6] & kb_Clear) {
		UnloadLevel();
		return false;
	}
	if (kb_Data[7] & kb_Right) {
		PlayerMove(&mario1, RIGHT);
	}
	if (kb_Data[7] & kb_Left) {
		PlayerMove(&mario1, LEFT);
	}
	if (kb_Data[1] & kb_2nd) {
		PlayerMove(&mario1, UP); // jump
	}
	
	// update player
	UpdatePlayer(&mario1, gameFrame);
	
	// draw updated player
	DrawScene(&mario1, gameFrame);
	++gameFrame;
	
	return true;
}

bool LoadLevel(void) {
	// we only need to draw the background once because we perform non-destructive sprite placement during the game loop in order to save on resources
	DrawBackground();
	
	// init level platform struct
	InitPlatformData();
	
	// spawn in platforms. width must be a multiple of 8 (the tile size)
	// bottom platforms
	CreatePlatform(0, 176, 112);
	CreatePlatform(208, 176, 112);
	// small side platforms
	CreatePlatform(0, 128, 48);
	CreatePlatform(272, 128, 48);
	// middle sized middle platform
	//CreatePlatform(272, 128, 48);
	
	// init player
	PlayerInit(&mario1);
	
	// change windows
	ChangeScreen(SCR_LEVEL);
	
	// return
	return true;
}

void UnloadLevel(void) {
	// deinit platform data
	FreePlatforms();
}