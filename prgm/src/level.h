#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t numPlatforms;
} gameData_t;

bool LevelLoop(void);
bool LoadLevel(void);
void UnloadLevel(void);