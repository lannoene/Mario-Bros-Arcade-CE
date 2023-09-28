#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>

#include "platforms.h"

#define FRAME_DELAY 5
#define PLAYER_ACCELERATION	0.5
#define PLAYER_MAX_SPEED	2
#define PLAYER_DECELERATION	0.2

void PlayerInit(player_t* player) {
	memset(player, 0, sizeof(*player));
	player->backgroundData[0] = PLAYER_WIDTH;
	player->backgroundData[1] = PLAYER_SPRITE_HEIGHT;
	player->y = 224 - PLAYER_HEIGHT;
	player->x = 16;
	player->lives = 4;
	player->state = PLAYER_NORMAL;
	player->verSpriteOffset = 0;
}

void PlayerMove(player_t* player, uint8_t direction) {
	switch (direction) {
		case LEFT:
			if (player->horAccel > -PLAYER_MAX_SPEED)
				player->horAccel -= PLAYER_ACCELERATION;
			else
				player->horAccel = -PLAYER_MAX_SPEED;
			player->dir = LEFT;
			player->moving = true;
			break;
		case RIGHT:
			if (player->horAccel < PLAYER_MAX_SPEED)
				player->horAccel += PLAYER_ACCELERATION;
			else
				player->horAccel = PLAYER_MAX_SPEED;
			player->dir = RIGHT;
			player->moving = true;
			break;
		case UP:
			if (!player->grounded)
				return;
			if (player->grounded) {
				player->verAccel = 4.5;
				player->grounded = false;
				player->sprite = 4;
			}
			break;
		default:
			if (player->horAccel < PLAYER_DECELERATION && player->horAccel > -PLAYER_DECELERATION)
				player->horAccel = 0;
			
			if (player->horAccel > 0 && player->grounded)
				player->horAccel -= PLAYER_DECELERATION;
			else if (player->horAccel < 0 && player->grounded)
				player->horAccel += PLAYER_DECELERATION;
			break;
	}
}

void UpdatePlayer(player_t* player, int gameFrame) {
	
	switch (player->state) {
		case PLAYER_NORMAL:
			player->verAccel -= GRAVITY;
			break;
		case PLAYER_DEAD:
			if (gameFrame - player->deathTime == 10)
				player->verAccel = 3;
			else if (gameFrame - player->deathTime > 10)
				player->verAccel -= GRAVITY;
			player->horAccel = 0;
			if (player->y > 240 && gameFrame - player->deathTime > 100) {
				if (player->lives > 0) {
					player->state = PLAYER_NORMAL;
					player->y = 224 - PLAYER_HEIGHT;
					player->x = 16;
				} else {
					gfx_PrintStringXY("Game Over", 100, 100);
					player->verAccel = 0;
				}
			}
			break;
	}
	
	if (player->state == PLAYER_NORMAL) {
		
		// checking x for offscreen transition to other side
		if (player->x < -PLAYER_WIDTH)
			player->x = 320; // if they go offscreen to the left, teleport them to the right side
		else if (player->x > 320)
			player->x = -PLAYER_WIDTH; // if they go offscreen to the right, teleport them to the left side
		
		if (player->grounded && player->x + PLAYER_WIDTH + player->horAccel > levelPlatforms.platformArray[player->lastGroundedPlatformIndex].x && player->x + player->horAccel < levelPlatforms.platformArray[player->lastGroundedPlatformIndex].x + levelPlatforms.platformArray[player->lastGroundedPlatformIndex].width) {
			player->verAccel = 0;
			player->grounded = true;
		} else if (player->y - player->verAccel > 224 - PLAYER_HEIGHT) {
			player->y = 224 - PLAYER_HEIGHT;
			player->verAccel = 0;
			player->grounded = true;
		} else {
			uint8_t i;
			for (i = 0; i < levelPlatforms.numPlatforms; i++) {
				if (player->y - player->verAccel + PLAYER_HEIGHT > levelPlatforms.platformArray[i].y && player->y - player->verAccel < levelPlatforms.platformArray[i].y + PLATFORM_HEIGHT && player->x + PLAYER_WIDTH + player->horAccel > levelPlatforms.platformArray[i].x && player->x + player->horAccel < levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width) {
					if (player->verAccel < 0 && player->y + PLAYER_HEIGHT <= levelPlatforms.platformArray[i].y) {
						player->y = levelPlatforms.platformArray[i].y - PLAYER_HEIGHT;
						player->verAccel = 0;
						player->lastGroundedPlatformIndex = i;
						player->grounded = true;
						break;
					} else if (player->verAccel > 0 && player->y >= levelPlatforms.platformArray[i].y + PLATFORM_HEIGHT) {
						player->y = levelPlatforms.platformArray[i].y + PLATFORM_HEIGHT;
						player->verAccel = 0;
						BumpPlatform(player->x, i, gameFrame);
						break;
					}
				}
			}
			if (player->grounded && i == levelPlatforms.numPlatforms) {
				player->grounded = false;
				player->sprite = 4;
			}
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
	}
	
	player->y -= player->verAccel;
	player->x += player->horAccel;
}

void KillPlayer(player_t* player, unsigned int gameFrame) {
	player->state = PLAYER_DEAD;
	player->verAccel = 0;
	player->dir = RIGHT;
	player->sprite = 5;
	player->deathTime = gameFrame;
	player->y -= PLAYER_SPRITE_HEIGHT - PLAYER_HEIGHT;
	--player->lives;
}