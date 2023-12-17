#include "platforms.h"

#include <stdlib.h>
#include <graphx.h>
#include <math.h>

#include "enemies.h"
#include "bonus.h"
#include "particles.h"

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
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].x = x;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].x_old = x;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].y = y;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].y_old = y;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].width = width;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData = malloc(width*(PLATFORM_HEIGHT*2) + 2);
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData[0] = width;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData[1] = PLATFORM_HEIGHT*2;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].beingBumped = false;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].needsRefresh = false;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].icy = false;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].invisible = false;
	// preprocess tile image
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage = malloc(width*PLATFORM_HEIGHT*2 + 2);
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage[0] = width;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage[1] = PLATFORM_HEIGHT*2;
	gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData, x, y - PLATFORM_HEIGHT); // get bg
	/*for (uint8_t i = 0; i < width/BLOCK_SIZE; i++)
		gfx_RLETSprite(pipes_block, x + i*BLOCK_SIZE, y); // process image
	gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage, x, y);*/
	//gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData, x, y - PLATFORM_HEIGHT); // return bg to original place
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
	if (levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width - player->x < BLOCK_SIZE)
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width - BLOCK_SIZE;
	else
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = player->x;
	
	// detect enemies
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		if (levelEnemies.enemyArray[i].grounded && levelEnemies.enemyArray[i].state != ENEMY_DEAD_SPINNING && levelEnemies.enemyArray[i].x + ENEMY_SPIKE_SIZE > player->x - 2 && levelEnemies.enemyArray[i].x < player->x + BLOCK_SIZE + 2 && levelEnemies.enemyArray[i].y + ENEMY_SPIKE_SIZE > levelPlatforms.platformArray[platformIndex].y - BLOCK_SIZE && levelEnemies.enemyArray[i].y < levelPlatforms.platformArray[platformIndex].y + PLATFORM_HEIGHT) {
			if (levelEnemies.enemyArray[i].type == ENEMY_FREEZIE) {
				levelEnemies.enemyArray[i].eventTime = gameFrame;
				levelEnemies.enemyArray[i].state = ENEMY_DEAD_SPINNING;
				levelEnemies.enemyArray[i].verAccel = 2.5;
				levelEnemies.enemyArray[i].horAccel = 0;
				EnemyShowScore(i, player, gameFrame);
				continue;
			}
			float playerVsEnemySlope = 57.2958*atan((levelEnemies.enemyArray[i].y - player->y)/(levelEnemies.enemyArray[i].x - player->x));
			
			if (levelEnemies.enemyArray[i].state != ENEMY_LAYING) {
				if (levelEnemies.enemyArray[i].type == ENEMY_CRAB && !levelEnemies.enemyArray[i].crabIsMad)
					levelEnemies.enemyArray[i].crabIsMad = true;
				else {
					levelEnemies.enemyArray[i].state = ENEMY_LAYING;
					levelEnemies.enemyArray[i].verSpriteOffset = 0;
					PlayerAddScore(player, 10);
				}
				if (playerVsEnemySlope < -70 || playerVsEnemySlope > 70)
					levelEnemies.enemyArray[i].horAccel = 0;
				else if (playerVsEnemySlope > 0) {
					levelEnemies.enemyArray[i].horAccel = -fabsf(levelEnemies.enemyArray[i].maxSpeed);
				} else {
					levelEnemies.enemyArray[i].horAccel = fabsf(levelEnemies.enemyArray[i].maxSpeed);
				}
				levelEnemies.enemyArray[i].verAccel = (levelEnemies.enemyArray[i].type != ENEMY_FLY) ? 2.5 : 1.7;
				levelEnemies.enemyArray[i].layStartTime = gameFrame;
				levelEnemies.enemyArray[i].grounded = false;
				levelEnemies.enemyArray[i].sprite = 3;
			} else {
				if (levelEnemies.enemyArray[i].type == ENEMY_CRAB && levelEnemies.enemyArray[i].crabIsMad)
					levelEnemies.enemyArray[i].crabIsMad = false;
				levelEnemies.enemyArray[i].grounded = false;
				levelEnemies.enemyArray[i].state = ENEMY_WALKING;
				levelEnemies.enemyArray[i].verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				if (playerVsEnemySlope < -75 || playerVsEnemySlope > 75)
					levelEnemies.enemyArray[i].horAccel = 0;
				else if (playerVsEnemySlope > 0) {
					levelEnemies.enemyArray[i].horAccel = -levelEnemies.enemyArray[i].maxSpeed;
					levelEnemies.enemyArray[i].dir = LEFT;
				} else {
					levelEnemies.enemyArray[i].horAccel = levelEnemies.enemyArray[i].maxSpeed;
					levelEnemies.enemyArray[i].dir = RIGHT;
				}
			}
			levelEnemies.enemyArray[i].verAccel = (levelEnemies.enemyArray[i].type != ENEMY_FLY) ? 2.5 : 1.7;
		}
	}
	
	// detect coins
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive && levelCoins.coinArray[i].grounded && levelCoins.coinArray[i].state != COIN_EXITING_PIPE && levelCoins.coinArray[i].x + COIN_WIDTH > player->x - BLOCK_SIZE && levelCoins.coinArray[i].x < player->x + 2*BLOCK_SIZE && levelCoins.coinArray[i].y + COIN_HEIGHT > levelPlatforms.platformArray[platformIndex].y - BLOCK_SIZE && levelCoins.coinArray[i].y < levelPlatforms.platformArray[platformIndex].y + PLATFORM_HEIGHT) {
			levelCoins.coinArray[i].shouldDie = true;
			PlayerAddScore(player, 800);
			SpawnParticle(levelCoins.coinArray[i].x, levelCoins.coinArray[i].y, PARTICLE_COIN_PICK, gameFrame);
			levelCoins.coinArray[i].y = 241;
		}
	}
}

void RefreshPlatformBackgroundData(uint8_t type) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y - PLATFORM_HEIGHT);
		for (uint8_t j = 0; j < levelPlatforms.platformArray[i].width/BLOCK_SIZE; j++)
			gfx_RLETSprite(platformBlocks[type], levelPlatforms.platformArray[i].x + j*BLOCK_SIZE, levelPlatforms.platformArray[i].y); // process image
		gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y);
		levelPlatforms.platformArray[i].icy = false;
		levelPlatforms.platformArray[i].invisible = false;
	}
}

void FreezePlatform(uint8_t index) {
	levelPlatforms.platformArray[index].icy = true;
	for (uint8_t j = 0; j < levelPlatforms.platformArray[index].width/BLOCK_SIZE; j++)
		gfx_RLETSprite((rand() % 2 == 0) ? snowy_iced_block1 : snowy_iced_block2, levelPlatforms.platformArray[index].x + j*BLOCK_SIZE, levelPlatforms.platformArray[index].y); // process image
	gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[index].processedTileImage, levelPlatforms.platformArray[index].x, levelPlatforms.platformArray[index].y);
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[index].processedTileImage, levelPlatforms.platformArray[index].x, levelPlatforms.platformArray[index].y);
	gfx_SetDrawBuffer();
}

void VanishPlatform(uint8_t index) {
	levelPlatforms.platformArray[index].invisible = true;
	gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[index].backgroundData, levelPlatforms.platformArray[index].x, levelPlatforms.platformArray[index].y - PLATFORM_HEIGHT);
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[index].backgroundData, levelPlatforms.platformArray[index].x, levelPlatforms.platformArray[index].y - PLATFORM_HEIGHT);
	gfx_SetDrawBuffer();
}