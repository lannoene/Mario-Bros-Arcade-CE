#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "collision.h"

#define PLATFORM_HEIGHT	8
#define BLOCK_SIZE 8

typedef struct player player_t;

typedef struct platform {
	int x, x_old, y, y_old;
	int width;
	uint8_t* backgroundData;
	uint8_t* processedTileImage;
	uint8_t bumpBackgroundData[BLOCK_SIZE*3*BLOCK_SIZE*2 + 2];
	bool beingBumped, needsRefresh, icy, invisible : 1;
	int16_t bumpedTileXpos;
	uint8_t bumpedTileYpos;
	unsigned int timeOfLastBump;
	player_t *lastBumpPlayer;
} platform_t;

typedef struct {
	bool hasColided;
	uint16_t x;
	uint8_t y, colidedSide, colidedIndex;
} colision_t;

typedef struct {
	uint8_t numPlatforms;
	platform_t* platformArray;
} levelPlatformData_t;

enum ICY_PLATFORM_PRESETS {
	NONE_ICY = 0,
	TOP_ICY = 0x1,
	BOTTOM_ICY = 0x2,
	MIDDLE_ICY = 0x4,
	PLATFORMS_ARE_INVISIBLE = 0x8,
};

extern levelPlatformData_t levelPlatforms;

colision_t CheckColision(int16_t* x, int16_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel, bool requireBottomColision);
void InitPlatformData(void);
void CreatePlatform(int16_t x, uint8_t y, uint8_t width);
void FreePlatforms(void);
void BumpPlatform(platform_t *platform, player_t* player, unsigned int gameFrame);
void RefreshPlatformBackgroundData(uint8_t type);
void FreezePlatformIdx(uint8_t index);
void FreezePlatform(platform_t *);
void VanishPlatform(uint8_t index);
void UpdatePlatforms(unsigned int gameFrame);
struct platform_bump_draw {
	uint16_t x;
	uint8_t overflowLR;
};
struct platform_bump_draw CalculatePlatformBumpDrawX(platform_t *);