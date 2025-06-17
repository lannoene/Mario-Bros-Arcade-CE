#include "level.h"

#include <keypadc.h>
#include <graphx.h>
#include <math.h>
#include <string.h>
#include <debug.h>
#include <sys/util.h>
#include <debug.h>

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
#include "phases.h"

#define LEVEL_BONUS	1
#define LEVEL_GAME	0

#define LEVEL_SETTINGS 0
#define LEVEL_ENEMYTYPE 1
#define LEVEL_ENEMYSPAWNTIME 2

#define FPS	60
#define LEVEL_FADE_DURATION 8

unsigned int gameFrame = 0;
player_t mario[MAX_PLAYERS];
gameData_t game_data = {
	.numPlatforms = 0,
	.level = 1,
	.levelStartTime = 0,
	.levelEndTime = 999999,
	.dtLevelStart = 0,
	.curEnemyWave = 0,
	.curWaveSpawnFlags = 0,
	.wavePointsLeft = 0,
	.wavePointsMin = 0,
	.nextWaveStart = 0,
	.enemySpawnWait = 1*FPS,
	.waveCutShort = true,
	.hasNextWave = true,
	.numPlayers = 1,
	.levelEnded = false,
	.levelStarted = false,
	.isBonusLevel = false,
	.paused = false,
	.spawningEnemies = false,
};
unsigned int scoreWhenRoundStarts = 0; // prevent infinite score

game_level_t *curLevelData = NULL;

void SaveCurrentLevel(void);
void LoadLevelFromSave(void);
void ManageEnemySpawns(void);
void LoadEnemyWave(void);
void SpawnWaveEnemy(void);

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
		DrawScene(mario, curLevelData->bg, gameFrame);
	} else {
		if (game_data.levelStarted) {
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
				PlayerMove(&mario[0], NONE, gameFrame); // deaccelerate
			}
			if (kb_Data[1] & kb_2nd) {
				PlayerMove(&mario[0], UP, gameFrame); // jump
			} else {
				PlayerMove(&mario[0], NOJUMP, gameFrame);
			}
			// check player 2 controls
			
			if (kb_Data[3] & kb_7) {
				PlayerMove(&mario[1], RIGHT, gameFrame);
			} else if (kb_Data[2] & kb_Log) {
				PlayerMove(&mario[1], LEFT, gameFrame);
			} else {
				PlayerMove(&mario[1], NONE, gameFrame); // deaccelerate
			}
			if (kb_Data[6] & kb_Mul) {
				PlayerMove(&mario[1], UP, gameFrame); // jump
			} else {
				PlayerMove(&mario[1], NOJUMP, gameFrame);
			}
		} else {
			game_data.levelStarted = game_data.dtLevelStart == LEVEL_LOAD_DURATION + 1;
			for (int i = 0; i < game_data.numPlayers; i++)
				PlayerMove(&mario[i], NONE, 0);
			
			if (game_data.dtLevelStart < LEVEL_FADE_DURATION + 1) { // fade in effect at the beginning of the level
				SetDarkness(7 - ((7*(game_data.dtLevelStart))/LEVEL_FADE_DURATION));
			} else if (game_data.dtLevelStart == LEVEL_FADE_DURATION + 1) {
				SetDarkness(0);
			}
		}
		
		// run level stuff
		if (!curLevelData->isBonus) { // normal level
			ManageEnemySpawns();
			if (!game_data.spawningEnemies && levelEnemies.enemiesLeft == 0) {
				EndLevel();
			}
		} else { // bonus level
			if (levelCoins.coinsLeft == 0 || levelCoins.bonusTimer == 0) {
				EndLevel();
			} else if (game_data.levelStarted) {
				--levelCoins.bonusTimer;
			}
		}
		
		
		ManageFireballSpawning(&mario[0], gameFrame, curLevelData->fireballFlags);
		
		
		if (game_data.dtLevelStart == LEVEL_LOAD_DURATION) {
			if (curLevelData->platformFlags & PLATFORMS_ARE_INVISIBLE) {
				for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
					//dbg_printf("Vanishing platform...\n");
					VanishPlatform(i);
				}
			}
		}
		// update movables
		if (!game_data.levelEnded) {
			UpdatePlayers(&mario[0], gameFrame);
		}
		
		UpdateEnemies(gameFrame);
		UpdateFireballs(&mario[0], gameFrame);
		UpdateIcicles(mario, gameFrame);
		UpdateParticles(gameFrame);
		// draw
		DrawScene(mario, curLevelData->bg, gameFrame);
		UpdatePlatforms(gameFrame);
		
		++gameFrame;
		++game_data.dtLevelStart;
	}
	
	return true;
}

