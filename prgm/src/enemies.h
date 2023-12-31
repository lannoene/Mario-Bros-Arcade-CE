#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define ENEMY_SPIKE_SIZE 16
#define ENEMY_SPIKE_HITBOX_HEIGHT 12
#define ENEMY_FREEZIE_WIDTH 12

typedef struct {
	bool dir, grounded, crabIsMad, freezieFreezeNextPlatform : 1; // crabIsMad is only for crab type enemies, freezieFreezeNextPlatform is whether or not to freeze the platform
	uint8_t type, state, sprite, lastGroundedPlatformIndex;
	float x, x_old, y, y_old;
	unsigned int spawnTime, layStartTime, groundedStartTime, eventTime;
	float horAccel, verAccel;
	int8_t verSpriteOffset, verSpriteOffset_old, horSpriteOffset, horSpriteOffset_old, lastBumpedEnemy;
	float maxSpeed;
	uint8_t backgroundData[ENEMY_SPIKE_SIZE*ENEMY_SPIKE_SIZE + 2];
} enemy_t;

typedef struct {
	uint8_t numEnemies, enemiesLeft, lastSpawnedPipe, currentCombo;
	unsigned int lastSpawnedTime, lastKilledTime;
	enemy_t* enemyArray;
} levelEnemies_t;

enum ENEMY_TYPE_IDS {
	ENEMY_SPIKE = 0,
	ENEMY_CRAB,
	ENEMY_FLY,
	ENEMY_FREEZIE,
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
void SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame);
void FreeEnemies(void);
void UpdateEnemies(player_t* player, unsigned int gameFrame);
void ResetEnemies(unsigned int gameFrame);
void EnemyShowScore(uint8_t enemyIndex, player_t* player, unsigned int gameFrame);