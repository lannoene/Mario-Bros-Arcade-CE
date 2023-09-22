#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t numPlatforms;
} gameData_t;

bool GameLoop(void);
bool LoadGame(void);
