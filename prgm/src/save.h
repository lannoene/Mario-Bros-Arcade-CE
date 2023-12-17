#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 9 // 8 + null terminator

typedef struct {
	bool inLevel, collectedBonus;
	unsigned int highScore, curScore;
	uint8_t highLevel, curLevel, livesLeft;
	char highScoreName[MAX_NAME_LENGTH];
} save_t;

void SaveCurrentData(save_t data);
save_t GetSaveData(void);