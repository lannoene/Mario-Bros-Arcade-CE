#include "icicles.h"

#include <stdlib.h>
#include <graphx.h>

#include "platforms.h"
#include "player.h"
#include "defines.h"

levelIcicleData_t levelIcicles;

void InitIcicles(void) {
	levelIcicles.icicleArray = malloc(0);
	levelIcicles.numIcicles = 0;
}

void SpawnIcicle(unsigned int gameFrame) {
	uint8_t i;
	for (i = 0; i < levelIcicles.numIcicles; i++) {
		if (levelIcicles.icicleArray[i].state == ICICLE_DEAD)
			break;
	}
	if (i == levelIcicles.numIcicles) {
		++levelIcicles.numIcicles;
		levelIcicles.icicleArray = realloc(levelIcicles.icicleArray, levelIcicles.numIcicles*sizeof(icicle_t));
	}
	levelIcicles.icicleArray[i].spawnTime = gameFrame;
	levelIcicles.icicleArray[i].backgroundData[0] = ICICLE_WIDTH;
	levelIcicles.icicleArray[i].backgroundData[1] = ICICLE_HEIGHT;
	levelIcicles.icicleArray[i].state = ICICLE_FORMING;
	levelIcicles.icicleArray[i].sprite = 0;
	levelIcicles.icicleArray[i].verAccel = 0;
	
	if (rand() % 2 == 0) { // left or right
		if (rand() % 6 == 0) { // on pipe or not
			levelIcicles.icicleArray[i].y = 47; // on the pipe left
			levelIcicles.icicleArray[i].x = 40;
		} else {
			levelIcicles.icicleArray[i].y = 72 + PLATFORM_HEIGHT; // on the ground left
			levelIcicles.icicleArray[i].x = 5 + (rand() % 130);
		}
	} else {
		if (rand() % 6 == 0) {
			levelIcicles.icicleArray[i].y = 47; // on the pipe right
			levelIcicles.icicleArray[i].x = 265;
		} else { // not on pipe
			levelIcicles.icicleArray[i].y = 72 + PLATFORM_HEIGHT; // on the ground right
			levelIcicles.icicleArray[i].x = 176 + (rand() % 130);
		}
	}
	levelIcicles.icicleArray[i].y_old = levelIcicles.icicleArray[i].y;
	levelIcicles.icicleArray[i].x_old = levelIcicles.icicleArray[i].x;
}

void UpdateIcicles(player_t* player, unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelIcicles.numIcicles; i++) {
		icicle_t* icicle = &levelIcicles.icicleArray[i];
		if (icicle->state == ICICLE_DEAD)
			continue;
		
		switch (icicle->state) {
			case ICICLE_FORMING:
				if (gameFrame - icicle->spawnTime >= 158) {
					icicle->state = ICICLE_FALLING;
				} else if (gameFrame - icicle->spawnTime >= 148) {
					icicle->sprite = ((gameFrame - icicle->spawnTime)/4 % 2) + 3;
				} else if (gameFrame - icicle->spawnTime >= 131) {
					icicle->sprite = 5;
				} else if (gameFrame - icicle->spawnTime >= 127) {
					icicle->sprite = 2;
				} else if (gameFrame - icicle->spawnTime >= 64) {
					icicle->sprite = 1;
				}
				
				if (gameFrame - icicle->spawnTime < 131 && (player->horAccel != 0 || player->verAccel != 0)) {
					// while the icicle is still forming, people can interact with it to destroy the icicle
					if (FIXED_POINT_TO_INT(player->x) + PLAYER_WIDTH > icicle->x && 
					FIXED_POINT_TO_INT(player->x) < icicle->x + ICICLE_WIDTH && 
					FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT > icicle->y && 
					FIXED_POINT_TO_INT(player->y) < icicle->y + ((gameFrame - icicle->spawnTime)*0.142) /* have the length dynamically increase (also known as linear interpolation) */ && 
					player->state == PLAYER_NORMAL) {
						/*
						 * make sure icicle is properly disposed of offscreen.
						 * if the icicle were to instantly be stopped drawing,
						 * a ghost of it would be kept on screen because of the double buffering
						 */
						icicle->y = 241;
						// setting state to icicle falling lets the other buffer update
						icicle->state = ICICLE_FALLING; 
					}
				} else {
					// when the icicle is done forming, people cannot touch it, or they will die.
					if (FIXED_POINT_TO_INT(player->x) + PLAYER_WIDTH > icicle->x && 
					FIXED_POINT_TO_INT(player->x) < icicle->x + ICICLE_WIDTH && 
					FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT > icicle->y + icicle->verAccel && 
					FIXED_POINT_TO_INT(player->y) < icicle->y + ICICLE_HEIGHT + icicle->verAccel && 
					player->state == PLAYER_NORMAL) {
						KillPlayer(player, gameFrame);
					}
				}
				
				break;
			case ICICLE_FALLING:
				// now let icicle fall
				icicle->sprite = ((gameFrame - icicle->spawnTime)/8 % 2) + 3;
				
				/*
				 * i don't want verAccel to be a float, and the GRAVITY constant is 0.2.
				 * 0.2 as a fraction is 1/5. take 1 away every 5 ticks/frames.
				 * and yes, i know it should be ((gameFrame - spawntime) % 5 == 0),
				 * but i don't want to do that extra calculation when it probably wont make a difference
				 */
				icicle->verAccel -= (gameFrame % 5 == 0) ? 1 : 0; 
				
				if (FIXED_POINT_TO_INT(player->x) + PLAYER_WIDTH > icicle->x && 
				FIXED_POINT_TO_INT(player->x) < icicle->x + ICICLE_WIDTH && 
				FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT > icicle->y + icicle->verAccel && 
				FIXED_POINT_TO_INT(player->y) < icicle->y + ICICLE_HEIGHT + icicle->verAccel && 
				player->state == PLAYER_NORMAL) {
					KillPlayer(player, gameFrame);
				}
				
				if (icicle->y > 240)
					icicle->state = ICICLE_DEAD; // mark icicle free
				break;
		}
		icicle->y -= icicle->verAccel;
	}
}

void FreeIcicles(void) {
	free(levelIcicles.icicleArray);
}

void ResetIcicles(void) {
	// replace all live icicles with their bg
	for (icicle_t* icicle = levelIcicles.icicleArray; icicle != &levelIcicles.icicleArray[levelIcicles.numIcicles]; icicle++) {
		if (icicle->state != ICICLE_DEAD) {
			gfx_Sprite((gfx_sprite_t*)icicle->backgroundData, icicle->x, icicle->y);
			gfx_SetDrawScreen();
			gfx_Sprite((gfx_sprite_t*)icicle->backgroundData, icicle->x, icicle->y);
			gfx_SetDrawBuffer();
		}
	}
	levelIcicles.numIcicles = 0;
}