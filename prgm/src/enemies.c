#include "enemies.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <graphx.h>
#include <sys/util.h>

#include "platforms.h"
#include "pipes.h"
#include "bonus.h"
#include "particles.h"
#include "defines.h"
#include "level.h"

#define GRAVITY_WINGED 0.07
#define ENEMY_DEAD_DECELERATION .017

// i am not mentally well

levelEnemies_t levelEnemies;

static inline void EnterRespawnPipe(enemy_t* enemy, unsigned int gameFrame);
// lol
static void CalcForSpinies(unsigned int gameFrame, enemy_t* enemy);
static void CalcForFlies(unsigned int gameFrame, enemy_t* enemy);
static void CalcForFreezies(unsigned int gameFrame, enemy_t* enemy);
static void CalcForCoins(unsigned int gameFrame, enemy_t *coin);
// epic
static uint8_t CalcCollsion(enemy_t* enemy, unsigned int gameFrame);

void InitEnemies(void) {
	memset(&levelEnemies, 0, sizeof(levelEnemies_t));
	levelEnemies.enemyArray = malloc(0);
}

enemy_t *SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame) {
	uint8_t i;
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		if (levelEnemies.enemyArray[i].state == ENEMY_DEAD)
			break;
	}
	if (i == levelEnemies.numEnemies) {
		++levelEnemies.numEnemies;
		void *temp = realloc(levelEnemies.enemyArray, levelEnemies.numEnemies*sizeof(enemy_t));
		if (!temp) {
			levelEnemies.numEnemies--;
			dbg_printf("ERROR: COULD NOT ALLOCATE ENEMY\n");
			return NULL;
		}
		levelEnemies.enemyArray = temp;
	}
	memset(levelEnemies.enemyArray + i, 0, sizeof(enemy_t)); // remember, you don't need to set things to null/0 because this does it already
	enemy_t* enemy = &levelEnemies.enemyArray[i];
	enemy->type = enemyType;
	enemy->height = I2FP(ENEMY_SPIKE_HITBOX_HEIGHT);
	enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
	if (enemyType == ENEMY_FREEZIE) {
		enemy->horSpriteOffset = enemy->horSpriteOffset_old = (ENEMY_FREEZIE_WIDTH - ENEMY_SPIKE_SIZE); // freezies are smaller width wise, so we need to compensate for that, and the best way to do that without jank was this
		enemy->width = TO_FIXED_POINT(ENEMY_FREEZIE_WIDTH);
	} else if (enemyType == ENEMY_COIN) {
		enemy->width = I2FP(COIN_WIDTH);
		enemy->height = I2FP(COIN_HEIGHT);
		enemy->verSpriteOffset = 0;
	} else {
		enemy->horSpriteOffset = enemy->horSpriteOffset_old = 0;
		enemy->width = TO_FIXED_POINT(ENEMY_SPIKE_SIZE);
	}
	enemy->spawnTime = gameFrame;
	enemy->state = ENEMY_EXITING_PIPE;
	if (levelEnemies.lastSpawnedPipe == RIGHT) {
		enemy->x = TO_FIXED_POINT(30);
		enemy->x_old = TO_FIXED_POINT(30);
		enemy->dir = RIGHT;
		levelEnemies.lastSpawnedPipe = LEFT;
	} else {
		enemy->x = TO_FIXED_POINT(274);
		enemy->x_old = TO_FIXED_POINT(274);
		enemy->dir = LEFT;
		levelEnemies.lastSpawnedPipe = RIGHT;
	}
	enemy->y = TO_FIXED_POINT(35);
	enemy->y_old = TO_FIXED_POINT(35);
	enemy->backgroundData[0] = ENEMY_SPIKE_SIZE;
	enemy->backgroundData[1] = ENEMY_SPIKE_SIZE;
	if (enemyType == ENEMY_FLY)
		enemy->maxSpeed = TO_FIXED_POINT(0.4);
	else if (enemyType == ENEMY_FREEZIE)
		enemy->maxSpeed = TO_FIXED_POINT(0.55);
	else
		enemy->maxSpeed = TO_FIXED_POINT(0.5);
	enemy->verSpriteOffset_old = enemy->verSpriteOffset;
	return enemy;
}

