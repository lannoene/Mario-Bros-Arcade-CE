#include "fireballs.h"

#include <stdlib.h>
#include <math.h>
#include <graphx.h>

#include "platforms.h"
#include "level.h"
#include "defines.h"

#define INITIAL_FIREBALL_SPAWN_WEIGHT 10000
#define FIREBALL_SPEED 1

levelFireballInfo_t levelFireballs;

void InitFireballs(void) {
	levelFireballs.numFireballs = 0;
	levelFireballs.fireballArray = malloc(0);
	levelFireballs.fireballSpawnWeight = INITIAL_FIREBALL_SPAWN_WEIGHT;
}

void CreateFireball(uint8_t y, bool dir, uint8_t type, unsigned int gameFrame) {
	uint8_t i;
	for (i = 0; i < levelFireballs.numFireballs; i++) {
		if (!levelFireballs.fireballArray[i].alive)
			break;
	}
	if (i == levelFireballs.numFireballs) {
		++levelFireballs.numFireballs;
		levelFireballs.fireballArray = realloc(levelFireballs.fireballArray, levelFireballs.numFireballs*sizeof(fireball_t));
	}
	
	if (type == FIREBALL_GREEN) {
		if (dir == LEFT)
			levelFireballs.fireballArray[i].x = 314;
		else
			levelFireballs.fireballArray[i].x = 0;
	} else {
		levelFireballs.fireballArray[i].verDir = DOWN;
		levelFireballs.fireballArray[i].x = 314;
	}
	levelFireballs.fireballArray[i].alive = true;
	levelFireballs.fireballArray[i].state = FIREBALL_SPAWNING;
	levelFireballs.fireballArray[i].y = levelFireballs.fireballArray[i].y_old = levelFireballs.fireballArray[i].original_y = y;
	levelFireballs.fireballArray[i].spawnTime = gameFrame;
	levelFireballs.fireballArray[i].backgroundData[0] = FIREBALL_SIZE;
	levelFireballs.fireballArray[i].backgroundData[1] = FIREBALL_SIZE;
	levelFireballs.fireballArray[i].type = type;
	levelFireballs.fireballArray[i].horDir = dir;
	
	levelFireballs.fireballSpawnWeight = INITIAL_FIREBALL_SPAWN_WEIGHT;
}

int16_t fpsin(int16_t i) { // source: https://nullhardware.com/blog/fixed-point-sine-and-cosine-for-embedded-systems
    /* Convert (signed) input to a value between 0 and 8192. (8192 is pi/2, which is the region of the curve fit). */
    /* ------------------------------------------------------------------- */
    i <<= 1;
    uint8_t c = i<0; //set carry for output pos/neg

    if(i == (i|0x4000)) // flip input value to corresponding value in range [0..8192)
        i = (1<<15) - i;
    i = (i & 0x7FFF) >> 1;
    /* ------------------------------------------------------------------- */

    /* The following section implements the formula:
     = y * 2^-n * ( A1 - 2^(q-p)* y * 2^-n * y * 2^-n * [B1 - 2^-r * y * 2^-n * C1 * y]) * 2^(a-q)
    Where the constants are defined as follows:
    */
    enum {A1=3370945099UL, B1=2746362156UL, C1=292421UL};
    enum {n=13, p=32, q=31, r=3, a=12};

    uint32_t y = (C1*((uint32_t)i))>>n;
    y = B1 - (((uint32_t)i*y)>>r);
    y = (uint32_t)i * (y>>n);
    y = (uint32_t)i * (y>>n);
    y = A1 - (y>>(p-q));
    y = (uint32_t)i * (y>>n);
    y = (y+(1UL<<(q-a-1)))>>(q-a); // Rounding

    return c ? -y : y;
}

