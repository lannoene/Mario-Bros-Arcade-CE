#include "level.h"


#include <keypadc.h>
#include <graphx.h>

#include "screens.h"
#include "draw.h"
#include "player.h"
#include "platforms.h"
#include "enemies.h"

unsigned int gameFrame = 0;
player_t mario1;
gameData_t game_data = {};

bool LevelLoop(void) {
	mario1.moving = false;
	// check for button presses
	if (kb_Data[6] & kb_Clear) {
		// make sure everything is freed before we exit
		FreeEnimies();
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
	
	if (gameFrame % 10000 == 0)
		SpawnEnemy(ENEMY_SPIKE, (rand() % 2), gameFrame);
	
	// update movables
	UpdatePlayer(&mario1, gameFrame);
	UpdateEnemies(&mario1, gameFrame);
	
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
	CreatePlatform(88, 120, BLOCK_SIZE*18);
	// top platforms
	CreatePlatform(0, 72, BLOCK_SIZE*18);
	CreatePlatform(176, 72, BLOCK_SIZE*18);
	
	// init enemies
	InitEnemies();
	
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