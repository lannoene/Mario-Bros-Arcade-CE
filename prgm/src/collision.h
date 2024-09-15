#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	int x, x_old, y, y_old;
	int verVel, horVel;
	uint8_t width, height;
	int8_t lastGroundedPlatformIndex;
	bool grounded;
	int groundedStartTime;
} position_t;

enum COLLISION_TYPE_IDS {
	NO_COL = 0,
	EXP_COL, // expensive collision
	CHP_COL, // cheap collision
	BTM_COL // bottom collision (collision with the bottom barrier)
};

uint8_t CalcCollision(position_t *pos, int gameFrame);