void ReadyLevel(void) {
	curLevelData = &levelLog[game_data.level - 1];
	// try to reset particle array for optimization
	CleanUpParticleArray();
	
	// draw new bg
	DrawBackground(curLevelData->bg); // levelLog[game_data.level - 1][0][1] is level background id
	RefreshPlatformBackgroundData(curLevelData->bg);
	
	// add static hud objects (to save on draw time)
	HudAddStaticObjects();
	
	InitPipeBackgroundData();
	for (uint8_t i = 0; i < NUM_OF_PIPES; i++)
		RedrawPipesWithNewSprite(i, 0, 0);
	
	game_data.levelStartTime = gameFrame;
	game_data.dtLevelStart = gameFrame - game_data.levelStartTime;
	game_data.levelEnded = false;
	game_data.levelStarted = false;
	game_data.spawningEnemies = true;
	game_data.isBonusLevel = curLevelData->isBonus;
	
	// make sure that the hud knows the amount of digits in the level number
	TitleCardSetNumDigits(iLog10(game_data.level));
	
	if (curLevelData->platformFlags & TOP_ICY) {
		FreezePlatformIdx(5);
		FreezePlatformIdx(6);
	}
	
	if (curLevelData->platformFlags & MIDDLE_ICY) {
		FreezePlatformIdx(3);
		FreezePlatformIdx(4);
		FreezePlatformIdx(2);
	}
	
	if (curLevelData->platformFlags & BOTTOM_ICY) {
		FreezePlatformIdx(1);
		FreezePlatformIdx(0);
	}
	
	uint8_t i;
	
	game_data.curEnemyWave = 0;
	LoadEnemyWave();
	
	levelEnemies.lastSpawnedPipe = randInt(0, 1);
	levelEnemies.enemiesLeft = 0;
	
	// reset player
	for (i = 0; i < game_data.numPlayers; i++) {
		mario[i].x = I2FP(16) + i*I2FP(240); // if i is 1 (player 2), then he will be on the other side
		mario[i].y = I2FP(GROUND_HEIGHT - PLAYER_HEIGHT);
		mario[i].dir = RIGHT;
		mario[i].verAccel = mario[i].horAccel = 0;
		mario[i].deceleration = mario[i].acceleration = I2FP(0.2);
		mario[i].lastGroundedPlatform = NULL;
	}
	
	if (curLevelData->isBonus) {
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
	dbg_printf("loading level\n");
	
	dbg_printf("initializing marios\n");
	for (int i = 0; i < game_data.numPlayers; i++)
		PlayerInit(&mario[i]);
	
	dbg_printf("loading save\n");
	LoadLevelFromSave();
	// init the bg for pipes. they're stationary, so we don't need to refresh the bg slice in order to save resources
	dbg_printf("initializing pipe data\n");
	InitPipeBackgroundData();
	
	// init level platform struct
	dbg_printf("initializing platform data\n");
	InitPlatformData();
	
	
	dbg_printf("creating platforms\n");
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
	dbg_printf("initializing pows\n");
	InitPows();
	
	// spawn in pows
	CreatePow(152, 176);
	CreatePow(152, 10);
	
	// init stuff
	dbg_printf("initializing extra...\n");
	InitEnemies();
	InitBonusData(); // coins & bonus data
	InitFireballs();
	InitIcicles();
	InitHud();
	InitParticles();
	
	ReadyLevel();
	
	// change windows
	ChangeScreen(SCR_LEVEL);
	
	dbg_printf("finished loading level\n");
	
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
	lvl_s_reset();
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
		game_data.levelStarted = false;
		ResetFireballs();
		ResetEnemies(gameFrame, false);
		ResetIcicles();
		
		scoreWhenRoundStarts = mario[0].score;
	}
	
	const unsigned int dtLevelEnd = gameFrame - game_data.levelEndTime;
	if (dtLevelEnd > LEVEL_END_DURATION - LEVEL_FADE_DURATION && dtLevelEnd != LEVEL_END_DURATION) { // fade out
		SetDarkness((dtLevelEnd - (LEVEL_END_DURATION - LEVEL_FADE_DURATION))/(float)(LEVEL_FADE_DURATION/7));
	} else if (dtLevelEnd == LEVEL_END_DURATION) { // wait a bit, then load the next level while the screen is black
		SetDarkness(8);
		// inc level
		game_data.level = MIN(game_data.level + 1, ARR_LEN(levelLog));
		
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
	scoreWhenRoundStarts = mario[0].score = 0;
	mario[0].lives = 4;
	mario[0].state = PLAYER_NORMAL;
	
	ResetParticleArray();
	ResetEnemies(gameFrame, true);
	ResetFireballs();
	ResetPows();
	ResetIcicles();
	
	ReadyLevel();
}

void ManageEnemySpawns(void) {
	#define ENEMIES_LEFT_TO_SPAWN (game_data.wavePointsLeft >= game_data.wavePointsMin)
	
	if (!game_data.levelStarted)
		return;
	
	// manage enemy spawning
	if (ENEMIES_LEFT_TO_SPAWN) {
		if (gameFrame - levelEnemies.lastSpawnedTime >= game_data.enemySpawnWait) {
			SpawnWaveEnemy();
		}
	} else if (!game_data.waveCutShort && levelEnemies.enemiesLeft == 0) {
		// cut wave short
		unsigned int newWaveStart = gameFrame + FPS*2;
		if (newWaveStart < game_data.nextWaveStart)
			game_data.nextWaveStart = newWaveStart;
		game_data.waveCutShort = true;
		dbg_printf("Wave was cut short\n");
		if (!game_data.hasNextWave) {
			// end level
			game_data.spawningEnemies = false;
		}
	}
	
	if (curLevelData->hasFreezies && randInt(0, 999) == 0) {
		FOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
			if (enemy->state != ENEMY_DEAD && enemy->type == ENEMY_FREEZIE) {
				break;
			}
		} FOR_ELSE {
			// make sure only one freezie max can go on field at a time
			SpawnEnemy(ENEMY_FREEZIE, (rand() % 2), gameFrame);
		}
	}
	
	if (curLevelData->maxIcicles && randInt(0, 99/curLevelData->maxIcicles) == 0) {
		int iciclesFound = 0;
		FFOR_EACH(levelIcicles.icicleArray, levelIcicles.numIcicles, icicle) {
			if (icicle->state != ICICLE_DEAD) {
				++iciclesFound;
			}
		}
		if (iciclesFound < curLevelData->maxIcicles)
			SpawnIcicle(gameFrame);
	}
	
	if (game_data.spawningEnemies) {
		// check if we need to start a new wave
		if (game_data.nextWaveStart <= gameFrame) {
			// empty all remaining wave enemies
			while (ENEMIES_LEFT_TO_SPAWN) {
				SpawnWaveEnemy();
			}
			if (!game_data.hasNextWave) {
				dbg_printf("cannot spawn any more enemies\n");
				game_data.spawningEnemies = false;
			} else {
				// go to next wave
				game_data.curEnemyWave++;
				LoadEnemyWave();
			}
		}
	}
	#undef ENEMIES_LEFT_TO_SPAWN
}

