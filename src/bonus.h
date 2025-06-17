#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define COIN_WIDTH	8
#define COIN_WIDTH_FP	8*256 // fixed point version
#define COIN_IMAGE_WIDTH 11
#define COIN_HEIGHT	11
#define COIN_HEIGHT_FP	11*256

typedef struct {
	int x, x_old;
	int y, y_old;
	uint8_t sprite, state, lastGroundedPlatformIndex;
	int verSpriteOffset, horSpriteOffset;
	bool alive, shouldDie, bonus, dir, grounded, firstTimeSpawning : 1;
	int verAccel, horAccel;
	unsigned int spawnFrame, pipeExitFrame, deathFrame;
	uint8_t backgroundData[COIN_WIDTH*COIN_HEIGHT + 2];
} bonusCoin_t;

typedef struct {
	uint8_t coinsLeft;
	uint16_t bonusTimer;
	bonusCoin_t* coinArray;
} bonusLevel_t;

enum COIN_STATES {
	COIN_NORMAL = 0,
	COIN_EXITING_PIPE,
	COIN_DEAD_SCORE,
	COIN_DEAD
};

extern bonusLevel_t levelCoins;

void InitBonusData(void);
//void SpawnBonusCoin(int16_t x, uint8_t y, bool bonus, bool dir, unsigned int gameFrame);
//void ResetCoins(void);
//void FreeBonusCoins(void);
//void UpdateBonusCoins(player_t* player, unsigned int gameFrame);
void HudAddStaticObjects(void);