void FreeEnemies(void) {
	free(levelEnemies.enemyArray);
}

void UpdateEnemies(unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		enemy_t* enemy = &levelEnemies.enemyArray[i];
		if (enemy->state == ENEMY_DEAD)
			continue;
		
		// run preliminary
		switch (enemy->state) {
			case ENEMY_EXITING_PIPE:
				enemy->verVel = 0;
				enemy->horVel = (enemy->dir == RIGHT) ? enemy->maxSpeed : -enemy->maxSpeed;
				RedrawPipesWithNewSprite(!enemy->dir, 0, gameFrame);
				if (enemy->dir == LEFT && FP2I(enemy->x + enemy->width) < 256) {
					enemy->state = ENEMY_WALKING;
				} else if (enemy->dir == RIGHT && FP2I(enemy->x) > 64) {
					enemy->state = ENEMY_WALKING;
				}
				break;
		}
		
		switch (enemy->type) {
			case ENEMY_SPIKE:
				// fallthrough
			case ENEMY_CRAB:
				CalcForSpinies(gameFrame, enemy); // the spiny func was made to calc for both spikes and crabs. this makes it slower, but i'm too lazy to change it
				break;
			case ENEMY_FLY:
				CalcForFlies(gameFrame, enemy);
				break;
			case ENEMY_FREEZIE:
				CalcForFreezies(gameFrame, enemy);
				break;
			case ENEMY_COIN:
				CalcForCoins(gameFrame, enemy);
				break;
		}
		for (enemy_t* oEnmy = enemy; oEnmy != &levelEnemies.enemyArray[levelEnemies.numEnemies]; oEnmy++) { // check for other enemy colision. (check every combination, not permutation)
			if (enemy != oEnmy && // i don't like this if statement.
			(enemy->lastBumpedEnemy != oEnmy || gameFrame - enemy->lastBumpedEnemyTime > 60) && // last bumped enemy isn't other enemy or delta last time > 60
			gfx_CheckRectangleHotspot(enemy->x + enemy->horVel, enemy->y - enemy->verVel, enemy->width, enemy->height,
			oEnmy->x + oEnmy->horVel, oEnmy->y - oEnmy->verVel, oEnmy->width, oEnmy->height) && // i thought this func would make it cleaner but it didn't really
			(enemy->state == ENEMY_WALKING || enemy->state == ENEMY_LAYING) &&
			(oEnmy->state == ENEMY_WALKING || oEnmy->state == ENEMY_LAYING)) {
				if (enemy->horVel != 0 && enemy->state != ENEMY_LAYING) { // if enemy1 is moving, switch their directions
					oEnmy->dir = enemy->dir;
					enemy->dir = !enemy->dir;
					enemy->lastBumpedEnemy = oEnmy;
					oEnmy->lastBumpedEnemy = enemy;
				} else if (oEnmy->state != ENEMY_LAYING) { // if enemy1 is not moving, make enemy 2 switch directions (inherit enemy1's opposite dir) so they don't intersect
					enemy->dir = oEnmy->dir;
					oEnmy->dir = !enemy->dir;
					enemy->lastBumpedEnemy = oEnmy;
					oEnmy->lastBumpedEnemy = enemy;
				}
				enemy->lastBumpedEnemyTime = gameFrame;
				
				// i hate this but i couldn't think of a better way
				// if the flies are flying into each other, set their horVel to 0
				if (enemy->type == ENEMY_FLY && 
				((enemy->dir == RIGHT && enemy->horVel < 0) || (enemy->dir == LEFT && enemy->horVel > 0)))
					enemy->horVel = 0;
				
				if (oEnmy->type == ENEMY_FLY && 
				((oEnmy->dir == RIGHT && oEnmy->horVel < 0) || (oEnmy->dir == LEFT && oEnmy->horVel > 0)))
					oEnmy->horVel = 0;
			}
		}
		
		enemy->y -= enemy->verVel;
		enemy->x += enemy->horVel;
	}
}

