#include "level.h"

#include <keypadc.h>
#include <graphx.h>
#include <math.h>
#include <string.h>
#include <debug.h>
#include <sys/util.h>

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
#include "defines.h"

#define MAX_ENEMIES 10

#define LEVEL_BONUS	1
#define LEVEL_GAME	0

#define LEVEL_SETTINGS 0
#define LEVEL_ENEMYTYPE 1
#define LEVEL_ENEMYSPAWNTIME 2

#define FPS	60
#define LEVEL_FADE_DURATION 8

unsigned int gameFrame = 0;
player_t mario[MAX_PLAYERS];
gameData_t game_data = {0, 1, 0, 999999, 1, false, false, false};
unsigned int scoreWhenRoundStarts = 0; // prevent infinite score
// arr part 1 is level index, pt 2 is switch between enemy id/enemy spawn time, pt 3 is spawn track
int16_t levelLog[][3][MAX_ENEMIES] = {
	{
		{false, BG_PIPES, NONE_ICY, false, false}, // level settings
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0, 0, 0}, // enemy spawn type
		{200, 500, 800, -1, -1, -1, -1, -1, -1, -1} // enemy spawn time in frames
	},
	{
		{false, BG_PIPES, NONE_ICY, false, false},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0, 0},
		{200, 400, 600, 800, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_PIPES, NONE_ICY, false, false},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0},
		{250, 350, 450, 600, 750, -1, -1, -1, -1}
	},
	{
		{true, BG_SNOWY, NONE_ICY, false, false},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-1, -1, -1, -1, -1, -1, -1, -1, -1}
	},
	{
		{false, BG_LAVA, NONE_ICY, false, false},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0, 0, 0},
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
		{false, BG_SNOWY, TOP_ICY, HAS_FIREBALL_GREEN, true, 1},
		{ENEMY_SPIKE, ENEMY_SPIKE, ENEMY_CRAB, ENEMY_SPIKE, ENEMY_SPIKE, 0, 0, 0, 0, 0},
		{3*FPS, 6*FPS, 14*FPS, 15*FPS, 16*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY, HAS_FIREBALL_GREEN, true, 1},
		{ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, 0, 0, 0, 0, 0},
		{3*FPS, 5*FPS, 12*FPS, 13*FPS, 19*FPS, -1, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY, HAS_FIREBALL_GREEN, true, 2},
		{ENEMY_FLY, ENEMY_CRAB, ENEMY_CRAB, ENEMY_CRAB, ENEMY_FLY, ENEMY_CRAB, 0, 0, 0, 0},
		{3*FPS, 4*FPS, 15*FPS, 16*FPS, 22*FPS, 25*FPS, -1, -1, -1}
	},
	{
		{false, BG_SNOWY, TOP_ICY, HAS_FIREBALL_GREEN, true, 3},
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
			mario[0].hasJumpedThisFrame = true; // prevent mario from jumping if the menu closes
			switch (PauseScreenInputEvent(4)) {
				case 0: // resume
					game_data.paused = false;
					return true;
				case 1: // save & quit game
					// make sure everything is freed before we exit
					SaveCurrentLevel();
					UnloadLevel();
					return false;
				case 2: // quit game
					UnloadLevel();
					return false;
				case 3: // restart game
					game_data.paused = false;
					RestartLevels();
					return true;
			}
		}
		// draw
		DrawScene(mario, levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_BACKGROUND], gameFrame);
	} else {
		if (gameFrame - game_data.levelStartTime > 150 && !game_data.levelEnded) {
			// check for button presses
			if (kb_Data[6] & kb_Clear) {
				game_data.paused = true;
				PauseScreenResetCursorPos(); // make sure the cursor always starts at the top
				return true;
			}
			// check player1 controls
			if (kb_Data[7] & kb_Right) {
				PlayerMove(&mario[0], RIGHT, gameFrame);
			} else if (kb_Data[7] & kb_Left) {
				PlayerMove(&mario[0], LEFT, gameFrame);
			} else {
				PlayerMove(&mario[0], NONE, 0); // deaccelerate
			}
			if (kb_Data[1] & kb_2nd) {
				PlayerMove(&mario[0], UP, 0); // jump
			} else {
				PlayerMove(&mario[0], NOJUMP, 0);
			}
			// check player 2 controls
			
			if (kb_Data[3] & kb_7) {
				PlayerMove(&mario[1], RIGHT, gameFrame);
			} else if (kb_Data[2] & kb_Log) {
				PlayerMove(&mario[1], LEFT, gameFrame);
			} else {
				PlayerMove(&mario[1], NONE, 0); // deaccelerate
			}
			if (kb_Data[6] & kb_Mul) {
				PlayerMove(&mario[1], UP, 0); // jump
			} else {
				PlayerMove(&mario[1], NOJUMP, 0);
			}
		} else {
			for (int i = 0; i < game_data.numPlayers; i++)
				PlayerMove(&mario[i], NONE, 0);
		}
		
		if (gameFrame - game_data.levelStartTime < LEVEL_FADE_DURATION + 1) { // fade in effect at the beginning of the level
			SetDarkness(7 - ((7*(gameFrame - game_data.levelStartTime))/LEVEL_FADE_DURATION));
		} else if (gameFrame - game_data.levelStartTime == LEVEL_FADE_DURATION + 1) {
			SetDarkness(0);
		}
		
		// run level stuff
		if (!levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_ISBONUS]) { // normal level
			uint8_t i;
			for (i = 0; i < MAX_ENEMIES; i++) {
				if (levelLog[game_data.level - 1][LEVEL_ENEMYSPAWNTIME][i] > 0 && levelLog[game_data.level - 1][LEVEL_ENEMYSPAWNTIME][i] == (int)(gameFrame - game_data.levelStartTime)) {
					SpawnEnemy(levelLog[game_data.level - 1][LEVEL_ENEMYTYPE][i], (rand() % 2), gameFrame);
					break;
				}
			}
			if (levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_HASFREEZIES] && randInt(0, 999) == 0 && !game_data.levelEnded && gameFrame - game_data.levelStartTime > 150) {
				for (i = 0; i < levelEnemies.numEnemies; i++) {
					if (levelEnemies.enemyArray[i].state != ENEMY_DEAD && levelEnemies.enemyArray[i].type == ENEMY_FREEZIE) {
						break;
					}
				}
				if (i == levelEnemies.numEnemies) // make sure only one freezie max can go on field at a time
					SpawnEnemy(ENEMY_FREEZIE, (rand() % 2), gameFrame);
			}
			int m = levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_MAXICICLES];
			if (m && randInt(0, 99/m) == 0 && !game_data.levelEnded && gameFrame - game_data.levelStartTime > 150) {
				int iciclesFound = 0;
				for (i = 0; i < levelIcicles.numIcicles; i++) {
					if (levelIcicles.icicleArray[i].state != ICICLE_DEAD) {
						++iciclesFound;
					}
				}
				if (iciclesFound < levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_MAXICICLES])
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
		
		ManageFireballSpawning(&mario[0], gameFrame, levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_HASFIREBALLS]);
		
		if (gameFrame - game_data.levelStartTime == 150) {
			if (levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_PLATFORMS] & PLATFORMS_ARE_INVISIBLE) {
				for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
					VanishPlatform(i);
				}
			}
		}
		// update movables
		if (!game_data.levelEnded) {
			for (int i = 0; i < game_data.numPlayers; i++) {
				UpdatePlayer(&mario[i], gameFrame);
			}
		}
		UpdateEnemies(gameFrame);
		UpdateFireballs(&mario[0], gameFrame);
		UpdateIcicles(mario, gameFrame);
		UpdateParticles(gameFrame);
		// draw
		DrawScene(mario, levelLog[game_data.level - 1][LEVEL_SETTINGS][1], gameFrame);
		UpdatePlatforms(gameFrame);
		++gameFrame;
	}
	
	return true;
}

