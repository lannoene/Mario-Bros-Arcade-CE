#include "collision.h"

#include "platforms.h"
#include "defines.h"

uint8_t CalcCollision(position_t *pos, int gameFrame) {
	if (pos->x < -pos->width) // pos on sides off screen? if so, teleport them to other side
		pos->x = TO_FIXED_POINT(GFX_LCD_WIDTH);
	else if (pos->x > TO_FIXED_POINT(GFX_LCD_WIDTH))
		pos->x = -pos->width; // 0 - pos->width
	/*
	// if pos is on ground and is out of view, do the thing idk
	if ((pos->x + pos->horVel >= TO_FIXED_POINT(GFX_LCD_WIDTH) || pos->x + pos->horVel <= -pos->width) && pos->y > TO_FIXED_POINT(176 + PLATFORM_HEIGHT)) { // 176 = bottom platforms' height. i dont wanna make macro though
		EnterRespawnPipe(pos, gameFrame);
	}*/
	
	if (pos->grounded 
	&& pos->verVel <= 0 
	&& pos->x + pos->horVel + pos->width > levelPlatforms.platformArray[pos->lastGroundedPlatformIndex].x 
	&& pos->x + pos->horVel < levelPlatforms.platformArray[pos->lastGroundedPlatformIndex].x + levelPlatforms.platformArray[pos->lastGroundedPlatformIndex].width) { // is pos still inside of last intersected platform's x footprint? if so, make sure they stay like so
		pos->verVel = 0;
		return CHP_COL;
	} else if (pos->y - pos->verVel > TO_FIXED_POINT(GROUND_HEIGHT - pos->height)) { // is pos on the bottom floor? if so, we can skip colision checking and make sure verVel is 0
		pos->y = TO_FIXED_POINT(GROUND_HEIGHT - pos->height);
		pos->verVel = 0;
		pos->grounded = true;
		return BTM_COL;
	} else { // otherwise, do expensive colision checking
		if (pos->verVel < 0) { // only test for physics if the verVel is less than 0
			uint8_t j = 0;
			for (; j < levelPlatforms.numPlatforms; j++) {
				if (pos->y - pos->verVel + TO_FIXED_POINT(pos->height) > levelPlatforms.platformArray[j].y 
				&& pos->y - pos->verVel < levelPlatforms.platformArray[j].y + TO_FIXED_POINT(PLATFORM_HEIGHT) 
				&& pos->x + pos->horVel + pos->width > levelPlatforms.platformArray[j].x 
				&& pos->x + pos->horVel < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
					pos->y = levelPlatforms.platformArray[j].y - TO_FIXED_POINT(pos->height);
					pos->verVel = 0;
					pos->grounded = true;
					pos->lastGroundedPlatformIndex = j;
					pos->groundedStartTime = gameFrame;
					return EXP_COL;
				}
			}
			if (j == levelPlatforms.numPlatforms && pos->grounded == true) {
				pos->grounded = false;
				return NO_COL;
			}
		}
	}
	return 255;
}