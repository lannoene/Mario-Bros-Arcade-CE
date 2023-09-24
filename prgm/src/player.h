#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PLAYER_HEIGHT	19
#define PLAYER_WIDTH	16

#define RIGHT	0
#define LEFT	1
#define UP		2
#define DOWN	3

#define GRAVITY 0.2

typedef struct {
	int16_t x, x_old;
	int16_t y, y_old;
	float verAccel, horAccel;
	uint8_t sprite;
	bool grounded, moving : 1; // moving: whether or not the player has pressed any directional keys this frame
	bool dir : 1;
	uint8_t backgroundData[PLAYER_HEIGHT*PLAYER_WIDTH + 2];
} player_t;

extern uint8_t mario_walking_sprite_table[4];

void PlayerInit(player_t* player);
void UpdatePlayer(player_t* player, int gameFrame);
void PlayerMove(player_t* player, uint8_t direction);