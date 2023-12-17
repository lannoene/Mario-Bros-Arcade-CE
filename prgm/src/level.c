#include "level.h"

#include <keypadc.h>
#include <graphx.h>
#include <math.h>
#include <string.h>

#include "screens.h"
#include "draw.h"
#include "player.h"
#include "platforms.h"
#include "enemies.h"
#include "hud.h"
#include "pipes.h"
#include "pow.h"
#include "bonus.h"
#include "fireballs.h"
#include "icicles.h"
#include "save.h"
#include "particles.h"

#define MAX_ENEMIES 10

#define LEVEL_BONUS	1
#define LEVEL_GAME	0

#define LEVEL_SETTINGS 0
#define LEVEL_ENEMYTYPE 1
#define LEVEL_ENEMYSPAWNTIME 2

#define FPS	60

unsigned int gameFrame = 0;
player_t mario1;
gameData_t game_data = {0, 1, 0, 999999, false, false, false};
unsigned int scoreWhenRoundStarts = 0; // prevent infinite score
// arr part 1 is level index, pt 2 is switch between enemy id/enemy spawn time, pt 3 is spawn track
int16_t levelLog[][3][MAX_ENEMIES] = {
	{
		{false, BG_PIPES, NONE_ICY, false, false}, // level settings
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // enemy spawn type
		{200, 500, 800, -1, -1, -1, -1, -1, -1, -1} // enemy spawn time in frames
	},
	{
		{false, BG_PIPES, NONE_ICY, false, false},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{200, 400, 600, 800, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_PIPES, NONE_ICY, false, false},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{250, 350, 450, 600, 750, -1, -1, -1, -1}
	},
	{
		{true, BG_SNOWY, NONE_ICY, false, false},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_LAVA, NONE_ICY, false, false},
		{1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
		{150, 310, 670, 790, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_LAVA, NONE_ICY, false, false},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0},
		{1*FPS, 3*FPS, 17*FPS, 18*FPS, 30*FPS, 31*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_CASTLE, NONE_ICY, false, false},
		{ENEMY_FLY, ENEMY_FLY, ENEMY_FLY, ENEMY_FLY, 0, 0, 0, 0, 0, 0},
		{3*FPS, 6*FPS, 11*FPS, 13*FPS, -1}
	},
	{
		{false, BG_CASTLE, NONE_ICY, false, false},
		{ENEMY_FLY, ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_FLY, 0, 0, 0, 0},
		{3*FPS, 4*FPS, 13*FPS, 14*FPS, 18*FPS, 23*FPS, -1}
	},
	{
		{true, BG_SNOWY, TOP_ICY | MIDDLE_ICY | BOTTOM_ICY, false, false},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, NONE_ICY, HAS_FIREBALL_GREEN, true},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_FLY, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0},
		{3*FPS, 4*FPS, 11*FPS, 12*FPS, 17*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, NONE_ICY, HAS_FIREBALL_GREEN, true},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 12*FPS, 13*FPS, 19*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, NONE_ICY, HAS_FIREBALL_GREEN, true},
		{ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 11*FPS, 12*FPS, 18*FPS, 22*FPS, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, NONE_ICY, HAS_FIREBALL_GREEN, true},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_FLY, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 14*FPS, 16*FPS, 24*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, NONE_ICY, HAS_FIREBALL_GREEN, true},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 14*FPS, 15*FPS, 24*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, NONE_ICY, HAS_FIREBALL_GREEN, true},
		{ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 11*FPS, 14*FPS, 20*FPS, 23*FPS, -1, -1, -1}
	},
	{
		{true, BG_SNOWY, PLATFORMS_ARE_INVISIBLE, false, false},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY | ICICLES_FORM_LOW, HAS_FIREBALL_GREEN, true},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_CRAB, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0},
		{3*FPS, 6*FPS, 14*FPS, 15*FPS, 16*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY | ICICLES_FORM_LOW, HAS_FIREBALL_GREEN, true},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 12*FPS, 13*FPS, 19*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY | ICICLES_FORM_LOW, HAS_FIREBALL_GREEN, true},
		{ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, 0, 0, 0, 0},
		{3*FPS, 4*FPS, 15*FPS, 16*FPS, 22*FPS, 25*FPS, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY | ICICLES_FORM_LOW, HAS_FIREBALL_GREEN, true},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_FLY, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0},
		{4*FPS, 5*FPS, 13*FPS, 14*FPS, 18*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, false, false, false},
		{0, 0, 0, 0, 0},
		{-2, -1, -1, -1, -1}
	},
};

void SaveCurrentLevel(void);
void LoadLevelFromSave(void);

