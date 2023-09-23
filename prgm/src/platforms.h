#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PLATFORM_HEIGHT	8
#define PLATFORM_TILE_WIDTH	8

typedef struct {
	int16_t x, x_old;
	uint8_t y, y_old;
	uint16_t width;
	uint8_t* backgroundData;
	bool beingBumped;
	int16_t bumpedTileXpos;
	int timeOfLastBump;
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

colision_t CheckColision(int16_t* x, uint8_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel);
void InitPlatformData(void);
void CreatePlatform(int16_t x, uint8_t y, uint8_t width);
void FreePlatforms(void);
void BumpPlatform(int16_t playerX, uint8_t platformIndex, int gameFrame);

extern levelPlatformData_t levelPlatforms;