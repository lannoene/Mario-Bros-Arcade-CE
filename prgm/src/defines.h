#pragma once
// garbage filename

// direction definitions
#define RIGHT	0
#define LEFT	1
#define UP		2
#define DOWN	3
#define NONE	4
#define NOJUMP	5 // for player control stuff

#define GROUND_HEIGHT 232

// fixed point definitions
#define FIXED_POINT_UNIT_SIZE 256

#define TO_FIXED_POINT(x) ((x)*256)
#define FIXED_POINT_TO_INT(x) ((x)/256)