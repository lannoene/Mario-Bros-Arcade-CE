#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platforms.h"

#define FRAME_DELAY 5

void PlayerInit(player_t* player) {
	memset(player, 0, sizeof(*player));
	player->backgroundData[0] = PLAYER_WIDTH;
	player->backgroundData[1] = PLAYER_HEIGHT;
	player->y = 224 - PLAYER_HEIGHT;
	player->x = 16;
}

void PlayerMove(player_t* player, uint8_t direction) {
	if (!player->grounded)
		return;
	switch (direction) {
		case LEFT:
			player->horAccel = -2;
			player->dir = LEFT;
			player->moving = true;
			break;
		case RIGHT:
			player->horAccel = 2;
			player->dir = RIGHT;
			player->moving = true;
			break;
		case UP:
			if (player->grounded) {
				player->verAccel = 4.5;
				player->grounded = false;
				player->sprite = 4;
			}
			break;
	}
}

void UpdatePlayer(player_t* player, int gameFrame) {
	player->verAccel -= GRAVITY;
	if (player->horAccel != 0 && !player->moving && player->grounded)
		player->horAccel = 0;
	
	colision_t colidedPlatform = CheckColision(&player->x, &player->y, PLAYER_WIDTH, PLAYER_HEIGHT, &player->verAccel, &player->horAccel);
	if (colidedPlatform.hasColided && colidedPlatform.colidedSide == UP) {
		player->y = colidedPlatform.y - PLAYER_HEIGHT;
		player->verAccel = 0;
		player->grounded = true;
	} else if (colidedPlatform.hasColided && colidedPlatform.colidedSide == DOWN) {
		player->y = colidedPlatform.y + PLATFORM_HEIGHT;
		player->verAccel = 0;
		BumpPlatform(player->x, colidedPlatform.colidedIndex, gameFrame);
	} else {
		player->grounded = false;
	}
	
	if (player->horAccel != 0 && player->grounded) {
		if (gameFrame % FRAME_DELAY == 0) {
			switch (gameFrame/FRAME_DELAY % 3) {
				case 0:
					player->sprite = 2;
					break;
				case 1:
					player->sprite = 1;
					break;
				case 2:
					player->sprite = 0;
					break;
			}
		}
	} else if (player->horAccel == 0 && player->grounded) {
		player->sprite = 0;
	}
	
	player->y -= player->verAccel;
	player->x += player->horAccel;
}