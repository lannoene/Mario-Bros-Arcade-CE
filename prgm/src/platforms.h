#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define PLATFORM_HEIGHT	8
#define BLOCK_SIZE 8
#define GROUND_HEIGHT 224

typedef struct {
	int16_t x, x_old;
	uint8_t y, y_old;
	uint16_t width;
	uint8_t* backgroundData;
	uint8_t* processedTileImage;
	bool beingBumped, needsRefresh, icy, invisible : 1;
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
	NONE_ICY = 0,
	TOP_ICY = 0x1,
	BOTTOM_ICY = 0x2,
	MIDDLE_ICY = 0x4,
	PLATFORMS_ARE_INVISIBLE = 0x8,
	ICICLES_FORM_LOW = 0x10,
};

extern levelPlatformData_t levelPlatforms;

colision_t CheckColision(int16_t* x, int16_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel, bool requireBottomColision);
void InitPlatformData(void);
void CreatePlatform(int16_t x, uint8_t y, uint8_t width);
void FreePlatforms(void);
void BumpPlatform(player_t* player, uint8_t platformIndex, unsigned int gameFrame);
void RefreshPlatformBackgroundData(uint8_t type);
void FreezePlatform(uint8_t index);
void VanishPlatform(uint8_t index);