#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PLAYER_HEIGHT	24
#define PLAYER_WIDTH	16

#define RIGHT	0
#define LEFT	1
#define UP		2
#define DOWN	3

typedef struct {
	int16_t x, x_old;
	uint8_t y, y_old;
	float verAccel, horAccel;
	bool grounded, moving; // moving: whether or not the player has pressed any directional keys this frame
	uint8_t sprite;
	bool dir;
	uint8_t backgroundData[PLAYER_HEIGHT*PLAYER_WIDTH + 2];
} player_t;

extern uint8_t mario_walking_sprite_table[4];

void PlayerInit(player_t* player);
void DrawPlayer(player_t* player, int* gameFrame);
void UpdatePlayer(player_t* player, int gameFrame);
void PlayerMove(player_t* player, uint8_t direction);