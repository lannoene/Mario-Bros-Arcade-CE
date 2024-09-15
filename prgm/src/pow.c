#include "pow.h"

#include <stdlib.h>

#include "enemies.h"
#include "defines.h"

powInfo_t levelPows;

void InitPows(void) {
	levelPows.numPows = 0;
	levelPows.powArray = malloc(0);
}

void CreatePow(int16_t x, uint8_t y) {
	++levelPows.numPows;
	levelPows.powArray = realloc(levelPows.powArray, levelPows.numPows*sizeof(pow_t));
	levelPows.powArray[levelPows.numPows - 1].x = TO_FIXED_POINT(x);
	levelPows.powArray[levelPows.numPows - 1].y = TO_FIXED_POINT(y);
	levelPows.powArray[levelPows.numPows - 1].state = 0;
	levelPows.powArray[levelPows.numPows - 1].backgroundData[0] = POW_SIZE;
	levelPows.powArray[levelPows.numPows - 1].backgroundData[1] = POW_SIZE;
}

void FreePows(void) {
	free(levelPows.powArray);
}

void BumpPow(player_t* player, uint8_t powIndex, unsigned int gameFrame) {
	++levelPows.powArray[powIndex].state;
	for (uint8_t i = 0; i < levelEnemies.numEnemies; i++) {
		enemy_t* enemy = &levelEnemies.enemyArray[i];
		if (enemy->grounded && enemy->state != ENEMY_DEAD) {
			if (enemy->state != ENEMY_LAYING && enemy->state != ENEMY_DEAD_SPINNING) {
				if (enemy->type == ENEMY_CRAB && !enemy->crabIsMad)
					enemy->crabIsMad = true;
				else if (enemy->type == ENEMY_FREEZIE) {
					enemy->state = ENEMY_DEAD_SPINNING;
					enemy->eventTime = gameFrame;
				} else if (enemy->type == ENEMY_COIN) {
					continue;
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
			} else if (enemy->state != ENEMY_DEAD_SPINNING) {
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
}

void ResetPows(void) {
	for (uint8_t i = 0; i < levelPows.numPows; i++) {
		levelPows.powArray[i].state = POW_FULL;
	}
}