void ReadyLevel(void) {
	// try to reset particle array for optimization
	CleanUpParticleArray();
	
	// draw new bg
	DrawBackground(levelLog[game_data.level - 1][LEVEL_SETTINGS][LVL_BACKGROUND]); // levelLog[game_data.level - 1][0][1] is level background id
	RefreshPlatformBackgroundData(levelLog[game_data.level - 1][0][1]);
	
	// add static hud objects (to save on draw time)
	HudAddStaticObjects();
	
	InitPipeBackgroundData();
	for (uint8_t i = 0; i < NUM_OF_PIPES; i++)
		RedrawPipesWithNewSprite(i, 0, 0);
	
	game_data.levelStartTime = gameFrame;
	game_data.levelEnded = false;
	game_data.isBonusLevel = levelLog[game_data.level - 1][0][0];
	
	// make sure that the hud knows the amount of digits in the level number
	TitleCardSetNumDigits(iLog10(game_data.level));
	
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
	levelEnemies.lastSpawnedPipe = randInt(0, 1);
	
	// reset player
	for (i = 0; i < game_data.numPlayers; i++) {
		mario[i].x = I2FP(16) + i*I2FP(240); // if i is 1 (player 2), then he will be on the other side
		mario[i].y = I2FP(GROUND_HEIGHT - PLAYER_HEIGHT);
		mario[i].dir = RIGHT;
		mario[i].verAccel = mario[i].horAccel = 0;
		mario[i].deceleration = mario[i].acceleration = I2FP(0.2);
		mario[i].lastGroundedPlatformIndex = -1;
	}
	
	if (levelLog[game_data.level - 1][0][0]) { // if bonus level
		ResetPows();
		levelCoins.bonusTimer = 1200;
		SpawnBonusCoin(16, 80, true, gameFrame);
		SpawnBonusCoin(300, 80, true, gameFrame);
		SpawnBonusCoin(32, 80, true, gameFrame);
		SpawnBonusCoin(274, 80, true, gameFrame);
		
		SpawnBonusCoin(96, 190, true, gameFrame);
		SpawnBonusCoin(224, 190, true, gameFrame);
		
		SpawnBonusCoin(136, 138, true, gameFrame);
		SpawnBonusCoin(195, 138, true, gameFrame);
		
		SpawnBonusCoin(100, 16, true, gameFrame);
		SpawnBonusCoin(220, 16, true, gameFrame);
	}
}

