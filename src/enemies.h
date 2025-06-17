#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "player.h"
#include "platforms.h"

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

typedef struct enemy {
	bool dir, grounded, crabIsMad, freezieFreezeNextPlatform, bonus : 1; // crabIsMad is only for crab type enemies, freezieFreezeNextPlatform is whether or not to freeze the platform
	uint8_t type, state, lastState, sprite;
	platform_t *lastGroundedPlatform;
	int x, x_old, y, y_old, width, height;
	unsigned int spawnTime, layStartTime, groundedStartTime, eventTime, lastBumpedEnemyTime;
	int horVel, verVel;
	int8_t verSpriteOffset, verSpriteOffset_old, horSpriteOffset, horSpriteOffset_old;
	void* lastBumpedEnemy; // stores the pointer of the last bumped enemy
	int maxSpeed;
	uint8_t backgroundData[ENEMY_SPIKE_SIZE*ENEMY_SPIKE_SIZE + 2];
} enemy_t;

typedef struct {
	uint8_t numEnemies, numAllocated, enemiesLeft, lastSpawnedPipe;
	unsigned int lastSpawnedTime;
	enemy_t* enemyArray;
} levelEnemies_t;

// ENEMIES MUST BE SORTED FROM LEAST EXPENSIVE TO MOST EXPENSIVE
enum ENEMY_TYPE_IDS {
	ENEMY_SPIKE = 1,
	ENEMY_CRAB = 1 << 1,
	ENEMY_FLY = 1 << 2,
	ENEMY_FREEZIE = 1 << 3,
	ENEMY_COIN = 1 << 4,
};

typedef struct {
	uint8_t enemyId;
	uint8_t spawnCost;
	bool autospawn;
} enemy_info_t;

enum ENEMY_STATE_IDS {
	ENEMY_DEAD = 0,
	ENEMY_TURNING,
	ENEMY_DEAD_SPINNING,
	ENEMY_EXITING_PIPE,
	ENEMY_ENTERING_PIPE,
	FREEZIE_FREEZING_PLATFORM,
	ENEMY_WALKING, // start of collidable states
	ENEMY_LAYING,
};

extern levelEnemies_t levelEnemies;
extern enemy_info_t levelEnemyInfo[5];

void InitEnemies(void);
enemy_t *SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame);
void FreeEnemies(void);
void UpdateEnemies(unsigned int gameFrame);
void ResetEnemies(unsigned int gameFrame, bool fast);
void EnemyShowScore(enemy_t* enemy, player_t* player, unsigned int gameFrame);
void EnemyShowScoreIndividual(enemy_t* enemy, player_t* player, unsigned int amount, unsigned int gameFrame);
void KillEnemy(enemy_t* enemy, player_t* player, unsigned int gameFrame);
void CollectCoin(player_t *player, enemy_t *coin);