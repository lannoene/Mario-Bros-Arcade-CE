#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t numPlatforms;
	uint8_t level;
} gameData_t;

bool LevelLoop(void);
bool LoadLevel(void);
void UnloadLevel(void);
void EndLevel(void);