static inline void EnterRespawnPipe(enemy_t* enemy, unsigned int gameFrame) {
	if (enemy->type == ENEMY_COIN) {
		enemy->state = ENEMY_DEAD_SPINNING;
		return;
	}
	enemy->y = TO_FIXED_POINT(35);
	enemy->state = ENEMY_EXITING_PIPE;
	enemy->spawnTime = gameFrame;
	enemy->lastGroundedPlatformIndex = -1;
	enemy->grounded = false;
	if (enemy->dir == RIGHT) {
		enemy->x = TO_FIXED_POINT(274);
		enemy->dir = LEFT;
	} else {
		enemy->x = TO_FIXED_POINT(30);
		enemy->dir = RIGHT;
	}
	RedrawPipesWithNewSprite(!enemy->dir, 0, gameFrame); // make sure that the enemy does not flicker when exiting
}

static void CalcForSpinies(unsigned int gameFrame, enemy_t* enemy) {
	
	switch (enemy->state) {
		case ENEMY_WALKING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY);
			if (enemy->dir == LEFT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = -enemy->maxSpeed;
			else if (enemy->dir == RIGHT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = enemy->maxSpeed;
			enemy->sprite = ((gameFrame - enemy->spawnTime)/8 % 3) + ((enemy->crabIsMad) ? 4 : 0);
			break;
		case ENEMY_TURNING:

			break;
		case ENEMY_LAYING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY);
			if (gameFrame - enemy->layStartTime > 400) {
				enemy->state = ENEMY_WALKING;
				enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				enemy->crabIsMad = false;
				enemy->dir = !enemy->dir; // reverse direction
			}
			if (enemy->grounded && enemy->horVel != 0)
				enemy->horVel = 0;
			break;
		case ENEMY_EXITING_PIPE:
			break;
		case ENEMY_DEAD_SPINNING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY);
			break;
	}
	
	if (enemy->state != ENEMY_DEAD_SPINNING) {
		CalcCollsion(enemy, gameFrame);
	} else {
		if (enemy->x < TO_FIXED_POINT(-ENEMY_SPIKE_SIZE)) // enemy on sides off screen? if so, teleport them to other side
			enemy->x = TO_FIXED_POINT(GFX_LCD_WIDTH);
		else if (enemy->x > TO_FIXED_POINT(GFX_LCD_WIDTH))
			enemy->x = TO_FIXED_POINT(-ENEMY_SPIKE_SIZE); // 0 - ENEMY_SPIKE_SIZE
		
		if (enemy->y > TO_FIXED_POINT(GFX_LCD_HEIGHT)) // wait for enemy to leave the screen
			enemy->state = ENEMY_DEAD;
		
		if (enemy->horVel > -TO_FIXED_POINT(ENEMY_DEAD_DECELERATION) && enemy->horVel < TO_FIXED_POINT(ENEMY_DEAD_DECELERATION))
			enemy->horVel = 0;
		else if (enemy->horVel > 0)
			enemy->horVel -= TO_FIXED_POINT(ENEMY_DEAD_DECELERATION);
		else if (enemy->horVel < 0)
			enemy->horVel += TO_FIXED_POINT(ENEMY_DEAD_DECELERATION);
		
		if (enemy->verVel < TO_FIXED_POINT(-4)) // cap fall speed
			enemy->verVel = TO_FIXED_POINT(-4);
	}
}

