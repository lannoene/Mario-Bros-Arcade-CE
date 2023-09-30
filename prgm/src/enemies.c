#include "enemies.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <graphx.h>

#include "platforms.h"
#include "pipes.h"

levelEnemies_t levelEnemies;

static inline void EnterRespawnPipe(uint8_t enemyIndex, unsigned int gameFrame);

void InitEnemies(void) {
	memset(&levelEnemies, 0, sizeof(levelEnemies_t));
	
	levelEnemies.enemyArray = malloc(0);
}

void SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame) {
	uint8_t i;
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		if (levelEnemies.enemyArray[i].state == ENEMY_DEAD)
			break;
	}
	if (i == levelEnemies.numEnemies) {
		++levelEnemies.numEnemies;
		levelEnemies.enemyArray = realloc(levelEnemies.enemyArray, levelEnemies.numEnemies*sizeof(enemy_t));
	}
	memset(levelEnemies.enemyArray + i, 0, sizeof(enemy_t));
	levelEnemies.enemyArray[i].type = enemyType;
	levelEnemies.enemyArray[i].spawnTime = gameFrame;
	levelEnemies.enemyArray[i].state = ENEMY_EXITING_PIPE;
	if (direction == RIGHT) {
		levelEnemies.enemyArray[i].x = 30;
		levelEnemies.enemyArray[i].x_old = 30;
	} else {
		levelEnemies.enemyArray[i].x = 274;
		levelEnemies.enemyArray[i].x_old = 274;
	}
	levelEnemies.enemyArray[i].y = 35;
	levelEnemies.enemyArray[i].y_old = 35;
	levelEnemies.enemyArray[i].dir = direction;
	levelEnemies.enemyArray[i].backgroundData[0] = ENEMY_SPIKE_SIZE;
	levelEnemies.enemyArray[i].backgroundData[1] = ENEMY_SPIKE_SIZE;
	levelEnemies.enemyArray[i].verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
	levelEnemies.enemyArray[i].verSpriteOffset_old = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
	levelEnemies.enemyArray[i].crabIsMad = false;
	levelEnemies.enemyArray[i].maxSpeed = 0.5;
}

void FreeEnemies(void) {
	free(levelEnemies.enemyArray);
}

