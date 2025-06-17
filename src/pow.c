#include "pow.h"

#include <stdlib.h>

#include "enemies.h"
#include "defines.h"
#include "fireballs.h"

powInfo_t levelPows;
pow_t powArray[2];

void InitPows(void) {
	levelPows.numPows = 0;
	levelPows.powArray = powArray;
}



void CreatePow(int16_t x, uint8_t y) {
	if (levelPows.numPows == ARR_LEN(powArray)) {
		dbg_printf("The maximum number of pows has been reached.\n");
		return;
	}
	++levelPows.numPows;
	pow_t *pow = &levelPows.powArray[levelPows.numPows - 1];
	pow->x = TO_FIXED_POINT(x);
	pow->y = TO_FIXED_POINT(y);
	pow->state = 0;
	pow->backgroundData[0] = POW_SIZE;
	pow->backgroundData[1] = POW_SIZE;
}

void FreePows(void) {
	//free(levelPows.powArray);
}

void BumpPow(player_t* player, uint8_t powIndex, unsigned int gameFrame) {
	++powArray[powIndex].state;
	FOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
		if (enemy->grounded && enemy->state != ENEMY_DEAD && enemy->state != ENEMY_DEAD_SPINNING) {
			if (enemy->state != ENEMY_LAYING) {
				if (enemy->type == ENEMY_CRAB && !enemy->crabIsMad)
					enemy->crabIsMad = true;
				else if (enemy->type == ENEMY_FREEZIE) {
					enemy->state = ENEMY_DEAD_SPINNING;
					enemy->eventTime = gameFrame;
				} else if (enemy->type == ENEMY_COIN) {
					CollectCoin(player, enemy);
				} else {
					enemy->state = ENEMY_LAYING;
					enemy->verSpriteOffset = 0;
					PlayerAddScore(player, 10);
				}
				enemy->horVel = 0;
				
				enemy->verVel = (enemy->type != ENEMY_FLY) ? TO_FIXED_POINT(2.5) : TO_FIXED_POINT(1.7);
				enemy->layStartTime = gameFrame;
				enemy->grounded = false;
				enemy->sprite = 3;
			} else {
				if (enemy->type == ENEMY_CRAB && enemy->crabIsMad)
					enemy->crabIsMad = false;
				
				enemy->grounded = false;
				enemy->state = ENEMY_WALKING;
				enemy->verVel = (enemy->type != ENEMY_FLY) ? TO_FIXED_POINT(2.5) : TO_FIXED_POINT(1.7);
				enemy->verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				enemy->horVel = 0;
			}
		}
	}
	FOR_EACH(levelFireballs.fireballArray, levelFireballs.numFireballs, fireball) {
		if (fireball->alive)
			KillFireball(fireball, gameFrame);
	}
}

void ResetPows(void) {
	for (uint8_t i = 0; i < ARR_LEN(powArray); i++) {
		powArray[i].state = POW_FULL;
	}
}