bool LevelLoop(void) {
	if (game_data.paused) {
		// check for kps
		if (kb_Data[7] & kb_Up) {
			PauseScreenInputEvent(UP);
		} else if (kb_Data[7] & kb_Down) {
			PauseScreenInputEvent(DOWN);
		} else {
			PauseScreenInputEvent(5);
		}
		if (kb_Data[1] & kb_2nd) {
			mario1.hasJumpedThisFrame = true; // prevent mario from jumping if the menu closes
			switch (PauseScreenInputEvent(4)) {
				case 0: // resume
					game_data.paused = false;
					return true;
					break;
				case 1: // quit game
					// make sure everything is freed before we exit
					SaveCurrentLevel();
					UnloadLevel();
					return false;
					break;
				case 2: // save & quit game
					UnloadLevel();
					return false;
					break;
				case 3: // restart game
					game_data.paused = false;
					RestartLevels();
					return true;
					break;
			}
		}
		// draw
		DrawScene(&mario1, levelLog[game_data.level - 1][LEVEL_SETTINGS][1], gameFrame);
	} else {
		if (gameFrame - game_data.levelStartTime > 150 && !game_data.levelEnded) {
			// check for button presses
			if (kb_Data[6] & kb_Clear) {
				game_data.paused = true;
				PauseScreenResetCursorPos(); // make sure the cursor always starts at the top
				return true;
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
			} else {
				PlayerMove(&mario1, NOJUMP);
			}
		} else
			PlayerMove(&mario1, NONE);
		
		// run level stuff
		if (!levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_ISBONUS]) { // normal level
			uint8_t i;
			for (i = 0; i < MAX_ENEMIES; i++) {
				if (levelLog[game_data.level - 1][LEVEL_ENEMYSPAWNTIME][i] > 0 && levelLog[game_data.level - 1][LEVEL_ENEMYSPAWNTIME][i] == (int)(gameFrame - game_data.levelStartTime)) {
					SpawnEnemy(levelLog[game_data.level - 1][LEVEL_ENEMYTYPE][i], (rand() % 2), gameFrame);
					break;
				}
			}
			if (levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_HASFREEZIES] && rand() % 1000 == 0 && !game_data.levelEnded && gameFrame - game_data.levelStartTime > 150) {
				for (i = 0; i < levelEnemies.numEnemies; i++) {
					if (levelEnemies.enemyArray[i].state != ENEMY_DEAD && levelEnemies.enemyArray[i].type == ENEMY_FREEZIE) {
						break;
					}
				}
				if (i == levelEnemies.numEnemies) // make sure only one freezie max can go on field at a time
					SpawnEnemy(ENEMY_FREEZIE, (rand() % 2), gameFrame);
			}
			if (levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_PLATFORMS] & ICICLES_FORM_LOW && rand() % 100 == 0 && !game_data.levelEnded && gameFrame - game_data.levelStartTime > 150) {
				for (i = 0; i < levelIcicles.numIcicles; i++) {
					if (levelIcicles.icicleArray[i].state != ICICLE_DEAD) {
						break;
					}
				}
				if (i == levelIcicles.numIcicles)
					SpawnIcicle(gameFrame);
			}
			
			if (levelEnemies.enemiesLeft == 0) {
				EndLevel();
			}
		} else { // bonus level
			if (levelCoins.coinsLeft == 0 || levelCoins.bonusTimer == 0) {
				EndLevel();
			} else if (gameFrame - game_data.levelStartTime > 150) {
				--levelCoins.bonusTimer;
			}
		}
		
		ManageFireballSpawning(&mario1, gameFrame, levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_HASFIREBALLS]);
		
		if (gameFrame - game_data.levelStartTime == 150) {
			if (levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_PLATFORMS] & PLATFORMS_ARE_INVISIBLE) {
				for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
					VanishPlatform(i);
				}
			}
		}
		
		// update movables
		if (!game_data.levelEnded)
			UpdatePlayer(&mario1, gameFrame);
		UpdateEnemies(&mario1, gameFrame);
		UpdateFireballs(&mario1, gameFrame);
		UpdateBonusCoins(&mario1, gameFrame);
		UpdateIcicles(&mario1, gameFrame);
		UpdateParticles(gameFrame);
		// draw
		DrawScene(&mario1, levelLog[game_data.level - 1][LEVEL_SETTINGS][1], gameFrame);
		++gameFrame;
	}
	
	return true;
}

