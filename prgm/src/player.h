#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PLAYER_HEIGHT	19
#define PLAYER_WIDTH	16
#define PLAYER_SPRITE_HEIGHT	23

#define RIGHT	0
#define LEFT	1
#define UP		2
#define DOWN	3
#define NONE	4
#define NOJUMP	5

#define GRAVITY 0.2

typedef struct {
	float x, x_old;
	int16_t y, y_old;
	float verAccel, horAccel, verAccelPassive, maxSpeed, acceleration, deceleration;
	uint8_t sprite, state, lives;
	bool grounded, dir, hasJumpedThisFrame, hasCollectedBonus : 1;
	int8_t verSpriteOffset, verSpriteOffset_old, lastGroundedPlatformIndex;
	unsigned int deathTime;
	unsigned int score;
	uint8_t backgroundData[PLAYER_SPRITE_HEIGHT*PLAYER_WIDTH + 2];
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
void PlayerMove(player_t* player, uint8_t direction);
void KillPlayer(player_t* player, unsigned int gameFrame);
void PlayerAddScore(player_t* player, uint16_t addedNum);