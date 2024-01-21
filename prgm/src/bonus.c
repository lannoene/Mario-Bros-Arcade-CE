#include "bonus.h"

#include <stdlib.h>
#include <graphx.h>
#include <string.h>

#include "defines.h"
#include "platforms.h"
#include "level.h"
#include "pipes.h"
#include "enemies.h"
#include "particles.h"

bonusLevel_t levelCoins;

// honestly, the only reason this is in bonus is cuz i originally made coins for bonus levels. now that i'm having to add coins to classic, they really should go in their own file. whatever, i don't feel like breaking anything.

void InitBonusData(void) {
	levelCoins.coinArray = malloc(0);
	levelCoins.bonusTimer = 1200;
	levelCoins.numCoins = levelCoins.coinsLeft = 0;
}

void SpawnBonusCoin(int16_t x, uint8_t y, bool bonus, bool dir, unsigned int gameFrame) {
	uint8_t i;
	for (i = 0; i < levelCoins.numCoins; i++) {
		if (!levelCoins.coinArray[i].alive)
			break;
	}
	if (i == levelCoins.numCoins) {
		++levelCoins.numCoins;
		levelCoins.coinArray = realloc(levelCoins.coinArray, levelCoins.numCoins*sizeof(bonusCoin_t));
	}
	memset(levelCoins.coinArray + i, 0, sizeof(bonusCoin_t));
	++levelCoins.coinsLeft;
	levelCoins.coinArray[i].alive = true;
	levelCoins.coinArray[i].shouldDie = false;
	levelCoins.coinArray[i].bonus = bonus;
	if (!bonus) {
		levelCoins.coinArray[i].y = levelCoins.coinArray[i].y_old = TO_FIXED_POINT(35);
		if (levelEnemies.lastSpawnedPipe == RIGHT) {
			levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = TO_FIXED_POINT(30);
			levelCoins.coinArray[i].dir = RIGHT;
			levelEnemies.lastSpawnedPipe = LEFT;
		} else {
			levelCoins.coinArray[i].dir = LEFT;
			levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = TO_FIXED_POINT(274);
			levelEnemies.lastSpawnedPipe = RIGHT;
		}
	} else {
		levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = TO_FIXED_POINT(x);
		levelCoins.coinArray[i].y = levelCoins.coinArray[i].y_old = TO_FIXED_POINT(y);
	}
	levelCoins.coinArray[i].spawnFrame = gameFrame;
	levelCoins.coinArray[i].state = COIN_EXITING_PIPE;
	levelCoins.coinArray[i].firstTimeSpawning = true;
	levelCoins.coinArray[i].grounded = false;
	levelCoins.coinArray[i].backgroundData[0] = COIN_WIDTH;
	levelCoins.coinArray[i].backgroundData[1] = COIN_HEIGHT;
}

void ResetCoins(void) {
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive) {
			gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, FIXED_POINT_TO_INT(levelCoins.coinArray[i].x), FIXED_POINT_TO_INT(levelCoins.coinArray[i].y));
			gfx_SetDrawScreen();
			gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, FIXED_POINT_TO_INT(levelCoins.coinArray[i].x), FIXED_POINT_TO_INT(levelCoins.coinArray[i].y));
			gfx_SetDrawBuffer();
		}
	}
	levelCoins.numCoins = 0;
	levelCoins.coinsLeft = 0;
}

void FreeBonusCoins(void) {
	free(levelCoins.coinArray);
}