bool LoadLevel(void) {
	PlayerInit(&mario1);
	LoadLevelFromSave();
	// we only need to draw the background once because we perform non-destructive sprite placement during the game loop in order to save on resources
	DrawBackground(levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_BACKGROUND]);
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
	
	// draw platform images
	RefreshPlatformBackgroundData(levelLog[game_data.level - 1][0][1]);
	
	// init level pow struct
	InitPows();
	
	// spawn in pows
	CreatePow(152, 176);
	CreatePow(152, 20);
	
	// init stuff
	InitEnemies();
	InitBonusData(); // coins & bonus data
	InitFireballs();
	InitIcicles();
	InitHud();
	InitParticles();
	
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
	game_data.levelEnded = false;
	
	TitleCardSetNumDigits(floor(log10(game_data.level) + 0.000002));
	
	if (levelLog[game_data.level - 1][0][2] & TOP_ICY) {
		FreezePlatform(5);
		FreezePlatform(6);
	}
	
	if (levelLog[game_data.level - 1][0][2] & MIDDLE_ICY) {
		FreezePlatform(3);
		FreezePlatform(4);
		FreezePlatform(2);
	}
	
	if (levelLog[game_data.level - 1][0][2] & BOTTOM_ICY) {
		FreezePlatform(1);
		FreezePlatform(0);
	}
	
	if (levelLog[game_data.level - 1][0][0]) { // if bonus level
		ResetPows();
		levelCoins.bonusTimer = 1200;
		SpawnBonusCoin(16, 80, true, 0, gameFrame);
		SpawnBonusCoin(300, 80, true, 0, gameFrame);
		SpawnBonusCoin(32, 80, true, 0, gameFrame);
		SpawnBonusCoin(274, 80, true, 0, gameFrame);
		
		SpawnBonusCoin(96, 190, true, 0, gameFrame);
		SpawnBonusCoin(224, 190, true, 0, gameFrame);
		
		SpawnBonusCoin(136, 138, true, 0, gameFrame);
		SpawnBonusCoin(195, 138, true, 0, gameFrame);
		
		SpawnBonusCoin(100, 16, true, 0, gameFrame);
		SpawnBonusCoin(220, 16, true, 0, gameFrame);
	}
	
	// change windows
	ChangeScreen(SCR_LEVEL);
	
	// return
	return true;
}

void UnloadLevel(void) {
	FreeEnemies();
	FreePlatforms();
	FreePows();
	FreeBonusCoins();
	FreeFireballs();
	FreeIcicles();
	FreeParticles();
}

void EndLevel(void) {
	// perform this action once
	if (!game_data.levelEnded) {
		if (levelCoins.coinsLeft == 0 && game_data.isBonusLevel)
			++mario1.lives;
		
		game_data.levelEndTime = gameFrame;
		game_data.levelEnded = true;
		ResetCoins();
		ResetFireballs();
		ResetEnemies(gameFrame);
		
		scoreWhenRoundStarts = mario1.score;
	}
	// wait a bit, then load the next level
	if (gameFrame - game_data.levelEndTime == 150) { // load next level
		// inc level
		++game_data.level;
		
		// try to reset particle array for optimization
		CleanUpParticleArray();
		
		// draw new bg
		DrawBackground(levelLog[game_data.level - 1][0][1]); // levelLog[game_data.level - 1][0][1] is level background id
		RefreshPlatformBackgroundData(levelLog[game_data.level - 1][0][1]);
		
		InitPipeBackgroundData();
		for (uint8_t i = 0; i < NUM_OF_PIPES; i++)
			RedrawPipesWithNewSprite(i, 0, 0);
		
		game_data.levelStartTime = gameFrame;
		game_data.levelEnded = false;
		game_data.isBonusLevel = levelLog[game_data.level - 1][0][0];
		
		// make sure that the hud knows the amount of digits in the level number
		TitleCardSetNumDigits(floor(log10(game_data.level) + 0.000002));
		
		if (levelLog[game_data.level - 1][0][2] & TOP_ICY) {
			FreezePlatform(5);
			FreezePlatform(6);
		}
		
		if (levelLog[game_data.level - 1][0][2] & MIDDLE_ICY) {
			FreezePlatform(3);
			FreezePlatform(4);
			FreezePlatform(2);
		}
		
		if (levelLog[game_data.level - 1][0][2] & BOTTOM_ICY) {
			FreezePlatform(1);
			FreezePlatform(0);
		}
		
		// reaccount for enemies
		uint8_t i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (levelLog[game_data.level - 1][2][i] == -1) {
				break;
			}
		}
		levelEnemies.enemiesLeft = i;
		levelEnemies.numEnemies = 0;
		
		// reset player
		mario1.x = 16;
		mario1.y = GROUND_HEIGHT - PLAYER_HEIGHT;
		mario1.dir = RIGHT;
		mario1.verAccel = mario1.horAccel = 0;
		mario1.deceleration = mario1.acceleration = 0.2;
		mario1.lastGroundedPlatformIndex = -1;
		
		if (levelLog[game_data.level - 1][0][0]) { // if bonus level
			ResetPows();
			levelCoins.bonusTimer = 1200;
			SpawnBonusCoin(16, 80, true, 0, gameFrame);
			SpawnBonusCoin(300, 80, true, 0, gameFrame);
			SpawnBonusCoin(32, 80, true, 0, gameFrame);
			SpawnBonusCoin(274, 80, true, 0, gameFrame);
			
			SpawnBonusCoin(96, 190, true, 0, gameFrame);
			SpawnBonusCoin(224, 190, true, 0, gameFrame);
			
			SpawnBonusCoin(136, 138, true, 0, gameFrame);
			SpawnBonusCoin(195, 138, true, 0, gameFrame);
			
			SpawnBonusCoin(100, 16, true, 0, gameFrame);
			SpawnBonusCoin(220, 16, true, 0, gameFrame);
		}
	}
}

