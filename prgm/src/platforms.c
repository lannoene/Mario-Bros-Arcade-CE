#include "platforms.h"

#include <stdlib.h>
#include <graphx.h>
#include <math.h>

#include "enemies.h"
#include "bonus.h"
#include "particles.h"
#include "defines.h"

#include "gfx/gfx.h"

levelPlatformData_t levelPlatforms = {};

gfx_rletsprite_t* platformBlocks[4] = {pipes_block, lava_block, castle_block, snowy_normal_block};

void InitPlatformData(void) {
	levelPlatforms.platformArray = malloc(0);
	levelPlatforms.numPlatforms = 0;
}

void CreatePlatform(int16_t x, uint8_t y, uint8_t width) {
	++levelPlatforms.numPlatforms;
	levelPlatforms.platformArray = realloc(levelPlatforms.platformArray, levelPlatforms.numPlatforms*sizeof(platform_t));
	platform_t* platform = &levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1];
	platform->x = TO_FIXED_POINT(x);
	platform->x_old = TO_FIXED_POINT(x);
	platform->y = TO_FIXED_POINT(y);
	platform->y_old = TO_FIXED_POINT(y);
	platform->width = TO_FIXED_POINT(width);
	platform->backgroundData = malloc(width*(PLATFORM_HEIGHT*2) + 2);
	platform->backgroundData[0] = width;
	platform->backgroundData[1] = PLATFORM_HEIGHT*2;
	platform->beingBumped = false;
	platform->needsRefresh = false;
	platform->icy = false;
	platform->invisible = false;
	// preprocess tile image
	platform->processedTileImage = malloc(width*PLATFORM_HEIGHT*2 + 2);
	platform->processedTileImage[0] = width;
	platform->processedTileImage[1] = PLATFORM_HEIGHT*2;
	gfx_GetSprite((gfx_sprite_t*)platform->backgroundData, x, y - PLATFORM_HEIGHT); // get bg
	/*for (uint8_t i = 0; i < width/BLOCK_SIZE; i++)
		gfx_RLETSprite(pipes_block, x + i*BLOCK_SIZE, y); // process image
	gfx_GetSprite((gfx_sprite_t*)platform->processedTileImage, x, y);*/
	//gfx_Sprite((gfx_sprite_t*)platform->backgroundData, x, y - PLATFORM_HEIGHT); // return bg to original place
}

void FreePlatforms(void) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		free(levelPlatforms.platformArray[i].backgroundData);
		free(levelPlatforms.platformArray[i].processedTileImage);
	}
	free(levelPlatforms.platformArray);
}

