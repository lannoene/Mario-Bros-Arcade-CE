#include "platforms.h"

#include <stdlib.h>
#include <graphx.h>

#include "player.h"

levelPlatformData_t levelPlatforms = {};

colision_t CheckColision(int16_t* x, uint8_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel) {
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
			} else if (*y >= levelPlatforms.platformArray[i].y + PLATFORM_HEIGHT) {
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
}

void InitPlatformData(void) {
	levelPlatforms.platformArray = malloc(0);
}

void CreatePlatform(int16_t x, uint8_t y, uint8_t width) {
	++levelPlatforms.numPlatforms;
	levelPlatforms.platformArray = realloc(levelPlatforms.platformArray, levelPlatforms.numPlatforms*sizeof(platform_t));
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].x = x;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].x_old = 0;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].y = y;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].y_old = 0;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].width = width;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData = malloc(width*(PLATFORM_HEIGHT*2) + 2);
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData[0] = width;
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].backgroundData[1] = PLATFORM_HEIGHT*2; // this is to prevent the bumps from staying on the screen, so i'm getting the area above the block in order to restore it. this is super bad, because the bump is only 3x2, but i'm getting the WidthxHeightx2, when i only am using a small fraction of that. TODO: FIX THIS!!!!!
	levelPlatforms.platformArray[levelPlatforms.numPlatforms - 1].beingBumped = false;
}

void FreePlatforms(void) {
	for (uint8_t i = 0; i < levelPlatforms.numPlatforms; i++) {
		free(levelPlatforms.platformArray[i].backgroundData);
	}
	free(levelPlatforms.platformArray);
}

void BumpPlatform(int16_t playerX, uint8_t platformIndex, int gameFrame) {
	gfx_SetTextXY(0, 0);
	levelPlatforms.platformArray[platformIndex].beingBumped = true;
	levelPlatforms.platformArray[platformIndex].timeOfLastBump = gameFrame;
	levelPlatforms.platformArray[platformIndex].bumpedTileXpos = playerX;
}