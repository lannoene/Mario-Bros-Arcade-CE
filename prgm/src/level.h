#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t numPlatforms;
	uint8_t level;
	unsigned int levelStartTime, levelEndTime;
	bool levelEnded, isBonusLevel, paused : 1;
} gameData_t;

enum level_settings {
	LVL_ISBONUS = 0,
	LVL_BACKGROUND,
	LVL_PLATFORMS,
	LVL_HASFIREBALLS,
	LVL_HASFREEZIES
};

extern gameData_t game_data;

bool LevelLoop(void);
bool LoadLevel(void);
void UnloadLevel(void);
void EndLevel(void);
void RestartLevels(void);