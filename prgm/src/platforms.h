#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define PLATFORM_HEIGHT	8
#define BLOCK_SIZE 8

typedef struct {
	int16_t x, x_old;
	uint8_t y, y_old;
	uint16_t width;
	uint8_t* backgroundData;
	uint8_t* processedTileImage;
	bool beingBumped, needsRefresh, icy : 1;
	int16_t bumpedTileXpos;
	unsigned int timeOfLastBump;
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
	NONE_ICY,
	TOP_ICY,
	BOTTOM_ICY,
	MIDDLE_ICY,
	TOP_BOTTOM_ICY,
	MIDDLE_BOTTOM_ICY,
	TOP_MIDDLE_ICY,
	ALL_ICY,
};

colision_t CheckColision(int16_t* x, int16_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel, bool requireBottomColision);
void InitPlatformData(void);
void CreatePlatform(int16_t x, uint8_t y, uint8_t width);
void FreePlatforms(void);
void BumpPlatform(player_t* player, uint8_t platformIndex, unsigned int gameFrame);
void RefreshPlatformBackgroundData(uint8_t type);
void FreezePlatform(uint8_t index);

extern levelPlatformData_t levelPlatforms;