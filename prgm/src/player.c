#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>

#include "platforms.h"
#include "pow.h"
#include "bonus.h"
#include "save.h"
#include "level.h"
#include "particles.h"
#include "defines.h"
#include "enemies.h"
#include "hud.h"

#define FRAME_DELAY 3
#define PLAYER_ACCELERATION	0.2
#define PLAYER_DECELERATION	0.2
#define ENEMY_KILL_KICK_TIME 20

void PlayerInit(player_t* player) {
	memset(player, 0, sizeof(*player));
	player->backgroundData[0] = PLAYER_WIDTH;
	player->backgroundData[1] = PLAYER_SPRITE_HEIGHT;
	player->y = TO_FIXED_POINT(GROUND_HEIGHT - PLAYER_HEIGHT);
	player->x = TO_FIXED_POINT(16);
	player->lives = 4;
	player->state = PLAYER_NORMAL;
	player->verSpriteOffset = 0;
	player->maxSpeed = TO_FIXED_POINT(1.5);
	player->deceleration = TO_FIXED_POINT(0.2);
	player->acceleration = TO_FIXED_POINT(0.2);
	player->hasJumpedThisFrame = false;
	player->hasCollectedBonus = false;
	player->currentCombo = 1;
}

void PlayerMove(player_t* player, uint8_t direction, unsigned int gameFrame) {
	switch (direction) {
		case LEFT:
			if (player->state == PLAYER_RESPAWNING) { // prevent movement while spawning
				if (gameFrame - player->spawnTime >= PLAYER_RESP_FALL_DURATION) {
					player->state = PLAYER_NORMAL;
					GetRidOfRespawnPlatformRemnants(player);
					player->grounded = false;
				} else
					break;
			}
			
			if (player->horAccel > -player->maxSpeed)
				player->horAccel -= player->acceleration;
			else
				player->horAccel = -player->maxSpeed;
			if (gameFrame - player->lastKilledEnemyTime >= ENEMY_KILL_KICK_TIME)
				player->dir = LEFT;
			break;
		case RIGHT:
			if (player->state == PLAYER_RESPAWNING) { // prevent movement while spawning
				if (gameFrame - player->spawnTime >= PLAYER_RESP_FALL_DURATION) {
					player->state = PLAYER_NORMAL;
					GetRidOfRespawnPlatformRemnants(player);
					player->grounded = false;
				} else
					break;
			}
			
			if (player->horAccel < player->maxSpeed)
				player->horAccel += player->acceleration;
			else
				player->horAccel = player->maxSpeed;
			if (gameFrame - player->lastKilledEnemyTime >= ENEMY_KILL_KICK_TIME)
				player->dir = RIGHT;
			break;
		case UP:
			if (!player->grounded || player->state == PLAYER_DEAD || player->hasJumpedThisFrame)
				return;
			if (player->grounded) {
				player->verAccel = TO_FIXED_POINT(5);
				player->grounded = false;
				player->sprite = 4;
				player->verAccelPassive = TO_FIXED_POINT(5);
				player->deceleration = TO_FIXED_POINT(0.2);
				player->acceleration = TO_FIXED_POINT(0.2);
				player->hasJumpedThisFrame = true;
			}
			break;
		case NONE:
			if (player->horAccel < player->deceleration && player->horAccel > -player->deceleration)
				player->horAccel = 0;
			
			if (player->horAccel > 0 && player->grounded)
				player->horAccel -= player->deceleration;
			else if (player->horAccel < 0 && player->grounded)
				player->horAccel += player->deceleration;
			break;
		case NOJUMP:
			if (player->grounded)
				player->hasJumpedThisFrame = false;
			break;
	}
}