void BumpPlatform(player_t* player, uint8_t platformIndex, unsigned int gameFrame) {
	levelPlatforms.platformArray[platformIndex].beingBumped = true;
	levelPlatforms.platformArray[platformIndex].timeOfLastBump = gameFrame;
	if (FIXED_POINT_TO_INT(levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width - player->x) < BLOCK_SIZE)
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = FIXED_POINT_TO_INT(levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width) - BLOCK_SIZE;
	else
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = FIXED_POINT_TO_INT(player->x) - (PLAYER_WIDTH/2);
	
	// detect enemies
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		enemy_t* enemy = &levelEnemies.enemyArray[i];
		if (enemy->grounded && enemy->state != ENEMY_DEAD_SPINNING 
		&& enemy->x + TO_FIXED_POINT(ENEMY_SPIKE_SIZE) > player->x - TO_FIXED_POINT(2) 
		&& enemy->x < player->x + TO_FIXED_POINT(BLOCK_SIZE + 2) 
		&& enemy->y + TO_FIXED_POINT(ENEMY_SPIKE_SIZE) > levelPlatforms.platformArray[platformIndex].y - TO_FIXED_POINT(BLOCK_SIZE) 
		&& enemy->y < levelPlatforms.platformArray[platformIndex].y + PLATFORM_HEIGHT) {
			if (enemy->type == ENEMY_FREEZIE) {
				enemy->eventTime = gameFrame;
				enemy->state = ENEMY_DEAD_SPINNING;
				enemy->verVel = TO_FIXED_POINT(2.5);
				enemy->horVel = 0;
				EnemyShowScore(enemy, player, gameFrame);
				continue;
			}
			float playerVsEnemySlope = 57.2958*atan((enemy->y - player->y)/(enemy->x - player->x)); // arctan???? why did i do this again?
			
			if (enemy->state != ENEMY_LAYING) {
				if (enemy->type == ENEMY_CRAB && !enemy->crabIsMad)
					enemy->crabIsMad = true;
				else {
					enemy->state = ENEMY_LAYING;
					enemy->verSpriteOffset = 0;
					PlayerAddScore(player, 10);
				}
				if (playerVsEnemySlope < -70 || playerVsEnemySlope > 70)
					enemy->horVel = 0;
				else if (playerVsEnemySlope > 0) {
					enemy->horVel = -abs(enemy->maxSpeed);
				} else {
					enemy->horVel = abs(enemy->maxSpeed);
				}
				enemy->layStartTime = gameFrame;
				enemy->grounded = false;
				enemy->sprite = 3;
			} else {
				if (enemy->type == ENEMY_CRAB && enemy->crabIsMad)
					enemy->crabIsMad = false;
				enemy->grounded = false;
				enemy->state = ENEMY_WALKING;
				enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				if (playerVsEnemySlope < -75 || playerVsEnemySlope > 75)
					enemy->horVel = 0;
				else if (playerVsEnemySlope > 0) {
					enemy->horVel = -enemy->maxSpeed;
					enemy->dir = LEFT;
				} else {
					enemy->horVel = enemy->maxSpeed;
					enemy->dir = RIGHT;
				}
			}
			enemy->verVel = (enemy->type != ENEMY_FLY) ? TO_FIXED_POINT(2.5) : TO_FIXED_POINT(1.7);
		}
	}
	
	// detect coins
	for (bonusCoin_t* coin = &levelCoins.coinArray[0]; coin != &levelCoins.coinArray[levelCoins.numCoins]; coin++) {
		if (coin->alive && 
		coin->grounded && 
		coin->state != COIN_EXITING_PIPE && 
		coin->x + TO_FIXED_POINT(COIN_WIDTH) > player->x - TO_FIXED_POINT(BLOCK_SIZE) && 
		coin->x < player->x + TO_FIXED_POINT(2*BLOCK_SIZE) && 
		coin->y + TO_FIXED_POINT(COIN_HEIGHT) > levelPlatforms.platformArray[platformIndex].y - TO_FIXED_POINT(BLOCK_SIZE) && 
		coin->y < levelPlatforms.platformArray[platformIndex].y + TO_FIXED_POINT(PLATFORM_HEIGHT)) {
			coin->shouldDie = true;
			PlayerAddScore(player, 800);
			SpawnParticle(FIXED_POINT_TO_INT(coin->x), FIXED_POINT_TO_INT(coin->y), PARTICLE_COIN_PICK, gameFrame);
			coin->y = TO_FIXED_POINT(241);
		}
	}
}

void RefreshPlatformBackgroundData(uint8_t type) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].y) - PLATFORM_HEIGHT);
		for (uint8_t j = 0; j < FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].width)/BLOCK_SIZE; j++)
			gfx_RLETSprite(platformBlocks[type], FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].x) + j*BLOCK_SIZE, FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].y)); // process image
		gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].y));
		levelPlatforms.platformArray[i].icy = false;
		levelPlatforms.platformArray[i].invisible = false;
	}
}

void FreezePlatform(uint8_t index) {
	levelPlatforms.platformArray[index].icy = true;
	for (uint8_t j = 0; j < FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].width)/BLOCK_SIZE; j++)
		gfx_RLETSprite((rand() % 2 == 0) ? snowy_iced_block1 : snowy_iced_block2, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].x) + j*BLOCK_SIZE, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].y)); // process image
	gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[index].processedTileImage, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].y));
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[index].processedTileImage, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].y));
	gfx_SetDrawBuffer();
}

void VanishPlatform(uint8_t index) {
	levelPlatforms.platformArray[index].invisible = true;
	gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[index].backgroundData, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].y) - PLATFORM_HEIGHT);
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[index].backgroundData, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].y) - PLATFORM_HEIGHT);
	gfx_SetDrawBuffer();
}