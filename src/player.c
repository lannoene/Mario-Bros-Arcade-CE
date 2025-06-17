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

#include "assembly.h"

#define FRAME_DELAY 3
#define PLAYER_ACCELERATION	0.2
#define PLAYER_DECELERATION	0.2
#define ENEMY_KILL_KICK_TIME 20

enum player_collision_type {
	CFROM_NONE = 0,
	CFROM_PLATFORM,
	CFROM_POW,
	CFROM_PLAYER,
	CFROM_GROUND,
};

void PlayerInit(player_t* player) {
	memset(player, 0, sizeof(*player));
	player->backgroundData[0] = PLAYER_WIDTH;
	player->backgroundData[1] = PLAYER_SPRITE_HEIGHT;
	player->y = I2FP(GROUND_HEIGHT - PLAYER_HEIGHT);
	player->x = I2FP(16);
	player->lives = 4;
	player->state = PLAYER_NORMAL;
	player->verSpriteOffset = 0;
	player->maxSpeed = I2FP(1.5);
	player->deceleration = I2FP(0.2);
	player->acceleration = I2FP(0.2);
	player->hasJumpedThisFrame = false;
	player->hasCollectedBonus = false;
	player->currentCombo = 1;
	player->respawnPlatformBgData[0] = 15;
	player->respawnPlatformBgData[1] = 7;
	player->lastJumpTime = 0;
	player->horAccelPassive = 0;
}

void PlayerMove(player_t* player, uint8_t direction, unsigned int gameFrame) {
	switch (direction) {
		case LEFT: {
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
		}
		case RIGHT: {
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
		}
		case UP: {
			#define GET_JUMP_ACCEL (player->doingFastJump ? I2FP(3.7) : I2FP(3.4))
			if (player->state != PLAYER_NORMAL)
				return;
			if (player->hasJumpedThisFrame && player->canExtendJump && !player->grounded && gameFrame - player->lastJumpTime < 12) {
				player->verAccel = GET_JUMP_ACCEL;
				player->verAccelPassive = GET_JUMP_ACCEL;
			} else if (player->grounded && !player->hasJumpedThisFrame) {
				player->doingFastJump = ABS(player->horAccel) > I2FP(0.8);
				player->verAccel = GET_JUMP_ACCEL;
				player->grounded = false;
				player->sprite = 4;
				player->verAccelPassive = GET_JUMP_ACCEL;
				player->deceleration = I2FP(0.2);
				player->acceleration = I2FP(0.2);
				player->hasJumpedThisFrame = true;
				player->lastJumpTime = gameFrame;
				player->canExtendJump = true;
			}
			#undef GET_JUMP_ACCEL
			break;
		}
		case NONE: {
			#define HOR_DECELRATE(x, by) \
			do {if (x < by && x > -(by)) {\
				x = 0;\
			} else if (x > 0) {\
				x -= by;\
			} else {\
				x += by;\
			}} while (0)
			
			if (player->grounded)
				HOR_DECELRATE(player->horAccel, player->deceleration);
			break;
		}
		case NOJUMP: {
			if (player->grounded)
				player->hasJumpedThisFrame = false;
			player->canExtendJump = false;
			break;
		}
	}
}

