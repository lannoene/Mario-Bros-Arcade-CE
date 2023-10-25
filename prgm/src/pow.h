#pragma once
#include <stdint.h>

#include "player.h"

#define POW_SIZE 16

typedef struct {
	int16_t x;
	uint8_t y, state;
	uint8_t backgroundData[POW_SIZE*POW_SIZE + 2];
} pow_t;

typedef struct {
	pow_t* powArray;
	uint8_t numPows;
} powInfo_t;

enum POW_STATES {
	POW_FULL = 0,
	POW_MEDUM,
	POW_LOW,
	POW_EMPTY
};

extern powInfo_t levelPows;

void InitPows(void);
void CreatePow(int16_t x, uint8_t y);
void FreePows(void);
void BumpPow(player_t* player, uint8_t powIndex, unsigned int gameFrame);
void ResetPows(void);