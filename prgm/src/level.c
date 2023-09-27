#include "level.h"

#include <keypadc.h>
#include <graphx.h>

#include "screens.h"
#include "draw.h"
#include "player.h"
#include "platforms.h"
#include "enemies.h"
#include "hud.h"
#include "pipes.h"

#define MAX_ENEMIES 10

unsigned int gameFrame = 0;
player_t mario1;
gameData_t game_data = {0, 1};
// arr part 1 is level index, pt 2 is switch between enemy id/enemy spawn time, pt 3 is spawn track
int16_t enemyLog[2][2][MAX_ENEMIES] = {
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{100, 250, 400, 550, 601, 602, 603, 604, 605, 606}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{200, 350, 500, 590, 610, 800, 900, -1, -1}
	}
};

bool LevelLoop(void) {
	mario1.moving = false;
	// check for button presses
	if (kb_Data[6] & kb_Clear) {
		// make sure everything is freed before we exit
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
	uint8_t i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemyLog[game_data.level - 1][1][i] == (int)gameFrame) {
			SpawnEnemy(enemyLog[game_data.level][0][i], (rand() % 2), gameFrame);
			break;
		}
	}
	/*if (i == MAX_ENEMIES) {
		EndLevel();
	}*/
	
	// update movables
	UpdatePlayer(&mario1, gameFrame);
	UpdateEnemies(&mario1, gameFrame);
	
	// draw
	DrawScene(&mario1, gameFrame);
	++gameFrame;
	
	return true;
}

bool LoadLevel(void) {
	// we only need to draw the background once because we perform non-destructive sprite placement during the game loop in order to save on resources
	DrawBackground();
	// init the bg for pipes. they're stationary, so we don't need to refresh the bg slice in order to save resources
	InitPipeBackgroundData();
	
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
	
	// init hud
	InitHud(&mario1);
	
	// change windows
	ChangeScreen(SCR_LEVEL);
	
	// return
	return true;
}

void UnloadLevel(void) {
	FreeEnemies();
	// deinit platform data
	FreePlatforms();
}

void EndLevel(void) {
	UnloadLevel();
	
	++game_data.level;
	LoadLevel();
}