void UpdatePlayer(player_t* player, int gameFrame) {
	switch (player->state) {
		case PLAYER_NORMAL:
			player->verAccel -= I2FP(GRAVITY);
			player->verAccelPassive -= I2FP(GRAVITY);
			HOR_DECELRATE(player->horAccelPassive, I2FP(0.1));
			#undef HOR_DECELRATE
			FOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
				if (AABB(
					player->x, player->y, I2FP(PLAYER_WIDTH), I2FP(PLAYER_HEIGHT),
					enemy->x, enemy->y, enemy->width, enemy->height
				)) {
					switch (enemy->state) {
						case ENEMY_WALKING:
							if (enemy->type != ENEMY_COIN)
								KillPlayer(player, gameFrame);
							else
								CollectCoin(player, enemy);
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
				player->verAccel = I2FP(3);
			else if (gameFrame - player->deathTime > 10)
				player->verAccel -= I2FP(GRAVITY);
			player->horAccel = 0;
			if (player->y > I2FP(240) && gameFrame - player->deathTime > 150) {
				if (player->lives > 0) {
					player->state = PLAYER_RESPAWNING;
					player->y = I2FP(-PLAYER_HEIGHT);
					player->x = I2FP(32);
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
			player->grounded = false;
			if (gameFrame - player->spawnTime < PLAYER_RESP_FALL_DURATION) {
				//player->sprite = 7;
				if ((gameFrame - player->spawnTime) % 3 == 0)
					player->y += I2FP(1);
			} else if (gameFrame - player->spawnTime < PLAYER_RESP_FALL_DURATION + PLAYER_RESP_WAIT_MAX) {
				//player->sprite = 7;
			} else {
				GetRidOfRespawnPlatformRemnants(player);
				player->state = PLAYER_NORMAL;
			}
			break;
	}
	
	if (player->state == PLAYER_NORMAL) {
		// checking x for offscreen transition to other side
		if (player->x < I2FP(-PLAYER_WIDTH)) {
			player->x = I2FP(320 - 1); // if they go offscreen to the left, teleport them to the right side
			if (player->grounded)
				player->verAccel = 0;
		} else if (player->x > I2FP(320)) {
			player->x = I2FP(-PLAYER_WIDTH + 1); // if they go offscreen to the right, teleport them to the left side
			if (player->grounded)
				player->verAccel = 0;
		}
		
		// platform colision
		if (player->y - player->verAccel > I2FP(GROUND_HEIGHT - PLAYER_HEIGHT)) {
			player->y = I2FP(GROUND_HEIGHT - PLAYER_HEIGHT);
			player->verAccel = 0;
			player->verAccelPassive = 0;
			player->grounded = true;
		} else if (player->lastGroundedPlatform != NULL 
		&& player->grounded
		&& (((player->x + I2FP(PLAYER_WIDTH) + player->horAccel > player->lastGroundedPlatform->x 
		&& player->x + player->horAccel < player->lastGroundedPlatform->x + player->lastGroundedPlatform->width)
		|| !(player->x > 0 && player->x + I2FP(PLAYER_WIDTH) < I2FP(320))))) { // if were are within the platform's last footprint or we are on the ground, going to the other screen (we don't want to fall off before we reach the other side!)
		// yes, i know there's theoretically a bug where if you jumped at the correct pixel, you could falsify your grounded var and fall down, but that's too specific for me to care
			player->verAccel = 0;
			player->verAccelPassive = 0;
			player->grounded = true;
			if (player->lastGroundedPlatform->icy) {
				player->deceleration = I2FP(0.07);
				player->acceleration = I2FP(0.07);
			}
		} else {
			FOR_EACH(levelPlatforms.platformArray, levelPlatforms.numPlatforms, platform) {
				if (player->y - player->verAccel + I2FP(PLAYER_HEIGHT) > platform->y 
				&& player->y - player->verAccel < platform->y + I2FP(PLATFORM_HEIGHT) 
				&& player->x + I2FP(PLAYER_WIDTH) + player->horAccel > platform->x 
				&& player->x + player->horAccel < platform->x + platform->width) {
					if (player->verAccel < 0 && player->y + I2FP(PLAYER_HEIGHT) <= platform->y) {
						player->y = platform->y - I2FP(PLAYER_HEIGHT);
						player->verAccel = 0;
						player->verAccelPassive = 0;
						player->lastGroundedPlatform = platform;
						player->grounded = true;
						break;
					} else if (player->verAccel > 0 && player->y >= platform->y + I2FP(PLATFORM_HEIGHT)) {
						player->y = platform->y + I2FP(PLATFORM_HEIGHT);
						player->verAccel = 0;
						BumpPlatform(platform, player, gameFrame);
						player->canExtendJump = false;
						break;
					} else if (player->horAccel > 0 && player->x <= platform->x) {
						player->horAccel = 0;
						player->x = platform->x - I2FP(PLAYER_WIDTH);
					} else if (player->horAccel < 0 && player->x <= platform->x + platform->width) {
						player->horAccel = 0;
						player->x = platform->x + platform->width;
					}
				}
			} FOR_ELSE if (player->grounded) {
				player->deceleration = I2FP(0.2);
				player->acceleration = I2FP(0.2);
				player->grounded = false;
			}
		}
		
		// pow colision
		if (!player->grounded) { // check again
			for (uint8_t i = 0; i < levelPows.numPows; i++) {
				pow_t* pow = &levelPows.powArray[i];
				if (pow->state != POW_EMPTY 
				&& AABB(
					player->x + player->horAccel, player->y - player->verAccel, I2FP(PLAYER_WIDTH), I2FP(PLAYER_HEIGHT),
					pow->x, pow->y + I2FP(pow->state*2), I2FP(POW_SIZE), I2FP(POW_SIZE) - I2FP(pow->state*2))
				) {
					if (player->verAccel < 0 && player->y + I2FP(PLAYER_HEIGHT) <= pow->y + I2FP(pow->state*2)) {
						player->y = pow->y - I2FP(PLAYER_HEIGHT) + I2FP(pow->state*2);
						player->verAccel = 0;
						player->verAccelPassive = 0;
						player->lastGroundedPlatform = NULL;
						player->grounded = true;
						break;
					} else if (player->verAccel > 0 && player->y >= pow->y + I2FP(POW_SIZE) - I2FP(pow->state*2)) {
						player->y = pow->y + I2FP(POW_SIZE) - I2FP(pow->state*2);
						player->verAccel = 0;
						player->verAccelPassive = 0;
						BumpPow(player, i, gameFrame);
						player->canExtendJump = false;
						break;
					} else if (player->horAccel > 0 && player->x <= pow->x) {
						player->horAccel = 0;
						player->x = pow->x - I2FP(PLAYER_WIDTH);
					} else if (player->horAccel < 0 && player->x <= pow->x + I2FP(POW_SIZE)) {
						player->horAccel = 0;
						player->x = pow->x + I2FP(POW_SIZE);
					}
				}
			}
		}
	}
}

void PlayerCollideTop(player_t *player, player_t *oPlayer) {
	player->y = oPlayer->y - I2FP(PLAYER_HEIGHT);
	player->verAccel = oPlayer->verAccel;
	player->verAccelPassive = oPlayer->verAccelPassive;
	player->grounded = true;
	player->lastGroundedPlatform = NULL;
	player->horAccelPassive = oPlayer->horAccel;
}

void PlayerCollideBottom(player_t *player, player_t *oPlayer) {
	player->y = oPlayer->y + I2FP(PLAYER_HEIGHT);
	player->verAccel = oPlayer->verAccel;
	player->verAccelPassive = oPlayer->verAccelPassive;
	player->canExtendJump = false;
}

void PlayerCollideLeft(player_t *player, int x) {
	player->horAccel = 0;
	player->x = x - I2FP(PLAYER_WIDTH);
}

void PlayerCollideRight(player_t *player, int x, int w) {
	player->horAccel = 0;
	player->x = x + w;
}

bool PerformTopCol(player_t *p1, player_t *p2) {
	if ((p1->verAccel < p2->verAccel) && p1->y + I2FP(PLAYER_HEIGHT) <= p2->y) {
		player_t pCopy = *p1;
		PlayerCollideTop(p1, p2);
		PlayerCollideBottom(p2, &pCopy);
		return true;
	}
	return false;
}

bool PerformLeftCol(player_t *p1, player_t *p2) {
	if ((p1->horAccel > 0 || p2->horAccel < 0) && p1->x + I2FP(PLAYER_WIDTH) <= p2->x) {
		if (p1->horAccel > 0)
			PlayerCollideLeft(p1, p2->x);
		if (p2->horAccel < 0)
			PlayerCollideRight(p2, p1->x, I2FP(PLAYER_WIDTH));
		return true;
	}
	return false;
}

void UpdatePlayers(player_t *players, unsigned int gameFrame) {
	for (uint8_t i = 0; i < game_data.numPlayers; i++) {
		UpdatePlayer(&players[i], gameFrame);
	}
	
	// collide players
	for (uint8_t i = 0; i < game_data.numPlayers; i++) {
		player_t *p1 = &players[i];
		if (p1->state != PLAYER_NORMAL)
			continue;
		for (uint8_t j = i + 1; j < game_data.numPlayers; j++) {
			player_t *p2 = &players[j];
			if (p2->state != PLAYER_NORMAL)
				continue;
			
			unsigned int p1x = p1->x + p1->horAccel;
			unsigned int p1y = p1->y - p1->verAccel;
			unsigned int p2x = p2->x + p2->horAccel;
			unsigned int p2y = p2->y - p2->verAccel;
			if (AABB(
				p1x, p1y,
				I2FP(PLAYER_WIDTH),
				I2FP(PLAYER_HEIGHT),
				p2x, p2y,
				I2FP(PLAYER_WIDTH),
				I2FP(PLAYER_HEIGHT))
			) {
				// collision
				if (PerformLeftCol(p1, p2)) {}
				else if (PerformLeftCol(p2, p1)) {}
				else if (PerformTopCol(p1, p2)) {}
				else if (PerformTopCol(p2, p1)) {}
			}
		}
	}
	
	FOR_EACH(players, game_data.numPlayers, player) {
		if (player->state == PLAYER_NORMAL) {
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
			} else if (player->horAccel == 0 && player->grounded) {
				player->sprite = 0;
				player->verSpriteOffset = 0;
			}
			
			if (player->verAccelPassive > player->verAccel) // if you hit under the ground but your ver accel passive hasn't reached its peak yet, just wait at the bottom of the platform
				player->verAccel = 0;
		}
		player->y -= player->verAccel;
		player->x += player->horAccel + player->horAccelPassive;
	}
}

void KillPlayer(player_t* player, unsigned int gameFrame) {
	player->state = PLAYER_DEAD;
	player->verAccel = 0;
	player->verAccelPassive = 0;
	player->dir = RIGHT;
	player->sprite = 5;
	player->deathTime = gameFrame;
	player->y -= I2FP(PLAYER_SPRITE_HEIGHT - PLAYER_HEIGHT);
	player->acceleration = player->deceleration = I2FP(PLAYER_ACCELERATION);
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