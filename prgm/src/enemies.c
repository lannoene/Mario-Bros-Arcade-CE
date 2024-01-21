#include "enemies.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <graphx.h>

#include "platforms.h"
#include "pipes.h"
#include "bonus.h"
#include "particles.h"
#include "defines.h"

#define GRAVITY_WINGED 0.07
#define ENEMY_DEAD_DECELERATION .017

// i am not mentally well

enum COLLISION_TYPE_IDS {
	NO_COL = 0,
	EXP_COL, // expensive collision
	CHP_COL, // cheap collision
	BTM_COL // bottom collision (collision with the bottom barrier)
};

levelEnemies_t levelEnemies;

static inline void EnterRespawnPipe(enemy_t* enemy, unsigned int gameFrame);
// lol
static void CalcForSpinies(player_t* player, unsigned int gameFrame, enemy_t* enemy);
static void CalcForFlies(player_t* player, unsigned int gameFrame, enemy_t* enemy);
static void CalcForFreezies(player_t* player, unsigned int gameFrame, enemy_t* enemy);
// epic
static uint8_t CalcCollsion(enemy_t* enemy, unsigned int gameFrame);

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
	enemy_t* enemy = &levelEnemies.enemyArray[i];
	enemy->type = enemyType;
	if (enemyType == ENEMY_FREEZIE) {
		enemy->horSpriteOffset = enemy->horSpriteOffset_old = (ENEMY_FREEZIE_WIDTH - ENEMY_SPIKE_SIZE); // freezies are smaller width wise, so we need to compensate for that, and the best way to do that without jank was this
		enemy->width = TO_FIXED_POINT(ENEMY_FREEZIE_WIDTH);
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
	enemy->lastBumpedEnemy = NULL;
	enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
	enemy->verSpriteOffset_old = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
	enemy->crabIsMad = false;
	enemy->freezieFreezeNextPlatform = false;
	if (enemyType == ENEMY_FLY)
		enemy->maxSpeed = TO_FIXED_POINT(0.4);
	else if (enemyType == ENEMY_FREEZIE)
		enemy->maxSpeed = TO_FIXED_POINT(0.75);
	else
		enemy->maxSpeed = TO_FIXED_POINT(0.5);
	enemy->maxSpeed = (enemyType == ENEMY_FLY) ? TO_FIXED_POINT(0.4) : TO_FIXED_POINT(0.5);
}

void FreeEnemies(void) {
	free(levelEnemies.enemyArray);
}