bool LoadLevel(void) {
	for (int i = 0; i < game_data.numPlayers; i++)
		PlayerInit(&mario[i]);
	
	LoadLevelFromSave();
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
	CreatePow(152, 20);
	
	// init stuff
	InitEnemies();
	InitBonusData(); // coins & bonus data
	InitFireballs();
	InitIcicles();
	InitHud();
	InitParticles();
	
	ReadyLevel();
	
	// change windows
	ChangeScreen(SCR_LEVEL);
	
	// return
	return true;
}

void UnloadLevel(void) {
	FreeEnemies();
	FreePlatforms();
	FreePows();
	FreeFireballs();
	FreeIcicles();
	FreeParticles();
}

void EndLevel(void) {
	// perform this action once
	if (!game_data.levelEnded) {
		if (game_data.isBonusLevel) {
			if (levelCoins.coinsLeft == 0) { // special bonus for collecting all coins
				++mario[0].lives;
				PlayerAddScore(&mario[0], 5000);
			}
			PlayerAddScore(&mario[0], (10 - levelCoins.coinsLeft)*800); // add all the score for the coins
		}
		
		game_data.levelEndTime = gameFrame;
		game_data.levelEnded = true;
		ResetFireballs();
		ResetEnemies(gameFrame);
		ResetIcicles();
		
		scoreWhenRoundStarts = mario[0].score;
	}
	
	if (gameFrame - game_data.levelEndTime > 150 - LEVEL_FADE_DURATION && gameFrame - game_data.levelEndTime != 150) { // fade out
		SetDarkness((gameFrame - game_data.levelEndTime - (150 - LEVEL_FADE_DURATION))/(float)(LEVEL_FADE_DURATION/7));
	} else if (gameFrame - game_data.levelEndTime == 150) { // wait a bit, then load the next level while the screen is black
		SetDarkness(8);
		// inc level
		++game_data.level;
		
		ReadyLevel();
	}
}

void SaveCurrentLevel(void) {
	save_t newSave = GetSaveData();
	if (mario[0].lives > 0) {
		newSave.livesLeft = mario[0].lives;
		newSave.curLevel = game_data.level;
		newSave.curScore = scoreWhenRoundStarts;
		newSave.inLevel = true;
		newSave.collectedBonus = mario[0].hasCollectedBonus;
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
		mario[0].lives = save.livesLeft;
		mario[0].score = save.curScore;
		mario[0].hasCollectedBonus = save.collectedBonus;
		scoreWhenRoundStarts = save.curScore;
	}
}

void RestartLevels(void) {
	// pretty much a copy paste of EndLevel func with a few tweaks
	game_data.level = 1;
	mario[0].score = 0;
	mario[0].lives = 4;
	mario[0].state = PLAYER_NORMAL;
	
	ResetParticleArray();
	ResetEnemies(gameFrame);
	ResetFireballs();
	ResetPows();
	ResetIcicles();
	
	ReadyLevel();
}