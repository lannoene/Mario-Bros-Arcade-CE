#include "icicles.h"

#include <stdlib.h>
#include <graphx.h>
#include <sys/util.h>

#include "platforms.h"
#include "player.h"
#include "defines.h"
#include "level.h"

#define ICICLE_TOP_Y 47
#define ICICLE_TOP_LEFT_X 40
#define ICICLE_TOP_RIGHT_X 265

levelIcicleData_t levelIcicles;
icicle_t icicleArray[8];

void InitIcicles(void) {
	levelIcicles.icicleArray = icicleArray;
	levelIcicles.numIcicles = ARR_LEN(icicleArray);
	levelIcicles.hasIciclesTop[LEFT] = false;
	levelIcicles.hasIciclesTop[RIGHT] = false;
	for (uint8_t i = 0; i < levelIcicles.numIcicles; i++) {
		levelIcicles.icicleArray[i].state = ICICLE_DEAD;
	}
}

static inline bool IcicleOverlapping(icicle_t *icicle) {
	FOR_EACH(levelIcicles.icicleArray, levelIcicles.numIcicles, icicle2) {
		if (icicle2 != icicle
			&& AABB(
				icicle->x, icicle->y, ICICLE_WIDTH, 1,
				icicle2->x, icicle2->y, ICICLE_WIDTH, 1
				)) {
			return true;
		}
	}
	return false;
}

void SpawnIcicle(unsigned int gameFrame) {
	icicle_t *icicle = NULL;
	FOR_EACH(levelIcicles.icicleArray, levelIcicles.numIcicles, _icicle) {
		if (_icicle->state == ICICLE_DEAD) {
			icicle = _icicle;
			break;
		}
	} FOR_ELSE {
		dbg_printf("The maximum number of icicles has been reached.");
		return;
	}
	icicle->spawnTime = gameFrame;
	icicle->backgroundData[0] = ICICLE_WIDTH;
	icicle->backgroundData[1] = ICICLE_HEIGHT;
	icicle->state = ICICLE_FORMING;
	icicle->sprite = 0;
	icicle->verAccel = 0;
	icicle->isOnTop = false;
	
	bool sideSelect = randInt(0, 1) == 0; // left or right
	bool pipeSelect = randInt(0, 4) == 0; // is on pipe or not
	
	icicle->isOnTop = pipeSelect;
	if (pipeSelect && !levelIcicles.hasIciclesTop[sideSelect]) {
		// spawn on pipe
		icicle->y = ICICLE_TOP_Y;
		icicle->x = (sideSelect) ? ICICLE_TOP_LEFT_X : ICICLE_TOP_RIGHT_X;
		levelIcicles.hasIciclesTop[sideSelect] = true;
	} else {
		// do regular
		icicle->y = 72 + PLATFORM_HEIGHT;
		do {
			icicle->x = ((sideSelect) ? 5 : 176) + randInt(0, 129);
		} while (IcicleOverlapping(icicle));
	}
	
	icicle->y_old = icicle->y;
	icicle->x_old = icicle->x;
}

void UpdateIcicles(player_t* players, unsigned int gameFrame) {
	FOR_EACH(levelIcicles.icicleArray, levelIcicles.numIcicles, icicle) {
		if (icicle->state == ICICLE_DEAD)
			continue;
		
		uint16_t dtSpawnTime = gameFrame - icicle->spawnTime;
		
		switch (icicle->state) {
			case ICICLE_FORMING:
				if (dtSpawnTime >= 158) {
					icicle->state = ICICLE_FALLING;
				} else if (dtSpawnTime >= 148) {
					icicle->sprite = ((gameFrame - icicle->spawnTime)/4 % 2) + 3;
				} else if (dtSpawnTime >= 131) {
					icicle->sprite = 5;
				} else if (dtSpawnTime >= 127) {
					icicle->sprite = 2;
				} else if (dtSpawnTime >= 64) {
					icicle->sprite = 1;
				}
				
				FOR_EACH(players, game_data.numPlayers, player) {
					if (dtSpawnTime < 131) {
						// while the icicle is still forming, people can interact with it to destroy the icicle
						if (AABB(player->x, player->y, I2FP(PLAYER_WIDTH), I2FP(PLAYER_HEIGHT), 
							I2FP(icicle->x), I2FP(icicle->y), I2FP(ICICLE_WIDTH), I2FP((dtSpawnTime*142)/1000))
							&& player->state == PLAYER_NORMAL) {
							/*
							 * make sure icicle is properly disposed of offscreen.
							 * if the icicle were to instantly be stopped drawing,
							 * a ghost of it would be kept on screen because of the double buffering
							 */
							icicle->y = 241;
							// setting state to icicle falling lets the other buffer update
							icicle->state = ICICLE_FALLING; 
						}
					} else {
						// when the icicle is done forming, people cannot touch it, or they will die.
						if (AABB(player->x, player->y, I2FP(PLAYER_WIDTH), I2FP(PLAYER_HEIGHT), 
							I2FP(icicle->x), I2FP(icicle->y), I2FP(ICICLE_WIDTH), I2FP(ICICLE_HEIGHT))
							&& player->state == PLAYER_NORMAL) {
							KillPlayer(player, gameFrame);
						}
					}
				}
				
				break;
			case ICICLE_FALLING:
				if (icicle->y == ICICLE_TOP_Y) {
					if (icicle->x == ICICLE_TOP_LEFT_X)
						levelIcicles.hasIciclesTop[LEFT] = false;
					else if (icicle->x == ICICLE_TOP_RIGHT_X)
						levelIcicles.hasIciclesTop[RIGHT] = false;
				}
				// now let icicle fall
				icicle->sprite = ((dtSpawnTime)/8 % 2) + 3;
				
				/*
				 * i don't want verAccel to be a float, and the GRAVITY constant is 0.2.
				 * 0.2 as a fraction is 1/5. take 1 away every 5 ticks/frames.
				 * and yes, i know it should be ((gameFrame - spawntime) % 5 == 0),
				 * but i don't want to do that extra calculation when it probably wont make a difference
				 */
				icicle->verAccel -= !(gameFrame % 5);
				
				FFOR_EACH(players, game_data.numPlayers, player) {
					if (AABB(player->x, player->y, I2FP(PLAYER_WIDTH), I2FP(PLAYER_HEIGHT),
						I2FP(icicle->x), I2FP(icicle->y + icicle->verAccel), I2FP(ICICLE_WIDTH), I2FP(ICICLE_HEIGHT))
						&& player->state == PLAYER_NORMAL) {
						KillPlayer(player, gameFrame);
					}
				}
				
				if (icicle->y > 240)
					icicle->state = ICICLE_DEAD; // mark icicle free
				break;
		}
		icicle->y -= icicle->verAccel;
	}
}

void FreeIcicles(void) {
	//free(levelIcicles.icicleArray);
}

void ResetIcicles(void) {
	// replace all live icicles with their bg
	FOR_EACH(levelIcicles.icicleArray, levelIcicles.numIcicles, icicle) {
		if (icicle->state != ICICLE_DEAD) {
			gfx_Sprite((gfx_sprite_t*)icicle->backgroundData, icicle->x, icicle->y);
			gfx_SetDrawScreen();
			gfx_Sprite((gfx_sprite_t*)icicle->backgroundData, icicle->x, icicle->y);
			gfx_SetDrawBuffer();
			icicle->state = ICICLE_DEAD;
		}
	}
	//levelIcicles.numIcicles = 0;
}