#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define ENEMY_SPIKE_SIZE 16
#define ENEMY_SPIKE_HITBOX_HEIGHT 12

typedef struct {
	bool dir, grounded, crabIsMad : 1; // crabIsMad is only for crab type enemies
	uint8_t type, state, sprite, lastGroundedPlatformIndex;
	float x, x_old;
	int16_t y, y_old;
	unsigned int spawnTime, layStartTime;
	float horAccel, verAccel;
	int8_t verSpriteOffset, verSpriteOffset_old;
	float maxSpeed;
	uint8_t backgroundData[ENEMY_SPIKE_SIZE*ENEMY_SPIKE_SIZE + 2];
} enemy_t;

typedef struct {
	uint8_t numEnemies, enemiesLeft;
	enemy_t* enemyArray;
} levelEnemies_t;

enum ENEMY_TYPE_IDS {
	ENEMY_SPIKE = 0,
	ENEMY_CRAB
};

enum ENEMY_STATE_IDS {
	ENEMY_WALKING,
	ENEMY_TURNING,
	ENEMY_LAYING,
	ENEMY_DEAD_SPINNING,
	ENEMY_DEAD,
	ENEMY_EXITING_PIPE,
	ENEMY_ENTERING_PIPE,
};

extern levelEnemies_t levelEnemies;

void InitEnemies(void);
void SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame);
void FreeEnemies(void);
void UpdateEnemies(player_t* player, unsigned int gameFrame);