static void CalcForFlies(unsigned int gameFrame, enemy_t* enemy) {
	switch (enemy->state) {
		case ENEMY_WALKING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY_WINGED);
			enemy->sprite = ((gameFrame - enemy->spawnTime)/8 % 2) + 1;
			
			if (enemy->grounded) {
				enemy->horVel = 0;
				enemy->sprite = 0;
				if (gameFrame - enemy->groundedStartTime > 40) {
					enemy->verVel = TO_FIXED_POINT(1.5);
					enemy->horVel = (enemy->dir == LEFT) ? -enemy->maxSpeed : enemy->maxSpeed;
					enemy->grounded = false;
					if (enemy->dir == LEFT)
						enemy->horVel = -enemy->maxSpeed;
					else
						enemy->horVel = enemy->maxSpeed;
				}
			}
			break;
		case ENEMY_TURNING:
			
			break;
		case ENEMY_LAYING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY_WINGED);
			if (gameFrame - enemy->layStartTime > 400) {
				enemy->state = ENEMY_WALKING;
				enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				enemy->dir = !enemy->dir; // reverse direction
			}
			if (enemy->grounded && enemy->horVel != 0)
				enemy->horVel = 0;
			break;
		case ENEMY_EXITING_PIPE:
			enemy->sprite = 0;
			break;
		case ENEMY_DEAD_SPINNING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY_WINGED);
			break;
	}
	
	if (enemy->state != ENEMY_DEAD_SPINNING) {
		bool oldEnemyGrounded = enemy->grounded;
		if (CalcCollsion(enemy, gameFrame)) { // != 0
			if (!oldEnemyGrounded && enemy->grounded)
				enemy->groundedStartTime = gameFrame;
		}
	} else {
		if (enemy->x < TO_FIXED_POINT(-ENEMY_SPIKE_SIZE)) // enemy on sides off screen? if so, teleport them to other side
			enemy->x = TO_FIXED_POINT(GFX_LCD_WIDTH);
		else if (enemy->x > TO_FIXED_POINT(GFX_LCD_WIDTH))
			enemy->x = TO_FIXED_POINT(-ENEMY_SPIKE_SIZE); // 0 - ENEMY_SPIKE_SIZE
		
		if (enemy->horVel > -TO_FIXED_POINT(ENEMY_DEAD_DECELERATION) && enemy->horVel < TO_FIXED_POINT(ENEMY_DEAD_DECELERATION))
			enemy->horVel = 0;
		else if (enemy->horVel > 0)
			enemy->horVel -= TO_FIXED_POINT(ENEMY_DEAD_DECELERATION);
		else if (enemy->horVel < 0)
			enemy->horVel += TO_FIXED_POINT(ENEMY_DEAD_DECELERATION);
		
		if (enemy->y > TO_FIXED_POINT(GFX_LCD_HEIGHT)) // wait for enemy to leave the screen
			enemy->state = ENEMY_DEAD;
		
		if (enemy->verVel < TO_FIXED_POINT(-4)) // cap fall speed
			enemy->verVel = TO_FIXED_POINT(-4);
	}

}

static void CalcForFreezies(unsigned int gameFrame, enemy_t* enemy) {
	switch (enemy->state) {
		case ENEMY_WALKING:
			enemy->verVel -= TO_FIXED_POINT(GRAVITY);
			if (enemy->dir == LEFT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = -enemy->maxSpeed;
			else if (enemy->dir == RIGHT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = enemy->maxSpeed;
			enemy->sprite = ((gameFrame - enemy->spawnTime)/4 % 3);
			break;
		case ENEMY_TURNING:
			
			break;
		case ENEMY_DEAD_SPINNING: // enemy_dead_spinning for a freezie is just it dying
			enemy->verVel -= TO_FIXED_POINT(GRAVITY);
			enemy->sprite = ((gameFrame - enemy->eventTime)/4 % 5) + 3;
			if (gameFrame - enemy->eventTime > 6) {
				enemy->verVel = 0;
				if (gameFrame - enemy->eventTime >= 20) {
					enemy->state = ENEMY_DEAD;
					enemy->y = TO_FIXED_POINT(250);
				}
			}
			break;
		case FREEZIE_FREEZING_PLATFORM:
			enemy->verVel = 0;
			enemy->sprite = ((gameFrame - enemy->eventTime)/4 % 5) + 3;
			if (gameFrame - enemy->eventTime >= 20) { // actually kill the enemy
				enemy->state = ENEMY_DEAD;
				enemy->y = TO_FIXED_POINT(250);
			}
			break;
	}
	
	if (enemy->state != ENEMY_DEAD_SPINNING &&
	enemy->state != FREEZIE_FREEZING_PLATFORM &&
	enemy->state != ENEMY_DEAD) {
		switch (CalcCollsion(enemy, gameFrame)) {
			case EXP_COL: // expesnive col only gets run when the enemy is off of the platform
				enemy->freezieFreezeNextPlatform = randInt(0, 2) == 0 && !levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].icy;
				dbg_printf("rolled the dice: %d\n", enemy->freezieFreezeNextPlatform);
				break;
			case CHP_COL:
				if (enemy->freezieFreezeNextPlatform && 
				enemy->x + enemy->width > levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].x + (levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].width/2)
				&& enemy->x < levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].x + (levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].width/2)) {
					FreezePlatform(enemy->lastGroundedPlatformIndex);
					enemy->state = FREEZIE_FREEZING_PLATFORM;
					enemy->eventTime = gameFrame;
					enemy->horVel = 0;
					
				}
				break;
			
		}
	}
}

