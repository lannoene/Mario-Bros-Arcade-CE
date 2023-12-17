#include "pow.h"

#include <stdlib.h>

#include "enemies.h"

powInfo_t levelPows;

void InitPows(void) {
	levelPows.numPows = 0;
	levelPows.powArray = malloc(0);
}

void CreatePow(int16_t x, uint8_t y) {
	++levelPows.numPows;
	levelPows.powArray = realloc(levelPows.powArray, levelPows.numPows*sizeof(pow_t));
	levelPows.powArray[levelPows.numPows - 1].x = x;
	levelPows.powArray[levelPows.numPows - 1].y = y;
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
		if (levelEnemies.enemyArray[i].grounded && levelEnemies.enemyArray[i].state != ENEMY_DEAD) {
			if (levelEnemies.enemyArray[i].state != ENEMY_LAYING && levelEnemies.enemyArray[i].state != ENEMY_DEAD_SPINNING) {
				if (levelEnemies.enemyArray[i].type == ENEMY_CRAB && !levelEnemies.enemyArray[i].crabIsMad)
					levelEnemies.enemyArray[i].crabIsMad = true;
				else if (levelEnemies.enemyArray[i].type == ENEMY_FREEZIE) {
					levelEnemies.enemyArray[i].state = ENEMY_DEAD_SPINNING;
					levelEnemies.enemyArray[i].eventTime = gameFrame;
				} else {
					levelEnemies.enemyArray[i].state = ENEMY_LAYING;
					levelEnemies.enemyArray[i].verSpriteOffset = 0;
					PlayerAddScore(player, 10);
				}
				levelEnemies.enemyArray[i].horAccel = 0;
				
				levelEnemies.enemyArray[i].verAccel = (levelEnemies.enemyArray[i].type != ENEMY_FLY) ? 2.5 : 1.7;
				levelEnemies.enemyArray[i].layStartTime = gameFrame;
				levelEnemies.enemyArray[i].grounded = false;
				levelEnemies.enemyArray[i].sprite = 3;
			} else if (levelEnemies.enemyArray[i].state != ENEMY_DEAD_SPINNING) {
				if (levelEnemies.enemyArray[i].type == ENEMY_CRAB && levelEnemies.enemyArray[i].crabIsMad)
					levelEnemies.enemyArray[i].crabIsMad = false;
				
				levelEnemies.enemyArray[i].grounded = false;
				levelEnemies.enemyArray[i].state = ENEMY_WALKING;
				levelEnemies.enemyArray[i].verAccel = (levelEnemies.enemyArray[i].type != ENEMY_FLY) ? 2.5 : 1.7;
				levelEnemies.enemyArray[i].verSpriteOffset = (ENEMY_SPIKE_HITBOX_HEIGHT - ENEMY_SPIKE_SIZE);
				levelEnemies.enemyArray[i].horAccel = 0;
			}
		}
	}
}

void ResetPows(void) {
	for (uint8_t i = 0; i < levelPows.numPows; i++) {
		levelPows.powArray[i].state = POW_FULL;
	}
}