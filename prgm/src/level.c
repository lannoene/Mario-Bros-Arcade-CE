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
#include "pow.h"

#define MAX_ENEMIES 10

unsigned int gameFrame = 0;
player_t mario1;
gameData_t game_data = {0, 1, 0, 999999, false};
// arr part 1 is level index, pt 2 is switch between enemy id/enemy spawn time, pt 3 is spawn track
int16_t enemyLog[4][2][MAX_ENEMIES] = {
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{200, 500, 800, -1, -1, -1, -1, -1, -1, -1}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{200, 400, 600, 800, -1, -1, -1, -1, -1}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{250, 350, 450, 600, 750, -1, -1, -1, -1}
	},
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	}
};

bool LevelLoop(void) {
	mario1.moving = false;
	if (gameFrame - game_data.levelStartTime > 150) {
		// check for button presses
		if (kb_Data[6] & kb_Clear) {
			// make sure everything is freed before we exit
			UnloadLevel();
			return false;
		}
		if (kb_Data[7] & kb_Right) {
			PlayerMove(&mario1, RIGHT);
		} else if (kb_Data[7] & kb_Left) {
			PlayerMove(&mario1, LEFT);
		} else {
			PlayerMove(&mario1, NONE); // deaccelerate
		}
		if (kb_Data[1] & kb_2nd) {
			PlayerMove(&mario1, UP); // jump
		}
	} else
		PlayerMove(&mario1, NONE);
	
	uint8_t i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemyLog[game_data.level - 1][1][i] == (int)(gameFrame - game_data.levelStartTime)) {
			SpawnEnemy(enemyLog[game_data.level - 1][0][i], (rand() % 2), gameFrame);
			break;
		}
	}
	if (levelEnemies.enemiesLeft == 0) {
		if (!game_data.levelEnded) {
			game_data.levelEndTime = gameFrame;
			game_data.levelEnded = true;
		}
		EndLevel();
	}
	
	// update movables
	if (!game_data.levelEnded)
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
	
	// init level pow struct
	InitPows();
	
	// spawn in pows
	CreatePow(152, 176);
	
	// init enemies
	InitEnemies();
	
	// init player
	PlayerInit(&mario1);
	
	// init hud
	InitHud(&mario1);
	
	// configure level
	// set num of level enemies
	uint8_t i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemyLog[game_data.level - 1][1][i] == -1) {
			break;
		}
	}
	
	levelEnemies.enemiesLeft = i;
	
	game_data.levelStartTime = gameFrame; // which should be 0
	
	// change windows
	ChangeScreen(SCR_LEVEL);
	
	// return
	return true;
}

void UnloadLevel(void) {
	FreeEnemies();
	// deinit platform data
	FreePlatforms();
	FreePows();
}

void EndLevel(void) {
	if (gameFrame - game_data.levelEndTime == 150) {
		// inc level
		++game_data.level;
		game_data.levelStartTime = gameFrame;
		game_data.levelEnded = false;
		
		// reaccount for enemies
		uint8_t i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemyLog[game_data.level - 1][1][i] == -1) {
				break;
			}
		}
		levelEnemies.enemiesLeft = i;
		
		// reset player
		mario1.x = 16;
		mario1.y = 224 - PLAYER_HEIGHT;
		mario1.dir = RIGHT;
		mario1.verAccel = mario1.horAccel = 0;
	}
}