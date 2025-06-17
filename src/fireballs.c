#include "fireballs.h"

#include <stdlib.h>
#include <math.h>
#include <graphx.h>
#include <sys/util.h>

#include "platforms.h"
#include "level.h"
#include "defines.h"

#define INITIAL_FIREBALL_SPAWN_WEIGHT 10000
#define FIREBALL_SPEED 1

levelFireballInfo_t levelFireballs;
fireball_t fireballArray[4];

void InitFireballs(void) {
	levelFireballs.numFireballs = ARR_LEN(fireballArray);
	levelFireballs.fireballArray = fireballArray;
	levelFireballs.fireballSpawnWeight = INITIAL_FIREBALL_SPAWN_WEIGHT;
	ResetFireballs();
}

void CreateFireball(uint8_t y, bool dir, uint8_t type, unsigned int gameFrame) {
	fireball_t *fireball = NULL;
	FOR_EACH(levelFireballs.fireballArray, levelFireballs.numFireballs, _fireball) {
		if (!_fireball->alive) {
			fireball = _fireball;
			break;
		}
	} FOR_ELSE {
		dbg_printf("Could not spawn fireball: the maximum number of fireballs has been reached");
		return;
	}
	
	if (type == FIREBALL_GREEN) {
		if (dir == LEFT)
			fireball->x = 314;
		else
			fireball->x = 0;
	} else {
		fireball->verDir = DOWN;
		fireball->x = 314;
	}
	fireball->alive = true;
	fireball->state = FIREBALL_SPAWNING;
	fireball->y = fireball->y_old = fireball->original_y = y;
	fireball->spawnTime = gameFrame;
	fireball->backgroundData[0] = FIREBALL_SIZE;
	fireball->backgroundData[1] = FIREBALL_SIZE;
	fireball->type = type;
	fireball->horDir = dir;
	
	levelFireballs.fireballSpawnWeight = INITIAL_FIREBALL_SPAWN_WEIGHT;
}