void UpdateEnemies(player_t* player, unsigned int gameFrame) {
	/*for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		enemy_t* enemy = &levelEnemies.enemyArray[i];*/
	for (enemy_t* enemy = &levelEnemies.enemyArray[0]; enemy != &levelEnemies.enemyArray[levelEnemies.numEnemies]; enemy++) {
		if (enemy->state == ENEMY_DEAD)
			continue;
		
		switch (enemy->type) {
			case ENEMY_SPIKE:
				// fallthrough
			case ENEMY_CRAB:
				CalcForSpinies(player, gameFrame, enemy); // the spiny func was made to calc for both spikes and crabs. this makes it slower, but i'm too lazy to change it
				break;
			case ENEMY_FLY:
				CalcForFlies(player, gameFrame, enemy);
				break;
			case ENEMY_FREEZIE:
				CalcForFreezies(player, gameFrame, enemy);
				break;
		}
		for (enemy_t* oEnmy = enemy; oEnmy != &levelEnemies.enemyArray[levelEnemies.numEnemies]; oEnmy++) { // check for other enemy colision. (check every combination, not permutation)
			if (enemy != oEnmy && // i don't like this if statement.
			(enemy->lastBumpedEnemy != oEnmy || gameFrame - enemy->lastBumpedEnemyTime > 60) && // last bumped enemy isn't other enemy or delta last time > 60
			gfx_CheckRectangleHotspot(enemy->x + enemy->horVel, enemy->y - enemy->verVel, enemy->width, TO_FIXED_POINT(ENEMY_SPIKE_HITBOX_HEIGHT),
			oEnmy->x + oEnmy->horVel, oEnmy->y - oEnmy->verVel, enemy->width, TO_FIXED_POINT(ENEMY_SPIKE_HITBOX_HEIGHT)) && // i thought this func would make it cleaner but it didn't really
			(enemy->state == ENEMY_WALKING || enemy->state == ENEMY_LAYING) &&
			(oEnmy->state == ENEMY_WALKING || oEnmy->state == ENEMY_LAYING)) {
				if (enemy->horVel != 0) { // if enemy1 is moving, switch their directions
					oEnmy->dir = enemy->dir;
					enemy->dir = !enemy->dir;
					enemy->lastBumpedEnemy = oEnmy;
					oEnmy->lastBumpedEnemy = enemy;
				} else { // if enemy1 is not moving, make enemy 2 switch directions (inherit enemy1's opposite dir) so they don't intersect
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

static void CalcForSpinies(player_t* player, unsigned int gameFrame, enemy_t* enemy) {
	// apply gravity
	enemy->verVel -= TO_FIXED_POINT(GRAVITY);
	
	switch (enemy->state) {
		case ENEMY_WALKING:
			if (enemy->dir == LEFT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = -enemy->maxSpeed;
			else if (enemy->dir == RIGHT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = enemy->maxSpeed;
			enemy->sprite = ((gameFrame - enemy->spawnTime)/8 % 3) + ((enemy->crabIsMad) ? 4 : 0);
			break;
		case ENEMY_TURNING:

			break;
		case ENEMY_LAYING:
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
			RedrawPipesWithNewSprite(!enemy->dir, 0, gameFrame); // pipe is opposite their dir
			enemy->verVel = 0;
			if (enemy->dir == RIGHT)
				enemy->horVel = TO_FIXED_POINT(0.5);
			else
				enemy->horVel = TO_FIXED_POINT(-0.5);
			
			if (gameFrame - enemy->spawnTime > 65)
				enemy->state = ENEMY_WALKING;
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

static void CalcForFlies(player_t* player, unsigned int gameFrame, enemy_t* enemy) {
	enemy->verVel -= TO_FIXED_POINT(GRAVITY_WINGED);
	
	switch (enemy->state) {
		case ENEMY_WALKING:
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
			if (gameFrame - enemy->layStartTime > 400) {
				enemy->state = ENEMY_WALKING;
				enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				enemy->dir = !enemy->dir; // reverse direction
			}
			if (enemy->grounded && enemy->horVel != 0)
				enemy->horVel = 0;
			break;
		case ENEMY_EXITING_PIPE:
			RedrawPipesWithNewSprite(!enemy->dir, 0, gameFrame); // pipe is opposite their dir
			enemy->verVel = 0;
			if (enemy->dir == RIGHT)
				enemy->horVel = TO_FIXED_POINT(0.5);
			else
				enemy->horVel = TO_FIXED_POINT(-0.5);
			
			if (gameFrame - enemy->spawnTime > 65)
				enemy->state = ENEMY_WALKING;
			break;
	}
	
	if (enemy->state != ENEMY_DEAD_SPINNING) {
		if (CalcCollsion(enemy, gameFrame)) { // != 0
			if (!enemy->grounded)
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

static void CalcForFreezies(player_t* player, unsigned int gameFrame, enemy_t* enemy) {
	enemy->verVel -= TO_FIXED_POINT(GRAVITY);
	
	switch (enemy->state) {
		case ENEMY_WALKING:
			if (enemy->dir == LEFT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = -enemy->maxSpeed;
			else if (enemy->dir == RIGHT && enemy->grounded == true && enemy->verVel <= 0)
				enemy->horVel = enemy->maxSpeed;
			enemy->sprite = ((gameFrame - enemy->spawnTime)/4 % 3);
			break;
		case ENEMY_TURNING:
			
			break;
		case ENEMY_EXITING_PIPE:
			RedrawPipesWithNewSprite(!enemy->dir, 0, gameFrame); // pipe is opposite their dir
			enemy->verVel = 0;
			if (enemy->dir == RIGHT)
				enemy->horVel = TO_FIXED_POINT(0.5);
			else
				enemy->horVel = TO_FIXED_POINT(-0.5);
			
			if (gameFrame - enemy->spawnTime > 65)
				enemy->state = ENEMY_WALKING;
			break;
		case ENEMY_DEAD_SPINNING: // enemy_dead_spinning for a freezie is just it dying
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
				if (rand() % 4 == 0 && !levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].icy)
					enemy->freezieFreezeNextPlatform = true;
				else
					enemy->freezieFreezeNextPlatform = false;
				break;
			case CHP_COL:
				if (enemy->freezieFreezeNextPlatform && 
				enemy->x + (enemy->width/2) == levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].x + (levelPlatforms.platformArray[enemy->lastGroundedPlatformIndex].width/2)) {
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
				if (enemy->y - enemy->verVel + TO_FIXED_POINT(ENEMY_SPIKE_HITBOX_HEIGHT) > levelPlatforms.platformArray[j].y 
				&& enemy->y - enemy->verVel < levelPlatforms.platformArray[j].y + TO_FIXED_POINT(PLATFORM_HEIGHT) 
				&& enemy->x + enemy->horVel + enemy->width > levelPlatforms.platformArray[j].x 
				&& enemy->x + enemy->horVel < levelPlatforms.platformArray[j].x + levelPlatforms.platformArray[j].width) {
					enemy->y = levelPlatforms.platformArray[j].y - TO_FIXED_POINT(ENEMY_SPIKE_HITBOX_HEIGHT);
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

void KillEnemy(enemy_t* enemy, player_t* player, unsigned int gameFrame) {
	enemy->verVel = (enemy->type != ENEMY_FLY) ? TO_FIXED_POINT(3) : TO_FIXED_POINT(2);
	enemy->state = ENEMY_DEAD_SPINNING;
	if (player->dir == RIGHT)
		enemy->horVel = TO_FIXED_POINT(1.5);
	else
		enemy->horVel = TO_FIXED_POINT(-1.5);
	enemy->grounded = false;
	
	--levelEnemies.enemiesLeft;
	SpawnBonusCoin(0, 0, false, enemy->dir, gameFrame);
	EnemyShowScore(enemy, player, gameFrame);
	player->lastKilledEnemyTime = gameFrame;
}