void ResetEnemies(unsigned int gameFrame) {
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		if (levelEnemies.enemyArray[i].state != ENEMY_DEAD) { // this should honestly only be useful for freezies
			if (levelEnemies.enemyArray[i].type == ENEMY_FREEZIE) {
				if (levelEnemies.enemyArray[i].state != ENEMY_EXITING_PIPE) {
					levelEnemies.enemyArray[i].state = FREEZIE_FREEZING_PLATFORM; // freezie freezing platform is just the normal death animation but it sets the ver accel to 0 automatically
					levelEnemies.enemyArray[i].eventTime = gameFrame;
					levelEnemies.enemyArray[i].horVel = 0;
				} else {
					levelEnemies.enemyArray[i].state = ENEMY_DEAD;
					levelEnemies.enemyArray[i].y = TO_FIXED_POINT(250);
					levelEnemies.enemyArray[i].verVel = 0;
				}
			} else {
				levelEnemies.enemyArray[i].state = ENEMY_DEAD_SPINNING;
			}
		}
	}
}

void EnemyShowScore(enemy_t* enemy, player_t* player, unsigned int gameFrame) {
	if (gameFrame - player->lastKilledTime > 40) {
		player->currentCombo = 1;
	} else {
		++player->currentCombo;
	}
	switch (player->currentCombo) {
		case 1:
			SpawnParticle(FIXED_POINT_TO_INT(enemy->x), FIXED_POINT_TO_INT(enemy->y), PARTICLE_SCORE_REG, gameFrame);
			PlayerAddScore(player, 800);
			break;
		case 2:
			SpawnParticle(FIXED_POINT_TO_INT(enemy->x), FIXED_POINT_TO_INT(enemy->y), PARTICLE_SCORE_DUB, gameFrame);
			PlayerAddScore(player, 1600);
			break;
		case 3:
			SpawnParticle(FIXED_POINT_TO_INT(enemy->x), FIXED_POINT_TO_INT(enemy->y), PARTICLE_SCORE_TRP, gameFrame);
			PlayerAddScore(player, 3200);
			break;
		case 4:
			SpawnParticle(FIXED_POINT_TO_INT(enemy->x), FIXED_POINT_TO_INT(enemy->y), PARTICLE_SCORE_QDP, gameFrame);
			PlayerAddScore(player, 3200);
			break;
		default: // 1 up
			SpawnParticle(FIXED_POINT_TO_INT(enemy->x), FIXED_POINT_TO_INT(enemy->y), PARTICLE_SCORE_1UP, gameFrame);
			PlayerAddScore(player, 3200);
			break;
	}
	
	player->lastKilledTime = gameFrame;
}

