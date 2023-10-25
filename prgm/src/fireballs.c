#include "fireballs.h"

#include <stdlib.h>
#include <math.h>

#include "platforms.h"
#include "level.h"

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
	levelFireballs.fireballArray[i].y = levelFireballs.fireballArray[i].y_old = levelFireballs.fireballArray[i].original_y = y;
	levelFireballs.fireballArray[i].spawnTime = gameFrame;
	levelFireballs.fireballArray[i].backgroundData[0] = FIREBALL_SIZE;
	levelFireballs.fireballArray[i].backgroundData[1] = FIREBALL_SIZE;
	levelFireballs.fireballArray[i].type = type;
	levelFireballs.fireballArray[i].horDir = dir;
	
	levelFireballs.fireballSpawnWeight = INITIAL_FIREBALL_SPAWN_WEIGHT;
}

void UpdateFireballs(player_t* player, unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelFireballs.numFireballs; i++) {
		if (!levelFireballs.fireballArray[i].alive || gameFrame - game_data.levelStartTime < 150)
			continue;
		if (levelFireballs.fireballArray[i].type == FIREBALL_GREEN) {
			if (levelFireballs.fireballArray[i].horDir == LEFT) {
				--levelFireballs.fireballArray[i].x;
				if (levelFireballs.fireballArray[i].x < -FIREBALL_SIZE)
					levelFireballs.fireballArray[i].alive = false;
			} else {
				if (levelFireballs.fireballArray[i].x > 320)
					levelFireballs.fireballArray[i].alive = false;
				++levelFireballs.fireballArray[i].x;
			}
			float sinOfY = sin((float)levelFireballs.fireballArray[i].x/10);
			levelFireballs.fireballArray[i].y = levelFireballs.fireballArray[i].original_y + -13*(sinOfY + sinOfY*sinOfY);
		} else {
			if (levelFireballs.fireballArray[i].x <= 0)
				levelFireballs.fireballArray[i].horDir = RIGHT;
			else if (levelFireballs.fireballArray[i].x > 320 - FIREBALL_SIZE)
				levelFireballs.fireballArray[i].horDir = LEFT;
			
			if (levelFireballs.fireballArray[i].y <= 0)
				levelFireballs.fireballArray[i].verDir = DOWN;
			else if (levelFireballs.fireballArray[i].y > 224 - FIREBALL_SIZE)
				levelFireballs.fireballArray[i].verDir = UP;
			
			uint8_t j = 0;
			for (; j < levelPlatforms.numPlatforms; j++) {
				if (levelFireballs.fireballArray[i].y + FIREBALL_SIZE + ((levelFireballs.fireballArray[i].verDir == DOWN) ? FIREBALL_SPEED : -FIREBALL_SPEED) > levelPlatforms.platformArray[j].y && levelFireballs.fireballArray[i].y + ((levelFireballs.fireballArray[i].verDir == DOWN) ? FIREBALL_SPEED : -FIREBALL_SPEED) < levelPlatforms.platformArray[j].y + PLATFORM_HEIGHT && levelFireballs.fireballArray[i].x + FIREBALL_SIZE + ((levelFireballs.fireballArray[i].horDir == RIGHT) ? FIREBALL_SPEED : -FIREBALL_SPEED) > levelPlatforms.platformArray[j].x && levelFireballs.fireballArray[i].x + ((levelFireballs.fireballArray[i].horDir == RIGHT) ? FIREBALL_SPEED : -FIREBALL_SPEED) < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
					if (levelFireballs.fireballArray[i].verDir == UP && levelFireballs.fireballArray[i].y >= levelPlatforms.platformArray[j].y + PLATFORM_HEIGHT)
						levelFireballs.fireballArray[i].verDir = DOWN;
					else if (levelFireballs.fireballArray[i].verDir == DOWN && levelFireballs.fireballArray[i].y + FIREBALL_SIZE <= levelPlatforms.platformArray[j].y)
						levelFireballs.fireballArray[i].verDir = UP;
					else if (levelFireballs.fireballArray[i].horDir == LEFT && levelFireballs.fireballArray[i].x >= levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width)
						levelFireballs.fireballArray[i].horDir = RIGHT;
					else if (levelFireballs.fireballArray[i].horDir == RIGHT && levelFireballs.fireballArray[i].x + FIREBALL_SIZE <= levelPlatforms.platformArray[j].x)
						levelFireballs.fireballArray[i].horDir = LEFT;
					break;
				}
			}
			
			if (levelFireballs.fireballArray[i].horDir == RIGHT)
				levelFireballs.fireballArray[i].x += 0.5;
			else
				levelFireballs.fireballArray[i].x -= 0.5;
			
			if (levelFireballs.fireballArray[i].verDir == UP)
				levelFireballs.fireballArray[i].y -= 0.5;
			else
				levelFireballs.fireballArray[i].y += 0.5;
		}
		
		if (player->y + PLAYER_HEIGHT > levelFireballs.fireballArray[i].y && player->y < levelFireballs.fireballArray[i].y + FIREBALL_SIZE && player->x + PLAYER_WIDTH > levelFireballs.fireballArray[i].x && player->x < levelFireballs.fireballArray[i].x + FIREBALL_SIZE && player->state == PLAYER_NORMAL)
			KillPlayer(player, gameFrame);
		
		levelFireballs.fireballArray[i].sprite = (gameFrame - levelFireballs.fireballArray[i].spawnTime)/4 % 4;
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
		// spawn green fireballs
		if (player->y < 72)
			CreateFireball(50, (player->x > 160) ? LEFT : RIGHT, FIREBALL_GREEN, gameFrame);
		else if (player->y < 120)
			CreateFireball(107, (player->x > 160) ? LEFT : RIGHT, FIREBALL_GREEN, gameFrame);
		else if (player->y < 176)
			CreateFireball(166, (player->x > 160) ? LEFT : RIGHT, FIREBALL_GREEN, gameFrame);
		else
			CreateFireball(200, (player->x > 160) ? LEFT : RIGHT, FIREBALL_GREEN, gameFrame);
		
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