void UpdatePlayer(player_t* player, int gameFrame) {
	
	switch (player->state) {
		case PLAYER_NORMAL:
			player->verAccel -= TO_FIXED_POINT(GRAVITY);
			player->verAccelPassive -= TO_FIXED_POINT(GRAVITY);
			for (enemy_t* enemy = levelEnemies.enemyArray; enemy != &levelEnemies.enemyArray[levelEnemies.numEnemies]; enemy++) {
				if (player->x + TO_FIXED_POINT(PLAYER_WIDTH) > enemy->x && 
				player->x < enemy->x + enemy->width && 
				player->y + TO_FIXED_POINT(PLAYER_HEIGHT) > enemy->y && 
				player->y < enemy->y + TO_FIXED_POINT(ENEMY_SPIKE_HITBOX_HEIGHT)) {
					switch (enemy->state) {
						case ENEMY_WALKING:
							KillPlayer(player, gameFrame);
							break;
						case ENEMY_LAYING:
							KillEnemy(enemy, player, gameFrame);
							break;
						default:
							
							break;
					}
				}
			}
			break;
		case PLAYER_DEAD:
			if (gameFrame - player->deathTime == 10)
				player->verAccel = TO_FIXED_POINT(3);
			else if (gameFrame - player->deathTime > 10)
				player->verAccel -= TO_FIXED_POINT(GRAVITY);
			player->horAccel = 0;
			if (player->y > TO_FIXED_POINT(240) && gameFrame - player->deathTime > 150) {
				if (player->lives > 0) {
					player->state = PLAYER_RESPAWNING;
					player->y = TO_FIXED_POINT(-PLAYER_HEIGHT);
					player->x = TO_FIXED_POINT(32);
					player->spawnTime = gameFrame;
					player->verSpriteOffset = 0;
					player->verAccel = 0;
					player->sprite = 7;
				} else {
					if (gameFrame - player->deathTime == 151) {
						save_t oldData = GetSaveData();
						unsigned int highScore;
						uint8_t highLevel;
						if (player->score > oldData.highScore)
							highScore = player->score;
						else
							highScore = oldData.highScore;
						
						if (game_data.level > oldData.highLevel)
							highLevel = game_data.level;
						else
							highLevel = oldData.highLevel;
						save_t saveData = {false, false, highScore, 0, highLevel, game_data.level, player->lives, "Lann"};
						SaveCurrentData(saveData);
					}
					gfx_PrintStringXY("Game Over", 100, 100);
					player->verAccel = 0;
				}
			}
			break;
		case PLAYER_RESPAWNING:
			if (gameFrame - player->spawnTime < PLAYER_RESP_FALL_DURATION) {
				//player->sprite = 7;
				if ((gameFrame - player->spawnTime) % 3 == 0)
					player->y += TO_FIXED_POINT(1);
			} else if (gameFrame - player->spawnTime < PLAYER_RESP_FALL_DURATION + PLAYER_RESP_WAIT_MAX) {
				//player->sprite = 7;
			} else {
				GetRidOfRespawnPlatformRemnants(player);
				player->state = PLAYER_NORMAL;
				player->grounded = false;
			}
			break;
	}
	
	if (player->state == PLAYER_NORMAL) {
		
		// checking x for offscreen transition to other side
		if (player->x < TO_FIXED_POINT(-PLAYER_WIDTH + 2)) {
			player->x = TO_FIXED_POINT(320 - 1); // if they go offscreen to the left, teleport them to the right side
			if (player->grounded)
				player->verAccel = 0;
		} else if (player->x > TO_FIXED_POINT(320 - 1)) {
			player->x = TO_FIXED_POINT(-PLAYER_WIDTH + 1); // if they go offscreen to the right, teleport them to the left side
			if (player->grounded)
				player->verAccel = 0;
		}
		
		// platform colision
		if (player->y - player->verAccel > TO_FIXED_POINT(GROUND_HEIGHT - PLAYER_HEIGHT)) {
			player->y = TO_FIXED_POINT(GROUND_HEIGHT - PLAYER_HEIGHT);
			player->verAccel = 0;
			player->verAccelPassive = 0;
			player->grounded = true;
		} else if (player->lastGroundedPlatformIndex != -1 
		&& player->grounded
		&& player->x + TO_FIXED_POINT(PLAYER_WIDTH) + player->horAccel > levelPlatforms.platformArray[player->lastGroundedPlatformIndex].x 
		&& player->x + player->horAccel < levelPlatforms.platformArray[player->lastGroundedPlatformIndex].x + levelPlatforms.platformArray[player->lastGroundedPlatformIndex].width) {
			player->verAccel = 0;
			player->verAccelPassive = 0;
			player->grounded = true;
			if (levelPlatforms.platformArray[player->lastGroundedPlatformIndex].icy) {
				player->deceleration = TO_FIXED_POINT(0.07);
				player->acceleration = TO_FIXED_POINT(0.07);
			}
		} else {
			uint8_t i;
			for (i = 0; i < levelPlatforms.numPlatforms; i++) {
				platform_t* platform = &levelPlatforms.platformArray[i];
				if (player->y - player->verAccel + TO_FIXED_POINT(PLAYER_HEIGHT) > platform->y 
				&& player->y - player->verAccel < platform->y + TO_FIXED_POINT(PLATFORM_HEIGHT) 
				&& player->x + TO_FIXED_POINT(PLAYER_WIDTH) + player->horAccel > platform->x 
				&& player->x + player->horAccel < platform->x + platform->width) {
					if (player->verAccel < 0 && player->y + TO_FIXED_POINT(PLAYER_HEIGHT) <= platform->y) {
						player->y = platform->y - TO_FIXED_POINT(PLAYER_HEIGHT);
						player->verAccel = 0;
						player->verAccelPassive = 0;
						player->lastGroundedPlatformIndex = i;
						player->grounded = true;
						break;
					} else if (player->verAccel > 0 && player->y >= platform->y + TO_FIXED_POINT(PLATFORM_HEIGHT)) {
						player->y = platform->y + TO_FIXED_POINT(PLATFORM_HEIGHT);
						player->verAccel = 0;
						BumpPlatform(player, i, gameFrame);
						break;
					} else if (player->horAccel > 0 && player->x <= platform->x) {
						player->horAccel = 0;
						player->x = platform->x - TO_FIXED_POINT(PLAYER_WIDTH);
					} else if (player->horAccel < 0 && player->x <= platform->x + platform->width) {
						player->horAccel = 0;
						player->x = platform->x + platform->width;
					}
				}
			}
			if (player->grounded && i == levelPlatforms.numPlatforms) {
				player->grounded = false;
				player->deceleration = TO_FIXED_POINT(0.2);
				player->acceleration = TO_FIXED_POINT(0.2);
			}
		}
		
		// pow colision
		if (!player->grounded) { // check again
			for (uint8_t i = 0; i < levelPows.numPows; i++) {
				pow_t* pow = &levelPows.powArray[i];
				if (pow->state != POW_EMPTY 
				&& player->y - player->verAccel + TO_FIXED_POINT(PLAYER_HEIGHT) > pow->y + TO_FIXED_POINT(pow->state*2) 
				&& player->y - player->verAccel < pow->y + TO_FIXED_POINT(POW_SIZE) - TO_FIXED_POINT(pow->state*2) 
				&& player->x + TO_FIXED_POINT(PLAYER_WIDTH) + player->horAccel > pow->x 
				&& player->x + player->horAccel < pow->x + TO_FIXED_POINT(POW_SIZE)) {
					if (player->verAccel < 0 && player->y + TO_FIXED_POINT(PLAYER_HEIGHT) <= pow->y + TO_FIXED_POINT(pow->state*2)) {
						player->y = pow->y - TO_FIXED_POINT(PLAYER_HEIGHT) + TO_FIXED_POINT(pow->state*2);
						player->verAccel = 0;
						player->verAccelPassive = 0;
						player->lastGroundedPlatformIndex = -1;
						player->grounded = true;
						break;
					} else if (player->verAccel > 0 && player->y >= pow->y + TO_FIXED_POINT(POW_SIZE) - TO_FIXED_POINT(pow->state*2)) {
						player->y = pow->y + TO_FIXED_POINT(POW_SIZE) - TO_FIXED_POINT(pow->state*2);
						player->verAccel = 0;
						player->verAccelPassive = 0;
						BumpPow(player, i, gameFrame);
						break;
					} else if (player->horAccel > 0 && player->x <= pow->x) {
						player->horAccel = 0;
						player->x = pow->x - TO_FIXED_POINT(PLAYER_WIDTH);
					} else if (player->horAccel < 0 && player->x <= pow->x + TO_FIXED_POINT(POW_SIZE)) {
						player->horAccel = 0;
						player->x = pow->x + TO_FIXED_POINT(POW_SIZE);
					}
				}
			}
		}
		
		if (!player->grounded)
			player->sprite = 4;
		else if ((player->horAccel < 0 && player->dir == RIGHT) || (player->horAccel > 0 && player->dir == LEFT)) { // he do be driftin
			player->sprite = 6;
			if (gameFrame % 5 == 0)
				SpawnParticle((player->dir) ? FIXED_POINT_TO_INT(player->x) + PLAYER_WIDTH : FIXED_POINT_TO_INT(player->x) - 7, FIXED_POINT_TO_INT(player->y) + 11, PARTICLE_DUST, gameFrame);
		} else if (gameFrame - player->lastKilledEnemyTime < ENEMY_KILL_KICK_TIME) {
			player->sprite = 2;
			player->verSpriteOffset = -1;
		} else if (player->horAccel != 0 && player->grounded) {
			if (gameFrame % FRAME_DELAY == 0) {
				switch (gameFrame/FRAME_DELAY % 3) { // walk cycle
					case 0:
						player->sprite = 0;
						player->verSpriteOffset = 0;
						break;
					case 1:
						player->sprite = 2;
						player->verSpriteOffset = -1;
						break;
					case 2:
						player->verSpriteOffset = 0;
						player->sprite = 1;
						break;
				}
			}
		} else if (player->horAccel == 0 && player->grounded) {
			player->sprite = 0;
			player->verSpriteOffset = 0;
		}
		
		if (player->verAccelPassive > player->verAccel) // if you hit under the ground but your ver accel passive hasn't reached its peak yet, just wait at the bottom of the platform
			player->verAccel = 0;
	}
	
	
	player->y -= player->verAccel;
	player->x += player->horAccel;
}

void KillPlayer(player_t* player, unsigned int gameFrame) {
	player->state = PLAYER_DEAD;
	player->verAccel = 0;
	player->dir = RIGHT;
	player->sprite = 5;
	player->deathTime = gameFrame;
	player->y -= TO_FIXED_POINT(PLAYER_SPRITE_HEIGHT - PLAYER_HEIGHT);
	player->acceleration = player->deceleration = TO_FIXED_POINT(PLAYER_ACCELERATION);
	--player->lives;
}

void PlayerAddScore(player_t* player, uint16_t addedNum) {
	player->score += addedNum;
	if (!player->hasCollectedBonus && player->score > 20000) {
		++player->lives;
		player->hasCollectedBonus = true;
	}
}

static int CalcCollisionPlayer(player_t* player) {
	
}