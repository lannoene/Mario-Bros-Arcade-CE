#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define COIN_WIDTH	8
#define COIN_HEIGHT	11

typedef struct {
	int16_t x;
	uint8_t y;
	bool alive;
	uint8_t backgroundData[COIN_WIDTH*COIN_HEIGHT + 2];
} bonusCoin_t;

typedef struct {
	uint8_t numCoins, coinsLeft;
	uint16_t bonusTimer;
	bonusCoin_t* coinArray;
} bonusLevel_t;

extern bonusLevel_t levelCoins;

void InitBonusData(void);
void SpawnBonusCoin(int16_t x, uint8_t y);
void CheckCoinColision(player_t* player);
void ResetCoins(void);
void FreeBonusCoins(void);