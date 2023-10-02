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
#include "bonus.h"

#define MAX_ENEMIES 10

#define LEVEL_BONUS	1
#define LEVEL_GAME	0

#define FPS	60

unsigned int gameFrame = 0;
player_t mario1;
gameData_t game_data = {0, 1, 0, 999999, false, false};
// arr part 1 is level index, pt 2 is switch between enemy id/enemy spawn time, pt 3 is spawn track
int16_t levelLog[][3][MAX_ENEMIES] = {
	{
		{false, BG_PIPES, NONE_ICY}, // level settings
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // enemy spawn type
		{200, 500, 800, -1, -1, -1, -1, -1, -1, -1} // enemy spawn time in frames
	},
	{
		{false, BG_PIPES, NONE_ICY},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{200, 400, 600, 800, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_PIPES, NONE_ICY},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{250, 350, 450, 600, 750, -1, -1, -1, -1}
	},
	{
		{true, BG_SNOWY, NONE_ICY},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_LAVA, NONE_ICY},
		{1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
		{150, 310, 670, 790, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_LAVA, NONE_ICY},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0},
		{1*FPS, 3*FPS, 17*FPS, 18*FPS, 30*FPS, 31*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_CASTLE, NONE_ICY},
		{ENEMY_FLY, ENEMY_FLY, ENEMY_FLY, ENEMY_FLY, 0, 0, 0, 0, 0, 0},
		{3*FPS, 6*FPS, 11*FPS, 13*FPS, -1}
	},
	{
		{false, BG_CASTLE, NONE_ICY},
		{ENEMY_FLY, ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_FLY, 0, 0, 0, 0},
		{3*FPS, 4*FPS, 13*FPS, 14*FPS, 18*FPS, 23*FPS, -1}
	},
	{
		{true, BG_SNOWY, ALL_ICY},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	},
};

bool LevelLoop(void) {
	mario1.moving = false;
	if (gameFrame - game_data.levelStartTime > 150 && !game_data.levelEnded) {
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
	
	// run level stuff
	if (!levelLog[game_data.level - 1][0][0]) { // normal level
		uint8_t i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (levelLog[game_data.level - 1][2][i] > 0 && levelLog[game_data.level - 1][2][i] == (int)(gameFrame - game_data.levelStartTime)) {
				SpawnEnemy(levelLog[game_data.level - 1][1][i], (rand() % 2), gameFrame);
				break;
			}
		}
		if (levelEnemies.enemiesLeft == 0) {
			EndLevel();
		}
	} else { // bonus level
		if (levelCoins.coinsLeft == 0 || levelCoins.bonusTimer == 0) {
			ResetCoins();
			EndLevel();
		} else {
			--levelCoins.bonusTimer;
		}
	}
	
	// update movables
	if (!game_data.levelEnded)
		UpdatePlayer(&mario1, gameFrame);
	UpdateEnemies(&mario1, gameFrame);
	
	// draw
	DrawScene(&mario1, levelLog[game_data.level - 1][0][1], gameFrame);
	++gameFrame;
	
	return true;
}

bool LoadLevel(void) {
	// we only need to draw the background once because we perform non-destructive sprite placement during the game loop in order to save on resources
	DrawBackground(levelLog[game_data.level - 1][0][1]);
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
	
	// init coins
	InitBonusData();
	
	// init player
	PlayerInit(&mario1);
	
	// init hud
	InitHud(&mario1);
	
	// configure level
	// set num of level enemies
	uint8_t i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (levelLog[game_data.level - 1][2][i] == -1) {
			break;
		}
	}
	
	levelEnemies.enemiesLeft = i;
	
	game_data.levelStartTime = gameFrame; // which should be 0
	game_data.isBonusLevel = levelLog[game_data.level - 1][0][0];
	
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
	FreeBonusCoins();
}

void EndLevel(void) {
	if (!game_data.levelEnded) {
		game_data.levelEndTime = gameFrame;
		game_data.levelEnded = true;
	}
	// wait a bit
	if (gameFrame - game_data.levelEndTime == 150) { // load next level
		// inc level
		++game_data.level;
		
		// draw new bg
		DrawBackground(levelLog[game_data.level - 1][0][1]); // levelLog[game_data.level - 1][0][1] is level background id
		RefreshPlatformBackgroundData(levelLog[game_data.level - 1][0][1]);
		
		InitPipeBackgroundData();
		for (uint8_t i = 0; i < NUM_OF_PIPES; i++)
			RedrawPipesWithNewSprite(i, 0, 0);
		
		game_data.levelStartTime = gameFrame;
		game_data.levelEnded = false;
		game_data.isBonusLevel = levelLog[game_data.level - 1][0][0];
		
		switch (levelLog[game_data.level - 1][0][2]) {
			case ALL_ICY:
				for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
					FreezePlatform(i);
				}
				break;
		}
		
		// reaccount for enemies
		uint8_t i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (levelLog[game_data.level - 1][2][i] == -1) {
				break;
			}
		}
		levelEnemies.enemiesLeft = i;
		
		// reset player
		mario1.x = 16;
		mario1.y = 224 - PLAYER_HEIGHT;
		mario1.dir = RIGHT;
		mario1.verAccel = mario1.horAccel = 0;
		
		if (levelLog[game_data.level - 1][0][0]) { // if bonus level
			ResetPows();
			levelCoins.bonusTimer = 1200;
			SpawnBonusCoin(16, 80);
			SpawnBonusCoin(300, 80);
			SpawnBonusCoin(32, 80);
			SpawnBonusCoin(274, 80);
			
			SpawnBonusCoin(96, 190);
			SpawnBonusCoin(224, 190);
			
			SpawnBonusCoin(136, 138);
			SpawnBonusCoin(195, 138);
			
			SpawnBonusCoin(100, 16);
			SpawnBonusCoin(220, 16);
		}
	}
}