void UpdateBonusCoins(player_t* player, unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		bonusCoin_t* coin = &levelCoins.coinArray[i];
		
		if (!coin->alive)
			continue;
		
		if (player->y - player->verAccel + TO_FIXED_POINT(PLAYER_HEIGHT) > coin->y 
		&& player->y - player->verAccel < coin->y + TO_FIXED_POINT(COIN_HEIGHT) 
		&& player->x + TO_FIXED_POINT(PLAYER_WIDTH) + player->horAccel > coin->x 
		&& player->x + player->horAccel < coin->x + COIN_WIDTH_FP) {
			coin->shouldDie = true;
			--levelCoins.coinsLeft;
			if (!game_data.isBonusLevel) {
				PlayerAddScore(player, 800);
			}
			SpawnParticle(FIXED_POINT_TO_INT(coin->x), FIXED_POINT_TO_INT(coin->y), PARTICLE_COIN_PICK, gameFrame);
			coin->y = TO_FIXED_POINT(241);
			coin->verAccel = 0;
			continue;
		}
		if (coin->shouldDie) {
			coin->alive = false;
		}
		
		coin->sprite = ((gameFrame - coin->spawnFrame)/4) % 5;
		if (coin->bonus)
			continue;
		
		coin->verAccel -= TO_FIXED_POINT(GRAVITY);
		
		switch (coin->state) {
			case COIN_NORMAL:
				
				break;
			case COIN_EXITING_PIPE:
				if (coin->firstTimeSpawning) { // delay coin exit. i didn't like how quickly they came out after the player killed enemies
					coin->verAccel = 0;
					RedrawPipesWithNewSprite((coin->dir + 1) % 2, 0, gameFrame); // pipe is opposite dir
					if (gameFrame - coin->spawnFrame < 70) {
						coin->horAccel = 0;
					} else {
						if (coin->dir == LEFT)
							coin->horAccel = TO_FIXED_POINT(-0.5);
						else
							coin->horAccel = TO_FIXED_POINT(0.5);
					}
					
					if (coin->dir == LEFT && gameFrame - coin->spawnFrame > 120) {
						coin->state = COIN_NORMAL;
						coin->firstTimeSpawning = false;
					} else if (coin->dir == RIGHT && gameFrame - coin->spawnFrame > 140) {
						coin->state = COIN_NORMAL;
						coin->firstTimeSpawning = false;
					}
				} else {
					coin->verAccel = 0;
					RedrawPipesWithNewSprite((coin->dir + 1) % 2, 0, gameFrame); // pipe is opposite dir
					
					if (gameFrame - coin->spawnFrame > 65)
						coin->state = COIN_NORMAL;
					if (coin->dir == LEFT)
							coin->horAccel = TO_FIXED_POINT(-0.5);
						else
							coin->horAccel = TO_FIXED_POINT(0.5);
				}
				break;
		}
		
		if (coin->x < -COIN_WIDTH_FP)
			coin->x = TO_FIXED_POINT(320);
		else if (coin->x > TO_FIXED_POINT(320))
			coin->x = -COIN_WIDTH_FP;
		
		if (coin->state != COIN_EXITING_PIPE) {
			if (coin->dir == RIGHT) {
				coin->horAccel = TO_FIXED_POINT(0.5);
			} else {
				coin->horAccel = TO_FIXED_POINT(-0.5);
			}
		}
		
		// this is taken from the enemy platform colision code
		if (coin->grounded 
		&& coin->x + coin->horAccel + COIN_WIDTH_FP > levelPlatforms.platformArray[coin->lastGroundedPlatformIndex].x 
		&& coin->x + coin->horAccel < levelPlatforms.platformArray[coin->lastGroundedPlatformIndex].x + levelPlatforms.platformArray[coin->lastGroundedPlatformIndex].width) {
			coin->verAccel = 0;
		} else if (coin->y - coin->verAccel > TO_FIXED_POINT(GROUND_HEIGHT - COIN_HEIGHT)) {
			coin->y = TO_FIXED_POINT(GROUND_HEIGHT - COIN_HEIGHT);
			coin->verAccel = 0;
			if (!coin->grounded)
			coin->grounded = true;
			if (coin->x + coin->horAccel >= TO_FIXED_POINT(320) || coin->x + coin->horAccel <= -COIN_WIDTH_FP) {
				coin->alive = false;
			}
		} else {
			if (coin->verAccel < 0) {
				uint8_t j = 0;
				for (; j < levelPlatforms.numPlatforms; j++) {
					if (coin->y - coin->verAccel + TO_FIXED_POINT(COIN_HEIGHT) > levelPlatforms.platformArray[j].y &&
					coin->y - coin->verAccel < levelPlatforms.platformArray[j].y + TO_FIXED_POINT(PLATFORM_HEIGHT) && 
					coin->x + coin->horAccel + COIN_WIDTH_FP > levelPlatforms.platformArray[j].x && 
					coin->x + coin->horAccel < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
						coin->y = levelPlatforms.platformArray[j].y - TO_FIXED_POINT(COIN_HEIGHT);
						coin->verAccel = 0;
						coin->grounded = true;
						coin->lastGroundedPlatformIndex = j;
						break;
					}
				}
				if (j == levelPlatforms.numPlatforms && coin->grounded == true)
					coin->grounded = false;
			}
		}
		
		coin->y -= coin->verAccel;
		coin->x += coin->horAccel;
	}
}