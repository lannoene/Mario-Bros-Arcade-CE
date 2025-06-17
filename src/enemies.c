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

enemy_info_t levelEnemyInfo[5] = {
	{
		.enemyId = ENEMY_SPIKE,
		.spawnCost = 1,
		.autospawn = true,
	},
	{
		.enemyId = ENEMY_CRAB,
		.spawnCost = 1,
		.autospawn = true,
	},
	{
		.enemyId = ENEMY_FLY,
		.spawnCost = 1,
		.autospawn = true,
	},
	{
		.enemyId = ENEMY_FREEZIE,
		.spawnCost = 0,
		.autospawn = false,
	},
	{
		.enemyId = ENEMY_COIN,
		.spawnCost = 0,
		.autospawn = false,
	},
};

levelEnemies_t levelEnemies;
extern int gameFrame;

static inline void EnterRespawnPipe(enemy_t* enemy, unsigned int gameFrame);

static void CalcForSpinies(unsigned int gameFrame, enemy_t* enemy);
static void CalcForFlies(unsigned int gameFrame, enemy_t* enemy);
static void CalcForFreezies(unsigned int gameFrame, enemy_t* enemy);
static void CalcForCoins(unsigned int gameFrame, enemy_t *coin);

static uint8_t CalcCollsion(enemy_t* enemy, unsigned int gameFrame);

#define MAX_ENEMIES 30

void InitEnemies(void) {
	memset(&levelEnemies, 0, sizeof(levelEnemies_t));
	static enemy_t eArray[MAX_ENEMIES];
	levelEnemies.enemyArray = eArray;
	levelEnemies.numAllocated = MAX_ENEMIES;
	memset(levelEnemies.enemyArray, 0, levelEnemies.numAllocated*sizeof(enemy_t));
	dbg_printf("init enemies: nun enemies: %d\n", levelEnemies.numAllocated);
}

enemy_t *SpawnEnemy(uint8_t enemyType, bool direction, unsigned int gameFrame) {
	enemy_t* enemy = NULL;
	FOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, _enemy) {
		if (_enemy->state == ENEMY_DEAD) {
			enemy = _enemy;
			break;
		}
	} FOR_ELSE {
		dbg_printf("ERROR: COULD NOT ALLOCATE ENEMY\n");
		return NULL;
	}
	memset(enemy, 0, sizeof(enemy_t)); // remember, you don't need to set things to null/0 because this does it already
	enemy->type = enemyType;
	enemy->height = I2FP(ENEMY_SPIKE_HITBOX_HEIGHT);
	enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
	if (enemyType == ENEMY_FREEZIE) {
		enemy->horSpriteOffset = enemy->horSpriteOffset_old = (ENEMY_FREEZIE_WIDTH - ENEMY_SPIKE_SIZE); // freezies are smaller width wise, so we need to compensate for that, and the best way to do that without jank was this
		enemy->width = I2FP(ENEMY_FREEZIE_WIDTH);
	} else if (enemyType == ENEMY_COIN) {
		enemy->width = I2FP(COIN_WIDTH);
		enemy->height = I2FP(COIN_HEIGHT);
		enemy->verSpriteOffset = 0;
	} else {
		enemy->horSpriteOffset = enemy->horSpriteOffset_old = 0;
		enemy->width = I2FP(ENEMY_SPIKE_SIZE);
	}
	enemy->spawnTime = gameFrame;
	enemy->state = ENEMY_EXITING_PIPE;
	if (levelEnemies.lastSpawnedPipe == RIGHT) {
		enemy->x = I2FP(30);
		enemy->x_old = I2FP(30);
		enemy->dir = RIGHT;
		levelEnemies.lastSpawnedPipe = LEFT;
	} else {
		enemy->x = I2FP(274);
		enemy->x_old = I2FP(274);
		enemy->dir = LEFT;
		levelEnemies.lastSpawnedPipe = RIGHT;
	}
	enemy->y = I2FP(35);
	enemy->y_old = I2FP(35);
	enemy->backgroundData[0] = ENEMY_SPIKE_SIZE;
	enemy->backgroundData[1] = ENEMY_SPIKE_SIZE;
	if (enemyType == ENEMY_FLY)
		enemy->maxSpeed = I2FP(0.4);
	else if (enemyType == ENEMY_FREEZIE)
		enemy->maxSpeed = I2FP(0.55);
	else
		enemy->maxSpeed = I2FP(0.5);
	enemy->verSpriteOffset_old = enemy->verSpriteOffset;
	levelEnemies.lastSpawnedTime = gameFrame;
	return enemy;
}

