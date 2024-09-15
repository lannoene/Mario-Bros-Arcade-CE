#include "platforms.h"

#include <stdlib.h>
#include <graphx.h>
#include <math.h>
#include <sys/util.h>

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
	int usX = FIXED_POINT_TO_INT(player->x) + (PLAYER_WIDTH/2);
	levelPlatforms.platformArray[platformIndex].beingBumped = true;
	levelPlatforms.platformArray[platformIndex].timeOfLastBump = gameFrame;
	if (FIXED_POINT_TO_INT(levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width - usX) < BLOCK_SIZE)
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = FIXED_POINT_TO_INT(levelPlatforms.platformArray[platformIndex].x + levelPlatforms.platformArray[platformIndex].width) - BLOCK_SIZE;
	else
		levelPlatforms.platformArray[platformIndex].bumpedTileXpos = usX;
	levelPlatforms.platformArray[platformIndex].bumpedTileYpos = FP2I(player->y);
	levelPlatforms.platformArray[platformIndex].lastBumpPlayer = player;
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
		gfx_RLETSprite((randInt(0, 1) == 0) ? snowy_iced_block1 : snowy_iced_block2, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].x) + j*BLOCK_SIZE, FIXED_POINT_TO_INT(levelPlatforms.platformArray[index].y)); // process image
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

static void RunPlatformBump(platform_t* platform, unsigned int gameFrame) {
	const int leeway = I2FP(BLOCK_SIZE);
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		enemy_t* enemy = &levelEnemies.enemyArray[i];
		if (enemy->grounded && enemy->state != ENEMY_DEAD_SPINNING 
		&& enemy->x + enemy->width > I2FP(platform->bumpedTileXpos) - leeway
		&& enemy->x < I2FP(platform->bumpedTileXpos) + leeway
		&& enemy->y + enemy->height > platform->y - TO_FIXED_POINT(BLOCK_SIZE) 
		&& enemy->y < platform->y + PLATFORM_HEIGHT) {
			if (enemy->type == ENEMY_FREEZIE) {
				enemy->eventTime = gameFrame;
				enemy->state = ENEMY_DEAD_SPINNING;
				enemy->verVel = I2FP(2.5);
				enemy->horVel = 0;
				EnemyShowScore(enemy, platform->lastBumpPlayer, gameFrame);
				continue;
			} else if (enemy->type == ENEMY_COIN) {
				if (!enemy->bonus) {
					PlayerAddScore(platform->lastBumpPlayer, 800);
				}
				enemy->state = ENEMY_DEAD_SPINNING;
				continue;
			}
			// slope ranges [40, -70), -90 is vertical
			float playerVsEnemySlope = atan((float)FP2I(enemy->y - I2FP(platform->bumpedTileYpos))/FP2I(enemy->x - I2FP(platform->bumpedTileXpos)))*180.0 / M_PI;
			//dbg_printf("hit: %f x dist %d y dist: %d\n", playerVsEnemySlope, (enemy->x - I2FP(platform->bumpedTileXpos)), (enemy->y - I2FP(platform->bumpedTileYpos)));
			#define SLOPE_RANGE 60 // make this larger for less up-hitting area
			
			if (playerVsEnemySlope < SLOPE_RANGE && playerVsEnemySlope > 0) {
				enemy->horVel = -abs(enemy->maxSpeed);
				if (enemy->state == ENEMY_LAYING)
					enemy->dir = LEFT;
			} else if (playerVsEnemySlope < 0 && playerVsEnemySlope < -SLOPE_RANGE) {
				enemy->horVel = abs(enemy->maxSpeed);
				if (enemy->state == ENEMY_LAYING)
					enemy->dir = RIGHT;
			} else
				enemy->horVel = 0;
			if (enemy->state != ENEMY_LAYING) {
				if (enemy->type == ENEMY_CRAB && !enemy->crabIsMad)
					enemy->crabIsMad = true;
				else {
					enemy->state = ENEMY_LAYING;
					enemy->verSpriteOffset = 0;
					PlayerAddScore(platform->lastBumpPlayer, 10);
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
			}
			enemy->verVel = (enemy->type != ENEMY_FLY) ? TO_FIXED_POINT(2.5) : TO_FIXED_POINT(1.7);
		}
	}
}

void UpdatePlatforms(unsigned int gameFrame) {
	for (int i = 0; i < levelPlatforms.numPlatforms; i++) {
		platform_t *platform = &levelPlatforms.platformArray[i];
		if (platform->beingBumped) {
			// detect enemies
			RunPlatformBump(platform, gameFrame);
		}
	}
}