static uint8_t CalcCollsion(enemy_t* enemy, unsigned int gameFrame) {
	if (enemy->x < -enemy->width) // enemy on sides off screen? if so, teleport them to other side
		enemy->x = TO_FIXED_POINT(GFX_LCD_WIDTH);
	else if (enemy->x > TO_FIXED_POINT(GFX_LCD_WIDTH))
		enemy->x = -enemy->width; // 0 - enemy->width
	
	// if enemy is on ground and is out of view, do the thing idk
	if ((enemy->x + enemy->horVel >= TO_FIXED_POINT(GFX_LCD_WIDTH) || enemy->x + enemy->horVel <= -enemy->width) && enemy->y > TO_FIXED_POINT(176 + PLATFORM_HEIGHT)) { // 176 = bottom platforms' height. i dont wanna make macro though
		EnterRespawnPipe(enemy, gameFrame);
	}
	
	if (enemy->grounded 
	&& enemy->verVel <= 0 
	&& enemy->x + enemy->horVel + enemy->width > levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].x 
	&& enemy->x + enemy->horVel < levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].x + levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].width) { // is enemy still inside of last intersected platform's x footprint? if so, make sure they stay like so
		enemy->verVel = 0;
		return CHP_COL;
	} else if (enemy->y - enemy->verVel > TO_FIXED_POINT(GROUND_HEIGHT - ENEMY_SPIKE_HITBOX_HEIGHT)) { // is enemy on the bottom floor? if so, we can skip colision checking and make sure verVel is 0
		enemy->y = TO_FIXED_POINT(GROUND_HEIGHT - ENEMY_SPIKE_HITBOX_HEIGHT);
		enemy->verVel = 0;
		enemy->grounded = true;
		return BTM_COL;
	} else { // otherwise, do expensive colision checking
		if (enemy->verVel < 0) { // only test for physics if the verVel is less than 0
			uint8_t j = 0;
			for (; j < levelPlatforms.numPlatforms; j++) {
				if (enemy->y - enemy->verVel + enemy->height > levelPlatforms.platformArray[j].y 
				&& enemy->y - enemy->verVel < levelPlatforms.platformArray[j].y + TO_FIXED_POINT(PLATFORM_HEIGHT) 
				&& enemy->x + enemy->horVel + enemy->width > levelPlatforms.platformArray[j].x 
				&& enemy->x + enemy->horVel < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
					enemy->y = levelPlatforms.platformArray[j].y - enemy->height;
					enemy->verVel = 0;
					enemy->grounded = true;
					enemy->lastGroundedPlatformIndex = j;
					enemy->groundedStartTime = gameFrame;
					return EXP_COL;
				}
			}
			if (j == levelPlatforms.numPlatforms && enemy->grounded == true) {
				enemy->grounded = false;
				return NO_COL;
			}
		}
	}
	return 255;
}

// this is for all enemies except freezies and coins
void KillEnemy(enemy_t* enemy, player_t* player, unsigned int gameFrame) {
	enemy->verVel = (enemy->type != ENEMY_FLY) ? I2FP(3) : I2FP(2);
	enemy->state = ENEMY_DEAD_SPINNING;
	if (player->dir == RIGHT)
		enemy->horVel = I2FP(1.5);
	else
		enemy->horVel = I2FP(-1.5);
	enemy->grounded = false;
	
	--levelEnemies.enemiesLeft;
	SpawnEnemy(ENEMY_COIN, enemy->dir, gameFrame);
	
	EnemyShowScore(enemy, player, gameFrame);
	player->lastKilledEnemyTime = gameFrame;
}

static void CalcForCoins(unsigned int gameFrame, enemy_t *coin) {
	coin->sprite = ((gameFrame - coin->spawnTime)/4) % 5;
	
	if (coin->bonus && coin->state != ENEMY_DEAD_SPINNING)
		return;
	
	switch (coin->state) {
		case ENEMY_WALKING:
			coin->verVel -= TO_FIXED_POINT(GRAVITY);
			if (coin->dir == RIGHT) {
				coin->horVel = I2FP(0.5);
			} else {
				coin->horVel = I2FP(-0.5);
			}
			break;
		case ENEMY_EXITING_PIPE:
			// delay coin exit. i didn't like how quickly they came out after the player killed enemies
			if (gameFrame - coin->spawnTime < 70) {
				coin->horVel = 0;
			} else {
				if (coin->dir == LEFT)
					coin->horVel = -coin->maxSpeed;
				else
					coin->horVel = coin->maxSpeed;
			}
			break;
		case ENEMY_DEAD_SPINNING:
			if (coin->bonus)
				--levelCoins.coinsLeft;
			if (!game_data.levelEnded)
				SpawnParticle(FP2I(coin->x), FP2I(coin->y), PARTICLE_COIN_PICK, gameFrame);
			coin->y = I2FP(241);
			coin->state = ENEMY_DEAD;
			return;
	}
	
	if (coin->x < -COIN_WIDTH_FP)
		coin->x = I2FP(320);
	else if (coin->x > I2FP(320))
		coin->x = -COIN_WIDTH_FP;
	
	CalcCollsion(coin, gameFrame);
}