#define RAND_OFFSET(spread) ((randInt(0, +spread - 1)) - (+spread)/2)

void LoadEnemyWave(void) {
	struct enemy_wave_t *wave = &curLevelData->enemyWaves[game_data.curEnemyWave];
	game_data.wavePointsMin = 0;
	game_data.curWaveSpawnFlags =
		wave->enemySpawnFlags
		? wave->enemySpawnFlags
		: curLevelData->defaultSpawnFlags;
	game_data.waveCutShort = false;
	game_data.hasNextWave = curLevelData->enemyWaves[game_data.curEnemyWave + 1].totalPoints != -1;
	levelEnemies.lastSpawnedTime = 0;
	game_data.enemySpawnWait = 0;
	
	for (uint8_t i = 0; i < ARR_LEN(levelEnemyInfo); i++) {
		if (game_data.curWaveSpawnFlags & levelEnemyInfo[i].enemyId) {
			game_data.wavePointsMin = levelEnemyInfo[i].spawnCost;
			break;
		}
	}
	game_data.wavePointsLeft = wave->totalPoints;
	uint16_t totalWaveDuration = curLevelData->waveDuration*FPS + RAND_OFFSET(curLevelData->waveDuration*(FPS*20)/100);
	game_data.nextWaveStart = gameFrame + totalWaveDuration;
	if (game_data.nextWaveStart < gameFrame)
		game_data.nextWaveStart = gameFrame;
	
	dbg_printf("new wave (%d). duration: %d, spawn flags: %d\n", game_data.curEnemyWave, totalWaveDuration, game_data.curWaveSpawnFlags);
}

