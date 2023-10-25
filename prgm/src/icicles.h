#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "player.h"

#define ICICLE_WIDTH	14
#define ICICLE_HEIGHT	21

typedef struct {
	uint8_t y, y_old, state, sprite;
	int16_t x, x_old;
	float verAccel;
	unsigned int spawnTime;
	uint8_t backgroundData[ICICLE_WIDTH*ICICLE_HEIGHT + 2];
} icicle_t;

typedef struct {
	icicle_t* icicleArray;
	uint8_t numIcicles;
} levelIcicleData_t;

enum icicle_states {
	ICICLE_FORMING,
	ICICLE_FALLING,
	ICICLE_DEAD
};

extern levelIcicleData_t levelIcicles;

void InitIcicles(void);
void SpawnIcicle(unsigned int gameFrame);
void UpdateIcicles(player_t* player, unsigned int gameFrame);
void FreeIcicles(void);