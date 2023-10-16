#include "bonus.h"

#include <stdlib.h>
#include <graphx.h>
#include <string.h>

#include "platforms.h"
#include "level.h"
#include "pipes.h"
#include "enemies.h"

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
	levelCoins.coinArray[i].bonus = bonus;
	if (!bonus) {
		levelCoins.coinArray[i].y = levelCoins.coinArray[i].y_old = 35;
		if (levelEnemies.lastSpawnedPipe == LEFT) {
			levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = 30;
			levelCoins.coinArray[i].dir = RIGHT;
			levelEnemies.lastSpawnedPipe = RIGHT;
		} else {
			levelCoins.coinArray[i].dir = LEFT;
			levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = 274;
			levelEnemies.lastSpawnedPipe = LEFT;
			
		}
		if (dir == LEFT) {
			levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = 30;
			levelCoins.coinArray[i].dir = RIGHT;
		} else {
			levelCoins.coinArray[i].dir = LEFT;
			levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = 274;
		}
	} else {
		levelCoins.coinArray[i].x = levelCoins.coinArray[i].x_old = x;
		levelCoins.coinArray[i].y = levelCoins.coinArray[i].y_old = y;
	}
	levelCoins.coinArray[i].spawnFrame = gameFrame;
	levelCoins.coinArray[i].state = COIN_EXITING_PIPE;
	levelCoins.coinArray[i].backgroundData[0] = COIN_WIDTH;
	levelCoins.coinArray[i].backgroundData[1] = COIN_HEIGHT;
}

void ResetCoins(void) {
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, levelCoins.coinArray[i].x, levelCoins.coinArray[i].y);
		gfx_SetDrawScreen();
		gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, levelCoins.coinArray[i].x, levelCoins.coinArray[i].y);
		gfx_SetDrawBuffer();
	}
	levelCoins.numCoins = 0;
	levelCoins.coinsLeft = 0;
}

void FreeBonusCoins(void) {
	free(levelCoins.coinArray);
}

void UpdateBonusCoins(player_t* player, unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		if (!levelCoins.coinArray[i].alive)
			continue;
		
		if (levelCoins.coinArray[i].alive && player->y - player->verAccel + PLAYER_HEIGHT > levelCoins.coinArray[i].y && player->y - player->verAccel < levelCoins.coinArray[i].y + COIN_HEIGHT && player->x + PLAYER_WIDTH + player->horAccel > levelCoins.coinArray[i].x && player->x + player->horAccel < levelCoins.coinArray[i].x + COIN_WIDTH) {
			levelCoins.coinArray[i].alive = false;
			--levelCoins.coinsLeft;
			if (!game_data.isBonusLevel)
				PlayerAddScore(player, 800);
		}
		
		levelCoins.coinArray[i].sprite = ((gameFrame - levelCoins.coinArray[i].spawnFrame)/4) % 5;
		if (levelCoins.coinArray[i].bonus)
			continue;
		
		levelCoins.coinArray[i].verAccel -= GRAVITY;
		
		switch (levelCoins.coinArray[i].state) {
			case COIN_NORMAL:
				
				break;
			case COIN_EXITING_PIPE:
				levelCoins.coinArray[i].verAccel = 0;
				RedrawPipesWithNewSprite((levelCoins.coinArray[i].dir + 1) % 2, 0, gameFrame); // pipe is opposite dir
				
				if (gameFrame - levelCoins.coinArray[i].spawnFrame > 65)
					levelCoins.coinArray[i].state = COIN_NORMAL;
				break;
		}
		
		if (levelCoins.coinArray[i].x < -COIN_WIDTH)
			levelCoins.coinArray[i].x = 320;
		else if (levelCoins.coinArray[i].x > 320)
			levelCoins.coinArray[i].x = -COIN_WIDTH;
		
		if (levelCoins.coinArray[i].dir == RIGHT) {
			levelCoins.coinArray[i].horAccel = 0.5;
		} else {
			levelCoins.coinArray[i].horAccel = -0.5;
		}
		
		// this is taken from the enemy platform colision code
		if (levelCoins.coinArray[i].grounded && levelCoins.coinArray[i].verAccel <= 0 && levelCoins.coinArray[i].x + levelCoins.coinArray[i].horAccel + COIN_WIDTH > levelPlatforms.platformArray[levelCoins.coinArray[i].lastGroundedPlatformIndex].x && levelCoins.coinArray[i].x + levelCoins.coinArray[i].horAccel < levelPlatforms.platformArray[levelCoins.coinArray[i].lastGroundedPlatformIndex].x + levelPlatforms.platformArray[levelCoins.coinArray[i].lastGroundedPlatformIndex].width) { // is enemy still inside of last intersected platform's x footprint? if so, make sure they stay like so
			levelCoins.coinArray[i].verAccel = 0;
		} else if (levelCoins.coinArray[i].y - levelCoins.coinArray[i].verAccel > 224 - COIN_HEIGHT) { // is enemy on the bottom floor? if so, we can skip colision checking and make sure verAccel is 0
			levelCoins.coinArray[i].y = 224 - COIN_HEIGHT;
			levelCoins.coinArray[i].verAccel = 0;
			if (!levelCoins.coinArray[i].grounded)
			levelCoins.coinArray[i].grounded = true;
			if ((levelCoins.coinArray[i].x + levelCoins.coinArray[i].horAccel >= 320 || levelCoins.coinArray[i].x + levelCoins.coinArray[i].horAccel <= -COIN_WIDTH)) {
				levelCoins.coinArray[i].y = 35;
				levelCoins.coinArray[i].state = COIN_EXITING_PIPE;
				levelCoins.coinArray[i].spawnFrame = gameFrame;
				if (levelCoins.coinArray[i].dir == RIGHT) {
					levelCoins.coinArray[i].x = 274;
					levelCoins.coinArray[i].dir = LEFT;
				} else {
					levelCoins.coinArray[i].x = 30;
					levelCoins.coinArray[i].dir = RIGHT;
				}
			}
		} else { // otherwise, do expensive colision checking
			if (levelCoins.coinArray[i].verAccel < 0) { // only test for physics if the verAccel is less than 0
				uint8_t j = 0;
				for (; j < levelPlatforms.numPlatforms; j++) {
					if (levelCoins.coinArray[i].y - levelCoins.coinArray[i].verAccel + COIN_HEIGHT > levelPlatforms.platformArray[j].y && levelCoins.coinArray[i].y - levelCoins.coinArray[i].verAccel < levelPlatforms.platformArray[j].y + PLATFORM_HEIGHT && levelCoins.coinArray[i].x + levelCoins.coinArray[i].horAccel + COIN_WIDTH > levelPlatforms.platformArray[j].x && levelCoins.coinArray[i].x + levelCoins.coinArray[i].horAccel < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
						levelCoins.coinArray[i].y = levelPlatforms.platformArray[j].y - COIN_HEIGHT;
						levelCoins.coinArray[i].verAccel = 0;
						levelCoins.coinArray[i].grounded = true;
						levelCoins.coinArray[i].lastGroundedPlatformIndex = j;
						break;
					}
				}
				if (j == levelPlatforms.numPlatforms && levelCoins.coinArray[i].grounded == true)
					levelCoins.coinArray[i].grounded = false;
			}
		}
		
		levelCoins.coinArray[i].y -= levelCoins.coinArray[i].verAccel;
		levelCoins.coinArray[i].x += levelCoins.coinArray[i].horAccel;
	}
}