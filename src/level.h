#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "alloc_table.h"

#define LEVEL_LOAD_DURATION 150
#define LEVEL_END_DURATION 150

typedef struct {
	// level appearance
	uint8_t numPlatforms;
	uint8_t level;
	unsigned int levelStartTime, levelEndTime, dtLevelStart;
	// wave
	uint8_t curEnemyWave;
	uint8_t curWaveSpawnFlags;
	uint8_t wavePointsLeft;
	uint8_t wavePointsMin; // the minimum amount of wave points that must be spent when spawning an enemy this round
	unsigned int nextWaveStart; // game frame of next wave start
	uint16_t enemySpawnWait; // how many frames should we wait before spawning a new enemy
	bool waveCutShort, hasNextWave;
	// players
	int numPlayers;
	// level flags
	// level started: active only during gameplay, level ended: active only during level transitions
	bool levelEnded : 1;
	bool levelStarted : 1;
	bool isBonusLevel : 1;
	bool paused : 1;
	bool spawningEnemies : 1; // set to false when no more enemies can be spawned (all waves are finished & all enemies are spawned)
} gameData_t;

extern gameData_t game_data;

bool LevelLoop(void);
bool LoadLevel(void);
void UnloadLevel(void);
void EndLevel(void);
void RestartLevels(void);

EXPORT_ALLOC_TABLE(lvl)