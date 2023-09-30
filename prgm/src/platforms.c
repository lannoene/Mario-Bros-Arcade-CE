#include "platforms.h"

#include <stdlib.h>
#include <graphx.h>
#include <math.h>

#include "enemies.h"

#include "gfx/gfx.h"

levelPlatformData_t levelPlatforms = {};
/* ---- CURRENTLY UNUSED AS COLISION IS VERY EXPENSIVE AND HAVING IT IN ANOTHER FUNCTION JUST SLOWED THINGS DOWN ----
colision_t CheckColision(int16_t* x, int16_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel, bool requireBottomColision) {
	
	colision_t retStruct = {false, 0, 0, 0, 0};
	// checking x for offscreen transition to other side
	if (*x + width < 0)
		*x = 320; // if they go offscreen to the left, teleport them to the right side
	else if (*x > 320)
		*x = 0 - width; // if they go offscreen to the right, teleport them to the left side
	
	// checking for bottom ground
	if (*y - *verAccel > 224 - height) {
		retStruct.hasColided = true;
		retStruct.x = 0;
		retStruct.y = 224;
		retStruct.colidedSide = UP;
		retStruct.colidedIndex = 0;
		return retStruct;
	}
	
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		if (*x + *horAccel > levelPlatforms.platformArray[i].x - width && *x + *horAccel < levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width && *y - *verAccel > levelPlatforms.platformArray[i].y - height && *y - *verAccel < levelPlatforms.platformArray[i].y + PLATFORM_HEIGHT) {
			if (*y + height <= levelPlatforms.platformArray[i].y) {
				retStruct.hasColided = true;
				retStruct.x = levelPlatforms.platformArray[i].x;
				retStruct.y = levelPlatforms.platformArray[i].y;
				retStruct.colidedSide = UP;
				retStruct.colidedIndex = i;
				return retStruct;
			} else if (requireBottomColision && *y >= levelPlatforms.platformArray[i].y + PLATFORM_HEIGHT) {
				retStruct.hasColided = true;
				retStruct.x = levelPlatforms.platformArray[i].x;
				retStruct.y = levelPlatforms.platformArray[i].y;
				retStruct.colidedSide = DOWN;
				retStruct.colidedIndex = i;
				return retStruct;
			}
		}
	}
	
	return retStruct;
	
}*/

void InitPlatformData(void) {
	levelPlatforms.platformArray = malloc(0);
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
	// preprocess tile image
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage = malloc(width*PLATFORM_HEIGHT*2 + 2);
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage[0] = width;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage[1] = PLATFORM_HEIGHT*2;
	gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData, x, y); // get bg
	for (uint8_t i = 0; i < width/BLOCK_SIZE; i++)
		gfx_Sprite((gfx_sprite_t*)level1_block, x + i*BLOCK_SIZE, y); // process image
	gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].processedTileImage, x, y - PLATFORM_HEIGHT);
	//gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData, x, y); // return bg to original place
}

void FreePlatforms(void) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		free(levelPlatforms.platformArray[i].backgroundData);
		free(levelPlatforms.platformArray[i].processedTileImage);
	}
	free(levelPlatforms.platformArray);
}

void BumpPlatform(player_t* player, uint8_t platformIndex, unsigned int gameFrame) {
	//player->x += PLAYER_WIDTH/2; // player x is the player's left x. so to make it the middle, we add half the player's width
	levelPlatforms.platformArray[platformIndex].beingBumped = true;
	levelPlatforms.platformArray[platformIndex].timeOfLastBump = gameFrame;
	if (levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width - player->x < BLOCK_SIZE)
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width - BLOCK_SIZE;
	else
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = player->x;
	
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		if (levelEnemies.enemyArray[i].grounded && levelEnemies.enemyArray[i].x + ENEMY_SPIKE_SIZE > player->x - BLOCK_SIZE && levelEnemies.enemyArray[i].x < player->x + 2*BLOCK_SIZE && levelEnemies.enemyArray[i].y + ENEMY_SPIKE_SIZE > levelPlatforms.platformArray[platformIndex].y - BLOCK_SIZE && levelEnemies.enemyArray[i].y < levelPlatforms.platformArray[platformIndex].y + PLATFORM_HEIGHT) {
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
					levelEnemies.enemyArray[i].horAccel = -fabsf(levelEnemies.enemyArray[i].horAccel);
				} else {
					levelEnemies.enemyArray[i].horAccel = fabsf(levelEnemies.enemyArray[i].horAccel);
				}
				levelEnemies.enemyArray[i].verAccel = 2.5;
				levelEnemies.enemyArray[i].layStartTime = gameFrame;
				levelEnemies.enemyArray[i].grounded = false;
				levelEnemies.enemyArray[i].sprite = 3;
			} else {
				if (levelEnemies.enemyArray[i].type == ENEMY_CRAB && levelEnemies.enemyArray[i].crabIsMad)
					levelEnemies.enemyArray[i].crabIsMad = false;
				levelEnemies.enemyArray[i].grounded = false;
				levelEnemies.enemyArray[i].state = ENEMY_WALKING;
				levelEnemies.enemyArray[i].verAccel = 2.5;
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
		}
	}
}