void UpdateFireballs(player_t* player, unsigned int gameFrame) {
	FOR_EACH(levelFireballs.fireballArray, levelFireballs.numFireballs, fireball) {
		if (!fireball->alive)
			continue;
		if (fireball->state == FIREBALL_DESPAWNING) {
			// fireball type agnostic despawning mechanism
			if (gameFrame - fireball->dieTime == 13) {
				fireball->y = 250;
			} else if (gameFrame - fireball->dieTime == 14) {
				fireball->alive = false;
			}
		} else if (fireball->type == FIREBALL_GREEN) {
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
				//float sinOfX = sin((float)fireball->x/10);
				//dbg_printf("%d,\n", (int)(-10*(sinOfX + sinOfX*sinOfX)));
				static int8_t sinYOffsetTbl[] = {
					-1,-2,-3,-5,-7,-8,-10,-12,-13,-15,-16,
					-18,-18,-19,-19,-19,-19,-19,-18,-17,-16,
					-14,-13,-11,-9,-7,-6,-4,-2,-1,0,0,1,1,2,
					2,2,2,2,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,2,
					2,2,2,2,2,1,0,0
				};
				fireball->y = fireball->original_y + sinYOffsetTbl[fireball->x % sizeof(sinYOffsetTbl)];
			}
		} else {
			if (gameFrame - fireball->spawnTime < 60) {
				
			} else if (gameFrame - fireball->spawnTime == 60) {
				fireball->state = FIREBALL_MOVING;
			}
			if (fireball->x <= 0)
				fireball->horDir = RIGHT;
			else if (fireball->x > 320 - FIREBALL_SIZE)
				fireball->horDir = LEFT;
			
			if (fireball->y <= 0)
				fireball->verDir = DOWN;
			else if (fireball->y > GROUND_HEIGHT - FIREBALL_SIZE)
				fireball->verDir = UP;
			
			FOR_EACH(levelPlatforms.platformArray, levelPlatforms.numPlatforms, platform) {
				uint16_t platformX = FIXED_POINT_TO_INT(platform->x);
				uint8_t platformY = FIXED_POINT_TO_INT(platform->y);
				uint16_t platformWidth = FIXED_POINT_TO_INT(platform->width);
				int8_t vSpeed = ((fireball->verDir == DOWN) ? FIREBALL_SPEED : -FIREBALL_SPEED);
				int8_t hSpeed = ((fireball->horDir == RIGHT) ? FIREBALL_SPEED : -FIREBALL_SPEED);
				if ((fireball->y + FIREBALL_SIZE + vSpeed) > platformY && 
				fireball->y + vSpeed < platformY + PLATFORM_HEIGHT && 
				fireball->x + FIREBALL_SIZE + hSpeed > platformX && 
				fireball->x + hSpeed < platformX + platformWidth) {
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
			
			if ((gameFrame & 0x1) == 0) { // avoid using floats
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
		int playerY = FIXED_POINT_TO_INT(player->y);
		int playerX = FIXED_POINT_TO_INT(player->x);
		if (AABB(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT,
		fireball->x, fireball->y, FIREBALL_SIZE, FIREBALL_SIZE)
		&& player->state == PLAYER_NORMAL
		&& fireball->state == FIREBALL_MOVING)
			KillPlayer(player, gameFrame);
		
		fireball->sprite = (gameFrame - fireball->spawnTime)/4 % 4 + ((fireball->state == FIREBALL_SPAWNING || fireball->state == FIREBALL_DESPAWNING) ? 4 : 0);
	}
}

void FreeFireballs(void) {
	//free(levelFireballs.fireballArray);
}

uint8_t NumFireballsSpawned(uint8_t flags) {
	if (!flags)
		flags = HAS_FIREBALL_GREEN | HAS_FIREBALL_RED;
	uint8_t count = 0;
	for (uint8_t i = 0; i < levelFireballs.numFireballs; i++) {
		fireball_t* fireball = &levelFireballs.fireballArray[i];
		if (fireball->alive) {
			if ((fireball->type == FIREBALL_RED && (flags & HAS_FIREBALL_RED))
				|| ((fireball->type == FIREBALL_GREEN) && (flags & HAS_FIREBALL_GREEN)))
				count++;
		}
	}
	return count;
}

void ManageFireballSpawning(player_t* player, unsigned int gameFrame, int16_t fireballFlags) {
	if (!game_data.levelStarted)
		return;
	if (fireballFlags & HAS_FIREBALL_GREEN && randInt(0, 1800) == 0 && NumFireballsSpawned(HAS_FIREBALL_GREEN) == 0) {
		uint8_t fy;
		// spawn green fireballs
		if (player->y < TO_FIXED_POINT(72))
			fy = 50;
		else if (player->y < TO_FIXED_POINT(120))
			fy = 100;
		else if (player->y < TO_FIXED_POINT(176))
			fy = 159;
		else
			fy = 210;
		
		CreateFireball(fy, (player->x > TO_FIXED_POINT(160)) ? LEFT : RIGHT, FIREBALL_GREEN, gameFrame);
	}
	
	if (fireballFlags & HAS_FIREBALL_RED && randInt(0, 4000) == 0 && NumFireballsSpawned(HAS_FIREBALL_RED) == 0) {
		CreateFireball(50 - (randInt(0, 20)) + 10, RIGHT, FIREBALL_RED, gameFrame);
	}
}

void ResetFireballs(void) {
	extern unsigned int gameFrame;
	FOR_EACH(levelFireballs.fireballArray, levelFireballs.numFireballs, fireball) {
		fireball->state = FIREBALL_MOVING; // force all fireballs to be killable
		KillFireball(fireball, gameFrame);
	}
}

void KillFireball(fireball_t *fireball, unsigned int gameFrame) {
	if (!fireball->alive || fireball->state != FIREBALL_MOVING)
		return;
	fireball->state = FIREBALL_DESPAWNING;
	fireball->dieTime = gameFrame;
}