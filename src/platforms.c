#include "platforms.h"

#include <stdlib.h>
#include <graphx.h>
#include <math.h>
#include <sys/util.h>

#include "enemies.h"
#include "bonus.h"
#include "particles.h"
#include "defines.h"
#include "level.h" // lvl_s_alloc()
#include "fireballs.h"

#include "gfx/gfx.h"

levelPlatformData_t levelPlatforms = {};

gfx_rletsprite_t* platformBlocks[4] = {pipes_block, lava_block, castle_block, snowy_normal_block};
platform_t platformArray[7];

void InitPlatformData(void) {
	levelPlatforms.platformArray = platformArray;
	levelPlatforms.numPlatforms = 0;
}

void CreatePlatform(int16_t x, uint8_t y, uint8_t width) {
	if (levelPlatforms.numPlatforms == ARR_LEN(platformArray)) {
		dbg_printf("The maximum number of platforms has been reached.\n");
		return;
	}
	++levelPlatforms.numPlatforms;
	platform_t* platform = &levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1];
	platform->x = I2FP(x);
	platform->x_old = I2FP(x);
	platform->y = I2FP(y);
	platform->y_old = I2FP(y);
	platform->width = I2FP(width);
	platform->backgroundData = lvl_s_alloc(width*(PLATFORM_HEIGHT) + 2);
	if (!platform->backgroundData) {
		dbg_printf("Could not allocate platform background data\n");
	}
	platform->backgroundData[0] = width;
	platform->backgroundData[1] = PLATFORM_HEIGHT;
	platform->beingBumped = false;
	platform->needsRefresh = false;
	platform->icy = false;
	platform->invisible = false;
	// preprocess tile image
	platform->processedTileImage = lvl_s_alloc(width*PLATFORM_HEIGHT + 2);
	platform->processedTileImage[0] = width;
	platform->processedTileImage[1] = PLATFORM_HEIGHT;
	gfx_GetSprite((gfx_sprite_t*)platform->backgroundData, x, y); // get bg
}

void FreePlatforms(void) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		//free(levelPlatforms.platformArray[i].backgroundData);
		//free(levelPlatforms.platformArray[i].processedTileImage);
	}
	//free(levelPlatforms.platformArray);
}

void BumpPlatform(platform_t *platform, player_t* player, unsigned int gameFrame) {
	if (platform->beingBumped)
		return;
	int usX = FP2I(player->x) + (PLAYER_WIDTH/2);
	platform->beingBumped = true;
	platform->timeOfLastBump = gameFrame;
	if (FP2I(platform->x + platform->width - usX) < BLOCK_SIZE)
		platform->bumpedTileXpos = FP2I(platform->x + platform->width) - BLOCK_SIZE;
	else
		platform->bumpedTileXpos = usX;
	platform->bumpedTileYpos = FP2I(player->y);
	platform->lastBumpPlayer = player;
}

void RefreshPlatformBackgroundData(uint8_t type) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		platform_t *platform = &levelPlatforms.platformArray[i];
		gfx_GetSprite((gfx_sprite_t*)platform->backgroundData, FP2I(platform->x), FP2I(platform->y));
		for (uint8_t j = 0; j < FP2I(platform->width)/BLOCK_SIZE; j++)
			gfx_RLETSprite(platformBlocks[type], FP2I(platform->x) + j*BLOCK_SIZE, FP2I(platform->y)); // process image
		gfx_GetSprite((gfx_sprite_t*)platform->processedTileImage, FP2I(platform->x), FP2I(platform->y));
		platform->icy = false;
		platform->invisible = false;
	}
}

void FreezePlatformIdx(uint8_t i) {
	FreezePlatform(&levelPlatforms.platformArray[i]);
}

