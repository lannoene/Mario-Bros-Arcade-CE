#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define ENEMY_SPIKE_SIZE 16
#define ENEMY_SPIKE_HITBOX_HEIGHT 12
#define ENEMY_FREEZIE_WIDTH 12

#define SpawnBonusCoin(sx, sy, b, f)\
do {\
	enemy_t *c = SpawnEnemy(ENEMY_COIN, false, f);\
	c->bonus = b;\
	if (b) {\
		c->x = c->x_old = I2FP(sx);\
		c->y = c->y_old = I2FP(sy);\
		c->state = ENEMY_WALKING;\
		++levelCoins.coinsLeft;\
	}\
} while(0)

typedef struct {
	bool dir, grounded, crabIsMad, freezieFreezeNextPlatform, bonus : 1; // crabIsMad is only for crab type enemies, freezieFreezeNextPlatform is whether or not to freeze the platform
	uint8_t type, state, sprite, lastGroundedPlatformIndex;
	int x, x_old, y, y_old, width, height;
	unsigned int spawnTime, layStartTime, groundedStartTime, eventTime, lastBumpedEnemyTime;
	int horVel, verVel;
	int8_t verSpriteOffset, verSpriteOffset_old, horSpriteOffset, horSpriteOffset_old;
	void* lastBumpedEnemy; // stores the pointer of the last bumped enemy
	int maxSpeed;
	uint8_t backgroundData[ENEMY_SPIKE_SIZE*ENEMY_SPIKE_SIZE + 2];
} enemy_t;

typedef struct {
	uint8_t numEnemies, enemiesLeft, lastSpawnedPipe;
	unsigned int lastSpawnedTime;
	enemy_t* enemyArray;
} levelEnemies_t;

enum ENEMY_TYPE_IDS {
	ENEMY_SPIKE = 0,
	ENEMY_CRAB,
	ENEMY_FLY,
	ENEMY_FREEZIE,
	ENEMY_COIN,
};

enum ENEMY_STATE_IDS {
	ENEMY_WALKING = 0,
	ENEMY_TURNING,
	ENEMY_LAYING,
	ENEMY_DEAD_SPINNING,
	ENEMY_DEAD,
	ENEMY_EXITING_PIPE,
	ENEMY_ENTERING_PIPE,
	FREEZIE_FREEZING_PLATFORM
};

extern levelEnemies_t levelEnemies;

void InitEnemies(void);
enemy_t *SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame);
void FreeEnemies(void);
void UpdateEnemies(unsigned int gameFrame);
void ResetEnemies(unsigned int gameFrame);
void EnemyShowScore(enemy_t* enemy, player_t* player, unsigned int gameFrame);
void KillEnemy(enemy_t* enemy, player_t* player, unsigned int gameFrame);
void CollectCoin(enemy_t *coin, unsigned int gameFrame);