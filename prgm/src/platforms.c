#include "platforms.h"

#include <stdbool.h>

platform_t platformArray[2] = {{0, 0, 180, 0, PLATFORM_WIDTH, {PLATFORM_WIDTH, PLATFORM_HEIGHT}}, {0, 0, 0, 0, PLATFORM_WIDTH, {PLATFORM_WIDTH, PLATFORM_HEIGHT}}}; // x, x_old, y, y_old
uint8_t numPlatforms = 2;

int16_t CheckColision(int16_t* x, uint8_t* y, uint8_t width, uint8_t height, float* verAccel, float* horAccel) {
	// checking x for offscreen transition to other side
	if (*x + width < 0)
		*x = 320; // if they go offscreen to the left, teleport them to the right side
	else if (*x > 320)
		*x = 0 - width; // if they go offscreen to the right, teleport them to the left side
	
	bool isColiding = false;
	// checking for bottom ground
	if (*y - *verAccel > 224 - height) {
		return 224;
		isColiding = true;
	}
	
	for (uint8_t i = 0; i < numPlatforms; i++) {
		if (*x + *horAccel > platformArray[i].x - width && *x + *horAccel < platformArray[i].x + PLATFORM_WIDTH  && *y - *verAccel > platformArray[i].y - height && *y - *verAccel < platformArray[i].y + PLATFORM_HEIGHT) {
			return platformArray[i].y;
		}
	}
	
	if (isColiding)
		return *y;
	else
		return -1;
}