void UpdateFireballs(player_t* player, unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelFireballs.numFireballs; i++) {
		fireball_t* fireball = &levelFireballs.fireballArray[i];
		if (!fireball->alive || gameFrame - game_data.levelStartTime < 150)
			continue;
		if (fireball->type == FIREBALL_GREEN) {
			if (gameFrame - fireball->spawnTime < 60) {
				
			} else if (gameFrame - fireball->spawnTime == 60) {
				fireball->state = FIREBALL_MOVING;
			} else {
				if (fireball->horDir == LEFT) {
					--fireball->x;
					if (fireball->x < -FIREBALL_SIZE)
						fireball->alive = false;
				} else {
					if (fireball->x > 320)
						fireball->alive = false;
					++fireball->x;
				}
				/* this is so painful. also i give up. it's 4 am
				int sinOfY = fpsin(TO_FIXED_POINT(fireball->x)); // except for here... idk how to get rid of this one
				fireball->y = FIXED_POINT_TO_INT(TO_FIXED_POINT(fireball->original_y) + -(sinOfY + FIXED_POINT_TO_INT(sinOfY*sinOfY)) /* sinOfY*sinOfY has 2 fixed point mults in it, 
				so we need to divide by fp m 2 times to get back to normal );*/
				float sinOfY = sin((float)fireball->x/10);
				fireball->y = fireball->original_y + -13*(sinOfY + sinOfY*sinOfY);
			}
		} else {
			if (gameFrame - fireball->spawnTime < 60) {
				
			} else if (gameFrame - fireball->spawnTime == 60) {
				fireball->state = FIREBALL_MOVING;
			} else {
				if (fireball->x <= 0)
					fireball->horDir = RIGHT;
				else if (fireball->x > 320 - FIREBALL_SIZE)
					fireball->horDir = LEFT;
				
				if (fireball->y <= 0)
					fireball->verDir = DOWN;
				else if (fireball->y > GROUND_HEIGHT - FIREBALL_SIZE)
					fireball->verDir = UP;
				
				
				for (uint8_t j = 0; j < levelPlatforms.numPlatforms; j++) {
					platform_t* platform = &levelPlatforms.platformArray[j];
					uint16_t platformX = FIXED_POINT_TO_INT(platform->x);
					uint8_t platformY = FIXED_POINT_TO_INT(platform->y);
					uint16_t platformWidth = FIXED_POINT_TO_INT(platform->width);
					if (fireball->y + FIREBALL_SIZE + ((fireball->verDir == DOWN) ? FIREBALL_SPEED : -FIREBALL_SPEED) > platformY && 
					fireball->y + ((fireball->verDir == DOWN) ? FIREBALL_SPEED : -FIREBALL_SPEED) < platformY + PLATFORM_HEIGHT && 
					fireball->x + FIREBALL_SIZE + ((fireball->horDir == RIGHT) ? FIREBALL_SPEED : -FIREBALL_SPEED) > platformX && 
					fireball->x + ((fireball->horDir == RIGHT) ? FIREBALL_SPEED : -FIREBALL_SPEED) < platformX + platformWidth) {
						if (fireball->verDir == UP && fireball->y >= platformY + PLATFORM_HEIGHT)
							fireball->verDir = DOWN;
						else if (fireball->verDir == DOWN && fireball->y + FIREBALL_SIZE <= platformY)
							fireball->verDir = UP;
						else if (fireball->horDir == LEFT && fireball->x >= platformX + platformWidth)
							fireball->horDir = RIGHT;
						else if (fireball->horDir == RIGHT && fireball->x + FIREBALL_SIZE <= platformX)
							fireball->horDir = LEFT;
						break;
					}
				}
				
				if (gameFrame % 2 == 0) { // avoid using floats
					if (fireball->horDir == RIGHT)
						++fireball->x;
					else
						--fireball->x;
					
					if (fireball->verDir == UP)
						--fireball->y;
					else
						++fireball->y;
				}
			}
		}
		int playerY = FIXED_POINT_TO_INT(player->y);
		int playerX = FIXED_POINT_TO_INT(player->x);
		if (playerY + PLAYER_HEIGHT > fireball->y && 
		playerY < fireball->y + FIREBALL_SIZE && 
		playerX + PLAYER_WIDTH > fireball->x && 
		playerX < fireball->x + FIREBALL_SIZE && 
		player->state == PLAYER_NORMAL &&
		fireball->state == FIREBALL_MOVING)
			KillPlayer(player, gameFrame);
		
		fireball->sprite = (gameFrame - fireball->spawnTime)/4 % 4 + ((fireball->state == FIREBALL_SPAWNING || fireball->state == FIREBALL_DESPAWNING) ? 4 : 0);
	}
}

void FreeFireballs(void) {
	free(levelFireballs.fireballArray);
}

void ManageFireballSpawning(player_t* player, unsigned int gameFrame, int16_t fireballFlags) {
	if (game_data.levelEnded || gameFrame - game_data.levelStartTime < 150)
		return;
	if (gameFrame % 4 == 0)
		--levelFireballs.fireballSpawnWeight;
	if (fireballFlags & HAS_FIREBALL_GREEN && rand() % levelFireballs.fireballSpawnWeight == 0) {
		uint8_t fy;
		// spawn green fireballs
		if (player->y < TO_FIXED_POINT(72))
			fy = 50;
		else if (player->y < TO_FIXED_POINT(120))
			fy = 107;
		else if (player->y < TO_FIXED_POINT(176))
			fy = 166;
		else
			fy = 200;
		
		CreateFireball(fy, (player->x > TO_FIXED_POINT(160)) ? LEFT : RIGHT, FIREBALL_GREEN, gameFrame);
		
		levelFireballs.fireballSpawnWeight = INITIAL_FIREBALL_SPAWN_WEIGHT;
	}
	
	if (fireballFlags & HAS_FIREBALL_RED && rand() % 30000 == 0) {
		CreateFireball(50 - (rand() % 20) + 10, RIGHT, FIREBALL_RED, gameFrame);
	}
}

void ResetFireballs(void) {
	for (uint8_t i = 0; i < levelFireballs.numFireballs; i++) {
		levelFireballs.fireballArray[i].alive = false;
	}
}