void FreezePlatform(platform_t *platform) {
	platform->icy = true;
	for (uint8_t j = 0; j < FP2I(platform->width)/BLOCK_SIZE; j++)
		gfx_RLETSprite((randInt(0, 1) == 0) ? snowy_iced_block1 : snowy_iced_block2, FP2I(platform->x) + j*BLOCK_SIZE, FP2I(platform->y)); // process image
	gfx_GetSprite((gfx_sprite_t*)platform->processedTileImage, FP2I(platform->x), FP2I(platform->y));
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)platform->processedTileImage, FP2I(platform->x), FP2I(platform->y));
	gfx_SetDrawBuffer();
}

void VanishPlatform(uint8_t index) {
	platform_t *platform = &levelPlatforms.platformArray[index];
	platform->invisible = true;
	gfx_Sprite((gfx_sprite_t*)platform->backgroundData, FP2I(platform->x), FP2I(platform->y));
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)platform->backgroundData, FP2I(platform->x), FP2I(platform->y));
	gfx_SetDrawBuffer();
}

static void RunPlatformBump(platform_t* platform, unsigned int gameFrame) {
	const int leeway = I2FP(BLOCK_SIZE);
	FOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
		if (enemy->grounded
		&& AABB(I2FP(platform->bumpedTileXpos) - leeway, platform->y - I2FP(PLATFORM_HEIGHT), leeway*2, I2FP(PLATFORM_HEIGHT)*2,
		enemy->x, enemy->y, enemy->width, enemy->height)) {
			if (enemy->type == ENEMY_FREEZIE) {
				enemy->eventTime = gameFrame;
				enemy->state = ENEMY_DEAD_SPINNING;
				enemy->verVel = I2FP(2.5);
				enemy->horVel = 0;
				EnemyShowScoreIndividual(enemy, platform->lastBumpPlayer, 500, gameFrame);
				continue;
			} else if (enemy->type == ENEMY_COIN) {
				CollectCoin(platform->lastBumpPlayer, enemy);
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
			enemy->verVel = (enemy->type != ENEMY_FLY) ? I2FP(2.5) : I2FP(1.7);
		}
	}
	
	FOR_EACH(levelFireballs.fireballArray, levelFireballs.numFireballs, fireball) {
		if (AABB(
			platform->bumpedTileXpos - BLOCK_SIZE, FP2I(platform->y) - PLATFORM_HEIGHT, BLOCK_SIZE*2, PLATFORM_HEIGHT*2,
			fireball->x, fireball->y, FIREBALL_SIZE, FIREBALL_SIZE)) {
			KillFireball(fireball, gameFrame);
		}
	}
	if ((gameFrame - platform->timeOfLastBump)/4 == 5) {
		platform->beingBumped = false;
		platform->needsRefresh = true;
	}
}

void UpdatePlatforms(unsigned int gameFrame) {
	FOR_EACH(levelPlatforms.platformArray, levelPlatforms.numPlatforms, platform) {
		if (platform->beingBumped) {
			// detect enemies
			RunPlatformBump(platform, gameFrame);
		}
	}
}

struct platform_bump_draw CalculatePlatformBumpDrawX(platform_t *platform) {
	uint16_t platFxToIntX = FP2I(platform->x);
	uint8_t platFxToIntY = FP2I(platform->y);
	uint8_t platFxToIntWidth = FP2I(platform->width);
	int16_t flooredXpos = ((platform->bumpedTileXpos - PLAYER_WIDTH/4)/BLOCK_SIZE)*BLOCK_SIZE;
	uint16_t drawX = flooredXpos;
	uint8_t overflowLR = NONE;
	if (flooredXpos - platFxToIntX >= platFxToIntWidth - 2*BLOCK_SIZE) {
		overflowLR = RIGHT;
		drawX = platFxToIntX + platFxToIntWidth - (2*BLOCK_SIZE);
	} else if (flooredXpos - platFxToIntX < 0) {
		overflowLR = LEFT;
		drawX = platFxToIntX - BLOCK_SIZE;
	}
	struct platform_bump_draw ret = {drawX, overflowLR};
	return ret;
}