void UpdateEnemies(player_t* player, unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		if (levelEnemies.enemyArray[i].state == ENEMY_DEAD)
			continue;
		// apply gravity
		levelEnemies.enemyArray[i].verAccel -= GRAVITY;
		
		switch (levelEnemies.enemyArray[i].state) {
			case ENEMY_WALKING:
				if (levelEnemies.enemyArray[i].dir == LEFT && levelEnemies.enemyArray[i].grounded == true && levelEnemies.enemyArray[i].verAccel <= 0)
					levelEnemies.enemyArray[i].horAccel = -levelEnemies.enemyArray[i].maxSpeed;
				else if (levelEnemies.enemyArray[i].dir == RIGHT && levelEnemies.enemyArray[i].grounded == true && levelEnemies.enemyArray[i].verAccel <= 0)
					levelEnemies.enemyArray[i].horAccel = levelEnemies.enemyArray[i].maxSpeed;
				levelEnemies.enemyArray[i].sprite = ((gameFrame - levelEnemies.enemyArray[i].spawnTime)/8 % 3) + ((levelEnemies.enemyArray[i].crabIsMad) ? 4 : 0); // if the crab is mad, we want it to use the angry sprites
				
				if (player->x + PLAYER_WIDTH > levelEnemies.enemyArray[i].x && player->x < levelEnemies.enemyArray[i].x + ENEMY_SPIKE_SIZE && player->y + PLAYER_HEIGHT > levelEnemies.enemyArray[i].y && player->y < levelEnemies.enemyArray[i].y + ENEMY_SPIKE_HITBOX_HEIGHT && player->state == PLAYER_NORMAL) {
					KillPlayer(player, gameFrame);
				}
				break;
			case ENEMY_TURNING:
				
				break;
			case ENEMY_LAYING:
				if (gameFrame - levelEnemies.enemyArray[i].layStartTime > 400) {
					levelEnemies.enemyArray[i].state = ENEMY_WALKING;
					levelEnemies.enemyArray[i].verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
					levelEnemies.enemyArray[i].crabIsMad = false;
					levelEnemies.enemyArray[i].dir = (levelEnemies.enemyArray[i].dir + 1) % 2; // reverse direction
				}
				if (levelEnemies.enemyArray[i].grounded && levelEnemies.enemyArray[i].horAccel != 0)
					levelEnemies.enemyArray[i].horAccel = 0;
				
				if (player->state == PLAYER_NORMAL && player->x + PLAYER_WIDTH > levelEnemies.enemyArray[i].x && player->x < levelEnemies.enemyArray[i].x + ENEMY_SPIKE_SIZE && player->y + PLAYER_HEIGHT > levelEnemies.enemyArray[i].y && player->y < levelEnemies.enemyArray[i].y + ENEMY_SPIKE_HITBOX_HEIGHT) {
					levelEnemies.enemyArray[i].verAccel = 3;
					levelEnemies.enemyArray[i].state = ENEMY_DEAD_SPINNING;
					if (player->dir == RIGHT)
						levelEnemies.enemyArray[i].horAccel = 1.5;
					else
						levelEnemies.enemyArray[i].horAccel = -1.5;
					PlayerAddScore(player, 800);
					--levelEnemies.enemiesLeft;
				}
				break;
			case ENEMY_EXITING_PIPE:
				RedrawPipesWithNewSprite((levelEnemies.enemyArray[i].dir + 1) % 2, 0, gameFrame); // pipe is opposite their dir
				levelEnemies.enemyArray[i].verAccel = 0;
				if (levelEnemies.enemyArray[i].dir == RIGHT)
					levelEnemies.enemyArray[i].horAccel = 0.5;
				else
					levelEnemies.enemyArray[i].horAccel = -0.5;
				
				if (gameFrame - levelEnemies.enemyArray[i].spawnTime > 65)
					levelEnemies.enemyArray[i].state = ENEMY_WALKING;
				break;
		}
		
		if (levelEnemies.enemyArray[i].state != ENEMY_DEAD_SPINNING) {
			// ---- start physics ----
			if (levelEnemies.enemyArray[i].x < -ENEMY_SPIKE_SIZE) // enemy on sides off screen? if so, teleport them to other side
				levelEnemies.enemyArray[i].x = 320;
			else if (levelEnemies.enemyArray[i].x > 320)
				levelEnemies.enemyArray[i].x = -ENEMY_SPIKE_SIZE; // 0 - ENEMY_SPIKE_SIZE
			
			if (levelEnemies.enemyArray[i].grounded && levelEnemies.enemyArray[i].verAccel <= 0 && levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horAccel + ENEMY_SPIKE_SIZE > levelPlatforms.platformArray[levelEnemies.enemyArray[i].lastGroundedPlatformIndex].x && levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horAccel < levelPlatforms.platformArray[levelEnemies.enemyArray[i].lastGroundedPlatformIndex].x + levelPlatforms.platformArray[levelEnemies.enemyArray[i].lastGroundedPlatformIndex].width) { // is enemy still inside of last intersected platform's x footprint? if so, make sure they stay like so
				levelEnemies.enemyArray[i].verAccel = 0;
			} else if (levelEnemies.enemyArray[i].y - levelEnemies.enemyArray[i].verAccel > 224 - ENEMY_SPIKE_HITBOX_HEIGHT) { // is enemy on the bottom floor? if so, we can skip colision checking and make sure verAccel is 0
				levelEnemies.enemyArray[i].y = 224 - ENEMY_SPIKE_HITBOX_HEIGHT;
				levelEnemies.enemyArray[i].verAccel = 0;
				levelEnemies.enemyArray[i].grounded = true;
				// if enemy is on ground and is out of view, do the thing idk
				if ((levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horAccel >= 320 || levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horAccel <= -ENEMY_SPIKE_SIZE)) {
					EnterRespawnPipe(i, gameFrame);
				}
			} else { // otherwise, do expensive colision checking
				if (levelEnemies.enemyArray[i].verAccel < 0) { // only test for physics if the verAccel is less than 0
					uint8_t j = 0;
					for (; j < levelPlatforms.numPlatforms; j++) {
						if (levelEnemies.enemyArray[i].y - levelEnemies.enemyArray[i].verAccel + ENEMY_SPIKE_HITBOX_HEIGHT > levelPlatforms.platformArray[j].y && levelEnemies.enemyArray[i].y - levelEnemies.enemyArray[i].verAccel < levelPlatforms.platformArray[j].y + PLATFORM_HEIGHT && levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horAccel + ENEMY_SPIKE_SIZE > levelPlatforms.platformArray[j].x && levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horAccel < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
							levelEnemies.enemyArray[i].y = levelPlatforms.platformArray[j].y - ENEMY_SPIKE_HITBOX_HEIGHT;
							levelEnemies.enemyArray[i].verAccel = 0;
							levelEnemies.enemyArray[i].grounded = true;
							levelEnemies.enemyArray[i].lastGroundedPlatformIndex = j;
							break;
						}
					}
					if (j == levelPlatforms.numPlatforms && levelEnemies.enemyArray[i].grounded == true)
						levelEnemies.enemyArray[i].grounded = false;
				}
			}
			// ---- end physics ----
			
			
			//colision_t colidedPlatform = CheckColision(&tmpX, &levelEnemies.enemyArray[i].y, ENEMY_SPIKE_SIZE, ENEMY_SPIKE_HITBOX_HEIGHT, &levelEnemies.enemyArray[i].verAccel, &levelEnemies.enemyArray[i].horAccel, false);
			//levelEnemies.enemyArray[i].x = ((float)tmpX != floor(levelEnemies.enemyArray[i].x)) ? (float)tmpX : levelEnemies.enemyArray[i].x;
			
			/*if (colidedPlatform.hasColided) {
				levelEnemies.enemyArray[i].y = colidedPlatform.y - ENEMY_SPIKE_HITBOX_HEIGHT;
				levelEnemies.enemyArray[i].verAccel = 0;
			}
			levelEnemies.enemyArray[i].grounded = colidedPlatform.hasColided;*/
			
		} else {
			if (levelEnemies.enemyArray[i].y > 240) // wait for enemy to leave the screen
				levelEnemies.enemyArray[i].state = ENEMY_DEAD;
			
			if (levelEnemies.enemyArray[i].verAccel < -4) // cap fall speed
				levelEnemies.enemyArray[i].verAccel = -4;
		}
		
		levelEnemies.enemyArray[i].y -= levelEnemies.enemyArray[i].verAccel;
		levelEnemies.enemyArray[i].x += levelEnemies.enemyArray[i].horAccel;
	}
}

static inline void EnterRespawnPipe(uint8_t enemyIndex, unsigned int gameFrame) {
	levelEnemies.enemyArray[enemyIndex].y = 35;
	levelEnemies.enemyArray[enemyIndex].state = ENEMY_EXITING_PIPE;
	levelEnemies.enemyArray[enemyIndex].spawnTime = gameFrame;
	if (levelEnemies.enemyArray[enemyIndex].dir == RIGHT) {
		levelEnemies.enemyArray[enemyIndex].x = 274;
		levelEnemies.enemyArray[enemyIndex].dir = LEFT;
	} else {
		levelEnemies.enemyArray[enemyIndex].x = 30;
		levelEnemies.enemyArray[enemyIndex].dir = RIGHT;
	}
	
}