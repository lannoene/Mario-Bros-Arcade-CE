#include "icicles.h"

#include <stdlib.h>
#include <graphx.h>

#include "platforms.h"
#include "player.h"

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
		if (levelIcicles.icicleArray[i].state == ICICLE_DEAD)
			continue;
		
		switch (levelIcicles.icicleArray[i].state) {
			case ICICLE_FORMING:
				if (gameFrame - levelIcicles.icicleArray[i].spawnTime >= 158) {
					levelIcicles.icicleArray[i].state = ICICLE_FALLING;
				} else if (gameFrame - levelIcicles.icicleArray[i].spawnTime >= 148) {
					levelIcicles.icicleArray[i].sprite = ((gameFrame - levelIcicles.icicleArray[i].spawnTime)/4 % 2) + 3;
				} else if (gameFrame - levelIcicles.icicleArray[i].spawnTime >= 131) {
					levelIcicles.icicleArray[i].sprite = 5;
				} else if (gameFrame - levelIcicles.icicleArray[i].spawnTime >= 127) {
					levelIcicles.icicleArray[i].sprite = 2;
				} else if (gameFrame - levelIcicles.icicleArray[i].spawnTime >= 64) {
					levelIcicles.icicleArray[i].sprite = 1;
				}
				
				if (gameFrame - levelIcicles.icicleArray[i].spawnTime < 131 && (player->horAccel != 0 || player->verAccel != 0)) {
					if (player->x + PLAYER_WIDTH > levelIcicles.icicleArray[i].x && player->x < levelIcicles.icicleArray[i].x + ICICLE_WIDTH && player->y + PLAYER_HEIGHT > levelIcicles.icicleArray[i].y && player->y < levelIcicles.icicleArray[i].y + ((gameFrame - levelIcicles.icicleArray[i].spawnTime)*0.142) /* have the length dynamically increase (also known as linear interpolation) */ && player->state == PLAYER_NORMAL) { // make sure icicle is properly disposed of offscreen. if the icicle were to instantly be stopped drawing, a ghost of it would be kept on screen.
						levelIcicles.icicleArray[i].y = 241;
						levelIcicles.icicleArray[i].state = ICICLE_FALLING;
					}
				} else {
					if (player->x + PLAYER_WIDTH > levelIcicles.icicleArray[i].x && player->x < levelIcicles.icicleArray[i].x + ICICLE_WIDTH && player->y + PLAYER_HEIGHT > levelIcicles.icicleArray[i].y + levelIcicles.icicleArray[i].verAccel && player->y < levelIcicles.icicleArray[i].y + ICICLE_HEIGHT + levelIcicles.icicleArray[i].verAccel && player->state == PLAYER_NORMAL) {
						KillPlayer(player, gameFrame);
					}
				}
				
				break;
			case ICICLE_FALLING:
				levelIcicles.icicleArray[i].sprite = ((gameFrame - levelIcicles.icicleArray[i].spawnTime)/8 % 2) + 3;
				
				levelIcicles.icicleArray[i].verAccel -= GRAVITY;
				
				if (player->x + PLAYER_WIDTH > levelIcicles.icicleArray[i].x && player->x < levelIcicles.icicleArray[i].x + ICICLE_WIDTH && player->y + PLAYER_HEIGHT > levelIcicles.icicleArray[i].y + levelIcicles.icicleArray[i].verAccel && player->y < levelIcicles.icicleArray[i].y + ICICLE_HEIGHT + levelIcicles.icicleArray[i].verAccel && player->state == PLAYER_NORMAL) {
					KillPlayer(player, gameFrame);
				}
				
				if (levelIcicles.icicleArray[i].y > 240)
					levelIcicles.icicleArray[i].state = ICICLE_DEAD;
				break;
		}
		levelIcicles.icicleArray[i].y -= levelIcicles.icicleArray[i].verAccel;
	}
}

void FreeIcicles(void) {
	free(levelIcicles.icicleArray);
}