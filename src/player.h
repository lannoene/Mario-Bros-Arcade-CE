#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "collision.h"

#define PLAYER_HEIGHT	19
#define PLAYER_WIDTH	16
#define PLAYER_SPRITE_HEIGHT	23
#define PLAYER_RESP_WAIT_MAX 350
#define PLAYER_RESP_FALL_DURATION 75

#define GRAVITY 0.2

typedef struct platform platform_t;

typedef struct player {
	position_t pos;
	int x, x_old, y, y_old;
	int verAccel, horAccel, horAccelPassive;
	int verAccelPassive, maxSpeed, acceleration, deceleration, lastKilledEnemyTime, lastKilledTime, spawnTime;
	uint8_t sprite, state, lives, currentCombo;
	bool grounded, dir, hasJumpedThisFrame, hasCollectedBonus, canExtendJump, doingFastJump : 1;
	int8_t verSpriteOffset, verSpriteOffset_old;
	platform_t *lastGroundedPlatform;
	unsigned int deathTime, lastJumpTime;
	unsigned int score;
	uint8_t backgroundData[PLAYER_SPRITE_HEIGHT*PLAYER_WIDTH + 2];
	uint8_t respawnPlatformBgData[15*7 + 2];
} player_t;

enum PLAYER_STATES {
	PLAYER_SPAWNING,
	PLAYER_INVINCIBILITY,
	PLAYER_NORMAL,
	PLAYER_DEAD,
	PLAYER_RESPAWNING,
};

extern uint8_t mario_walking_sprite_table[4];

void PlayerInit(player_t* player);
void UpdatePlayer(player_t* player, int gameFrame);
void PlayerMove(player_t* player, uint8_t direction, unsigned int gameFrame);
void KillPlayer(player_t* player, unsigned int gameFrame);
void PlayerAddScore(player_t* player, uint16_t addedNum);
void UpdatePlayers(player_t *players, unsigned int gameFrame);