void FreeEnemies(void) {
	//free(levelEnemies.enemyArray);
}

void UpdateEnemies(unsigned int gameFrame) {
	void *maxEnemyPtr = &levelEnemies.enemyArray[levelEnemies.numAllocated];
	for (enemy_t* enemy = levelEnemies.enemyArray; enemy != maxEnemyPtr; enemy++) {
		enemy->lastState = enemy->state;
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
			default:
				dbg_printf("Unknown enemy type: %d\n", enemy->type);
				break;
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
		if ((gameFrame & 2) && !enemy->bonus && enemy->state >= ENEMY_WALKING) {
			for (enemy_t* oEnmy = enemy + 1; oEnmy != maxEnemyPtr; oEnmy++) { // check for other enemy colision. (check every combination, not permutation)
				int x1 = enemy->x + enemy->horVel;
				int y1 = enemy->y - enemy->verVel;
				int x2 = oEnmy->x + oEnmy->horVel;
				int y2 = oEnmy->y - oEnmy->verVel;
				if (AABB(x1, y1, enemy->width, enemy->height,
						x2, y2, oEnmy->width, oEnmy->height)
					&& (oEnmy->state >= ENEMY_WALKING)
					&& (enemy->lastBumpedEnemy != oEnmy || gameFrame - enemy->lastBumpedEnemyTime > 90)) {
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
	enemy->y = I2FP(34);
	enemy->state = ENEMY_EXITING_PIPE;
	enemy->spawnTime = gameFrame;
	enemy->lastGroundedPlatform = NULL;
	enemy->grounded = false;
	//AddEnemyToPipeQueue(enemy, !enemy->dir);
	
	if (enemy->dir == RIGHT) {
		enemy->x = I2FP(274);
		enemy->dir = LEFT;
	} else {
		enemy->x = I2FP(30);
		enemy->dir = RIGHT;
	}
	RedrawPipesWithNewSprite(!enemy->dir, 0, gameFrame); // make sure that the enemy does not flicker when exiting
}

static void CalcForSpinies(unsigned int gameFrame, enemy_t* enemy) {
	switch (enemy->state) {
		case ENEMY_WALKING:
			enemy->verVel -= I2FP(GRAVITY);
			if (enemy->dir == LEFT && enemy->grounded && enemy->verVel <= 0)
				enemy->horVel = -enemy->maxSpeed;
			else if (enemy->dir == RIGHT && enemy->grounded && enemy->verVel <= 0)
				enemy->horVel = enemy->maxSpeed;
			enemy->sprite = ((gameFrame - enemy->spawnTime)/8 % 3) + ((!!enemy->crabIsMad)*4);
			break;
		case ENEMY_TURNING:

			break;
		case ENEMY_LAYING:
			enemy->verVel -= I2FP(GRAVITY);
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
			enemy->verVel -= I2FP(GRAVITY);
			break;
	}
	
	if (enemy->state != ENEMY_DEAD_SPINNING) {
		CalcCollsion(enemy, gameFrame);
	} else {
		if (enemy->x < I2FP(-ENEMY_SPIKE_SIZE)) // enemy on sides off screen? if so, teleport them to other side
			enemy->x = I2FP(GFX_LCD_WIDTH);
		else if (enemy->x > I2FP(GFX_LCD_WIDTH))
			enemy->x = I2FP(-ENEMY_SPIKE_SIZE); // 0 - ENEMY_SPIKE_SIZE
		
		if (enemy->y > I2FP(GFX_LCD_HEIGHT)) // wait for enemy to leave the screen
			enemy->state = ENEMY_DEAD;
		
		if (enemy->horVel > -I2FP(ENEMY_DEAD_DECELERATION) && enemy->horVel < I2FP(ENEMY_DEAD_DECELERATION))
			enemy->horVel = 0;
		else if (enemy->horVel > 0)
			enemy->horVel -= I2FP(ENEMY_DEAD_DECELERATION);
		else if (enemy->horVel < 0)
			enemy->horVel += I2FP(ENEMY_DEAD_DECELERATION);
		
		if (enemy->verVel < I2FP(-4)) // cap fall speed
			enemy->verVel = I2FP(-4);
	}
}

static void CalcForFlies(unsigned int gameFrame, enemy_t* enemy) {
	switch (enemy->state) {
		case ENEMY_WALKING:
			enemy->verVel -= I2FP(GRAVITY_WINGED);
			enemy->sprite = ((gameFrame - enemy->spawnTime)/8 % 2) + 1;
			
			if (enemy->grounded) {
				enemy->horVel = 0;
				enemy->sprite = 0;
				if (gameFrame - enemy->groundedStartTime > 40) {
					enemy->verVel = I2FP(1.5);
					enemy->horVel = (enemy->dir == LEFT) ? -enemy->maxSpeed : enemy->maxSpeed;
					enemy->grounded = false;
				}
			}
			break;
		case ENEMY_TURNING:
			
			break;
		case ENEMY_LAYING:
			enemy->verVel -= I2FP(GRAVITY_WINGED);
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
			enemy->verVel -= I2FP(GRAVITY_WINGED);
			break;
	}
	
	if (enemy->state != ENEMY_DEAD_SPINNING) {
		bool oldEnemyGrounded = enemy->grounded;
		if (CalcCollsion(enemy, gameFrame)) { // != 0
			if (!oldEnemyGrounded && enemy->grounded)
				enemy->groundedStartTime = gameFrame;
		}
	} else {
		if (enemy->x < I2FP(-ENEMY_SPIKE_SIZE)) // enemy on sides off screen? if so, teleport them to other side
			enemy->x = I2FP(GFX_LCD_WIDTH);
		else if (enemy->x > I2FP(GFX_LCD_WIDTH))
			enemy->x = I2FP(-ENEMY_SPIKE_SIZE); // 0 - ENEMY_SPIKE_SIZE
		
		if (enemy->horVel > -I2FP(ENEMY_DEAD_DECELERATION) && enemy->horVel < I2FP(ENEMY_DEAD_DECELERATION))
			enemy->horVel = 0;
		else if (enemy->horVel > 0)
			enemy->horVel -= I2FP(ENEMY_DEAD_DECELERATION);
		else if (enemy->horVel < 0)
			enemy->horVel += I2FP(ENEMY_DEAD_DECELERATION);
		
		if (enemy->y > I2FP(GFX_LCD_HEIGHT)) // wait for enemy to leave the screen
			enemy->state = ENEMY_DEAD;
		
		if (enemy->verVel < I2FP(-4)) // cap fall speed
			enemy->verVel = I2FP(-4);
	}

}

static void CalcForFreezies(unsigned int gameFrame, enemy_t* enemy) {
	switch (enemy->state) {
		case ENEMY_WALKING:
			enemy->verVel -= I2FP(GRAVITY);
			if (enemy->dir == LEFT && enemy->grounded && enemy->verVel <= 0)
				enemy->horVel = -enemy->maxSpeed;
			else if (enemy->dir == RIGHT && enemy->grounded && enemy->verVel <= 0)
				enemy->horVel = enemy->maxSpeed;
			enemy->sprite = ((gameFrame - enemy->spawnTime)/4 % 3);
			break;
		case ENEMY_TURNING:
			
			break;
		case ENEMY_DEAD_SPINNING: // enemy_dead_spinning for a freezie is just it dying
			enemy->grounded = false;
			enemy->verVel -= I2FP(GRAVITY);
			enemy->sprite = ((gameFrame - enemy->eventTime)/4 % 5) + 3;
			if (gameFrame - enemy->eventTime > 6) {
				enemy->verVel = 0;
				if (gameFrame - enemy->eventTime >= 20) {
					enemy->state = ENEMY_DEAD;
					enemy->y = I2FP(250);
				}
			}
			break;
		case FREEZIE_FREEZING_PLATFORM:
			enemy->verVel = 0;
			enemy->sprite = ((gameFrame - enemy->eventTime)/4 % 5) + 3;
			if (gameFrame - enemy->eventTime >= 20) { // actually kill the enemy
				enemy->state = ENEMY_DEAD;
				enemy->y = I2FP(250);
			}
			break;
	}
	
	if (enemy->state == ENEMY_WALKING) {
		switch (CalcCollsion(enemy, gameFrame)) {
			case EXP_COL: // expesnive col only gets run when the enemy is off of the platform
				enemy->freezieFreezeNextPlatform = randInt(0, 2) == 0 && enemy->lastGroundedPlatform && !enemy->lastGroundedPlatform->icy;
				break;
			case CHP_COL: {
				uint16_t hfWid = enemy->lastGroundedPlatform->width/2;
				if (enemy->freezieFreezeNextPlatform 
					&& enemy->x + enemy->width > enemy->lastGroundedPlatform->x + hfWid
					&& enemy->x < enemy->lastGroundedPlatform->x + hfWid) {
					FreezePlatform(enemy->lastGroundedPlatform);
					enemy->state = FREEZIE_FREEZING_PLATFORM;
					enemy->eventTime = gameFrame;
					enemy->horVel = 0;
				}
				break;
			}
		}
	}
}

void ResetEnemies(unsigned int gameFrame, bool fast) {
	FOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
		if (enemy->state != ENEMY_DEAD) { // this should honestly only be useful for freezies
			if (enemy->type == ENEMY_FREEZIE) {
				if (enemy->state != ENEMY_EXITING_PIPE) {
					enemy->state = FREEZIE_FREEZING_PLATFORM; // freezie freezing platform is just the normal death animation but it sets the ver accel to 0 automatically
					enemy->eventTime = gameFrame;
					enemy->horVel = 0;
				} else {
					enemy->state = ENEMY_DEAD;
					enemy->y = I2FP(250);
					enemy->verVel = 0;
				}
			} else {
				enemy->state = ENEMY_DEAD_SPINNING;
				if (fast)
					enemy->y = I2FP(250);
			}
		}
	}
}

void EnemyShowScore(enemy_t* enemy, player_t* player, unsigned int gameFrame) {
	if (gameFrame - player->lastKilledTime > 55) {
		player->currentCombo = 1;
	} else {
		++player->currentCombo;
	}
	EnemyShowScoreIndividual(enemy, player, 800*player->currentCombo, gameFrame);
	
	player->lastKilledTime = gameFrame;
}

void EnemyShowScoreIndividual(enemy_t* enemy, player_t* player, unsigned int amount, unsigned int gameFrame) {
	uint16_t x = FP2I(enemy->x);
	uint8_t y = FP2I(enemy->y);
	switch (amount) {
		case 500:
			SpawnParticle(x, y, PARTICLE_SCORE_500, gameFrame);
			break;
		case 800:
			SpawnParticle(x, y, PARTICLE_SCORE_800, gameFrame);
			break;
		case 1600:
			SpawnParticle(x, y, PARTICLE_SCORE_DUB, gameFrame);
			break;
		case 2400:
			SpawnParticle(x, y, PARTICLE_SCORE_TRP, gameFrame);
			break;
		case 3200:
			SpawnParticle(x, y, PARTICLE_SCORE_QDP, gameFrame);
			break;
		default:
			SpawnParticle(x, y, PARTICLE_SCORE_1UP, gameFrame);
	}
	PlayerAddScore(player, MIN(amount, 3200));
}

static uint8_t CalcCollsion(enemy_t* enemy, unsigned int gameFrame) {
	if (enemy->x < -enemy->width) // enemy on sides off screen? if so, teleport them to other side
		enemy->x = I2FP(GFX_LCD_WIDTH);
	else if (enemy->x > I2FP(GFX_LCD_WIDTH))
		enemy->x = -enemy->width; // 0 - enemy->width
	
	// if enemy is on ground and is out of view, do the thing idk
	if ((enemy->x + enemy->horVel >= I2FP(GFX_LCD_WIDTH) || enemy->x + enemy->horVel <= -enemy->width) && enemy->y > I2FP(176 + PLATFORM_HEIGHT)) { // 176 = bottom platforms' height. i dont wanna make macro though
		EnterRespawnPipe(enemy, gameFrame);
	}
	
	if (enemy->verVel <= 0) {
		int x = enemy->x + enemy->horVel;
		int y = enemy->y - enemy->verVel;
		if (enemy->grounded
			&& enemy->lastGroundedPlatform
			&& x + enemy->width > enemy->lastGroundedPlatform->x 
			&& x < enemy->lastGroundedPlatform->x + enemy->lastGroundedPlatform->width) { // is enemy still inside of last intersected platform's x footprint? if so, make sure they stay like so
			enemy->verVel = 0;
			return CHP_COL;
		} else if (y > I2FP(GROUND_HEIGHT - ENEMY_SPIKE_HITBOX_HEIGHT)) { // is enemy on the bottom floor? if so, we can skip colision checking and make sure verVel is 0
			enemy->y = I2FP(GROUND_HEIGHT - ENEMY_SPIKE_HITBOX_HEIGHT);
			enemy->verVel = 0;
			enemy->grounded = true;
			return BTM_COL;
		} else { // otherwise, do expensive colision checking
			FFOR_EACH(levelPlatforms.platformArray, levelPlatforms.numPlatforms, platform) {
				if (AABB(
					x, y, enemy->width, enemy->height,
					platform->x, platform->y, platform->width, I2FP(PLATFORM_HEIGHT)
				)) {
					enemy->y = platform->y - enemy->height;
					enemy->verVel = 0;
					enemy->grounded = true;
					enemy->lastGroundedPlatform = platform;
					enemy->groundedStartTime = gameFrame;
					return EXP_COL;
				}
			}
			enemy->grounded = false;
			return NO_COL;
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
	particle_t *p;
	p = SpawnParticle(FP2I(enemy->x) + 2, FP2I(enemy->y) + 2, PARTICLE_STAR, gameFrame);
	if (p)
		p->opt1 = 0;
	p = SpawnParticle(FP2I(enemy->x) - 2, FP2I(enemy->y) + 3, PARTICLE_STAR, gameFrame);
	if (p)
		p->opt1 = 1;
	p = SpawnParticle(FP2I(enemy->x) + 2, FP2I(enemy->y) - 2, PARTICLE_STAR, gameFrame);
	if (p)
		p->opt1 = 2;
	p = SpawnParticle(FP2I(enemy->x) - 2, FP2I(enemy->y) - 1, PARTICLE_STAR, gameFrame);
	if (p)
		p->opt1 = 3;
	
	EnemyShowScore(enemy, player, gameFrame);
	player->lastKilledEnemyTime = gameFrame;
}

static void CalcForCoins(unsigned int gameFrame, enemy_t *coin) {
	coin->sprite = ((gameFrame - coin->spawnTime)/4) % 5;
	
	if (coin->bonus && coin->state != ENEMY_DEAD_SPINNING)
		return;
	
	switch (coin->state) {
		case ENEMY_WALKING:
			coin->verVel -= I2FP(GRAVITY);
			coin->horVel = I2FP(coin->dir == RIGHT ? 0.5 : -0.5);
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
			coin->y = I2FP(241);
			coin->state = ENEMY_DEAD;
			return;
	}
	
	CalcCollsion(coin, gameFrame);
}

void CollectCoin(player_t *player, enemy_t *coin) {
	coin->state = ENEMY_DEAD_SPINNING;
	particle_t *p = SpawnParticle(FP2I(coin->x), FP2I(coin->y), PARTICLE_COIN_PICK, gameFrame);
	if (p) {
		p->opt1 = !coin->bonus;
	}
	if (!coin->bonus) {
		PlayerAddScore(player, 800);
	}
}