void SpawnWaveEnemy(void) {
	game_data.enemySpawnWait = curLevelData->enemyWaves[game_data.curEnemyWave].enemySpawnWait*FPS;
	const uint8_t spawnSpread = 20;
	if (curLevelData->enemyWaves[game_data.curEnemyWave].enemySpawnWait > spawnSpread/2)
		game_data.enemySpawnWait += RAND_OFFSET(spawnSpread);
	uint8_t spawnableEnemyIndexes[ARR_LEN(levelEnemyInfo)];
	// pick enemy at random
	uint8_t numSpawnable = 0;
	for (uint8_t i = 0; i < ARR_LEN(levelEnemyInfo); i++) {
		if (levelEnemyInfo[i].autospawn
			&& (levelEnemyInfo[i].enemyId & game_data.curWaveSpawnFlags)) {
			spawnableEnemyIndexes[numSpawnable] = i;
			numSpawnable++;
		}
	}
	if (!numSpawnable) {
		dbg_printf("There are no spawnable enemies this wave, but wave points were allocated. Was this an error?\n");
		// prevent game from locking up in next wave start while loop
		game_data.wavePointsLeft = 0;
		game_data.wavePointsMin = 1;
		return;
	}
	uint8_t enemyToSpawn = randInt(0, numSpawnable - 1);
	// now check if we have enough points
	while (levelEnemyInfo[spawnableEnemyIndexes[enemyToSpawn]].spawnCost > game_data.wavePointsLeft) {
		enemyToSpawn--;
	}
	
	// we have our enemy
	uint8_t spawnEnemyIdx = spawnableEnemyIndexes[enemyToSpawn];
	SpawnEnemy(levelEnemyInfo[spawnEnemyIdx].enemyId, (rand() % 2), gameFrame);
	game_data.wavePointsLeft -= levelEnemyInfo[spawnEnemyIdx].spawnCost;
	levelEnemies.enemiesLeft++;
	dbg_printf("spawning enemy: d time: %d, points cost: %d, wave points left: %d\n",
		gameFrame - levelEnemies.lastSpawnedTime,
		levelEnemyInfo[spawnEnemyIdx].spawnCost,
		game_data.wavePointsLeft);
}

DEF_ALLOC_TABLE(lvl, 12500)