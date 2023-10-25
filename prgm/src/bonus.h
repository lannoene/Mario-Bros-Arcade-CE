#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define COIN_WIDTH	8
#define COIN_IMAGE_WIDTH 11
#define COIN_HEIGHT	11

typedef struct {
	float x, x_old;
	uint8_t y, y_old, sprite, state, lastGroundedPlatformIndex;
	int8_t verSpriteOffset, horSpriteOffset;
	bool alive, bonus, dir, grounded, firstTimeSpawning : 1;
	float verAccel, horAccel;
	unsigned int spawnFrame, pipeExitFrame, deathFrame;
	uint8_t backgroundData[COIN_WIDTH*COIN_HEIGHT + 2];
} bonusCoin_t;

typedef struct {
	uint8_t numCoins, coinsLeft;
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
void SpawnBonusCoin(int16_t x, uint8_t y, bool bonus, bool dir, unsigned int gameFrame);
void ResetCoins(void);
void FreeBonusCoins(void);
void UpdateBonusCoins(player_t* player, unsigned int gameFrame);