void SaveCurrentLevel(void) {
	save_t newSave = GetSaveData();
	if (mario1.lives > 0) {
		newSave.livesLeft = mario1.lives;
		newSave.curLevel = game_data.level;
		newSave.curScore = scoreWhenRoundStarts;
		newSave.inLevel = true;
		newSave.collectedBonus = mario1.hasCollectedBonus;
	} else {
		newSave.inLevel = false;
		newSave.collectedBonus = false;
		newSave.livesLeft = 4;
		newSave.curLevel = 1;
		newSave.curScore = 0;
	}
	
	SaveCurrentData(newSave);
}

void LoadLevelFromSave(void) {
	save_t save = GetSaveData();
	if (save.inLevel) {
		game_data.level = save.curLevel;
		mario1.lives = save.livesLeft;
		mario1.score = save.curScore;
		mario1.hasCollectedBonus = save.collectedBonus;
		scoreWhenRoundStarts = save.curScore;
	}
}

void RestartLevels(void) {
	// pretty much a copy paste of EndLevel func with a few tweaks
	game_data.level = 1;
	mario1.score = 0;
	mario1.lives = 4;
	
	ResetParticleArray();
	ResetCoins();
	ResetEnemies(gameFrame);
	ResetFireballs();
	ResetPows();
	
	// draw new bg
	DrawBackground(levelLog[game_data.level - 1][0][1]); // levelLog[game_data.level - 1][0][1] is level background id
	RefreshPlatformBackgroundData(levelLog[game_data.level - 1][0][1]);
	
	InitPipeBackgroundData();
	for (uint8_t i = 0; i < NUM_OF_PIPES; i++)
		RedrawPipesWithNewSprite(i, 0, 0);
	
	game_data.levelStartTime = gameFrame;
	game_data.levelEnded = false;
	game_data.isBonusLevel = levelLog[game_data.level - 1][0][0];
	
	// make sure that the hud knows the amount of digits in the level number
	TitleCardSetNumDigits(floor(log10(game_data.level) + 0.000002));
	
	if (levelLog[game_data.level - 1][0][2] & TOP_ICY) {
		FreezePlatform(5);
		FreezePlatform(6);
	}
	
	if (levelLog[game_data.level - 1][0][2] & MIDDLE_ICY) {
		FreezePlatform(3);
		FreezePlatform(4);
		FreezePlatform(2);
	}
	
	if (levelLog[game_data.level - 1][0][2] & BOTTOM_ICY) {
		FreezePlatform(1);
		FreezePlatform(0);
	}
	
	// reaccount for enemies
	uint8_t i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (levelLog[game_data.level - 1][2][i] == -1) {
			break;
		}
	}
	levelEnemies.enemiesLeft = i;
	levelEnemies.numEnemies = 0;
	
	// reset player
	mario1.x = 16;
	mario1.y = GROUND_HEIGHT - PLAYER_HEIGHT;
	mario1.dir = RIGHT;
	mario1.verAccel = mario1.horAccel = 0;
	mario1.deceleration = mario1.acceleration = 0.2;
	mario1.lastGroundedPlatformIndex = -1;
	
	if (levelLog[game_data.level - 1][0][0]) { // if bonus level
		ResetPows();
		levelCoins.bonusTimer = 1200;
		SpawnBonusCoin(16, 80, true, 0, gameFrame);
		SpawnBonusCoin(300, 80, true, 0, gameFrame);
		SpawnBonusCoin(32, 80, true, 0, gameFrame);
		SpawnBonusCoin(274, 80, true, 0, gameFrame);
		
		SpawnBonusCoin(96, 190, true, 0, gameFrame);
		SpawnBonusCoin(224, 190, true, 0, gameFrame);
		
		SpawnBonusCoin(136, 138, true, 0, gameFrame);
		SpawnBonusCoin(195, 138, true, 0, gameFrame);
		
		SpawnBonusCoin(100, 16, true, 0, gameFrame);
		SpawnBonusCoin(220, 16, true, 0, gameFrame);
	}
}