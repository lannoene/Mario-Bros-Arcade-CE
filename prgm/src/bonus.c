#include "bonus.h"

#include <stdlib.h>
#include <graphx.h>

#include "player.h"

bonusLevel_t levelCoins;

void InitBonusData(void) {
	levelCoins.coinArray = malloc(0);
	levelCoins.bonusTimer = 1200;
	levelCoins.numCoins = levelCoins.coinsLeft = 0;
}

void SpawnBonusCoin(int16_t x, uint8_t y) {
	++levelCoins.numCoins;
	++levelCoins.coinsLeft;
	
	levelCoins.coinArray = realloc(levelCoins.coinArray, levelCoins.numCoins*sizeof(bonusCoin_t));
	levelCoins.coinArray[levelCoins.numCoins - 1].x = x;
	levelCoins.coinArray[levelCoins.numCoins - 1].y = y;
	levelCoins.coinArray[levelCoins.numCoins - 1].alive = true;
	levelCoins.coinArray[levelCoins.numCoins - 1].backgroundData[0] = COIN_WIDTH;
	levelCoins.coinArray[levelCoins.numCoins - 1].backgroundData[1] = COIN_HEIGHT;
}

void CheckCoinColision(player_t* player) {
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive && player->y - player->verAccel + PLAYER_HEIGHT > levelCoins.coinArray[i].y && player->y - player->verAccel < levelCoins.coinArray[i].y + COIN_HEIGHT && player->x + PLAYER_WIDTH + player->horAccel > levelCoins.coinArray[i].x && player->x + player->horAccel < levelCoins.coinArray[i].x + COIN_WIDTH) {
			levelCoins.coinArray[i].alive = false;
			--levelCoins.coinsLeft;
		}
	}
}

void ResetCoins(void) {
	for (uint8_t i = 0; i < levelCoins.numCoins; i++) {
		gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, levelCoins.coinArray[i].x, levelCoins.coinArray[i].y);
		gfx_SetDrawScreen();
		gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, levelCoins.coinArray[i].x, levelCoins.coinArray[i].y);
		gfx_SetDrawBuffer();
	}
	levelCoins.numCoins = 0;
}

void FreeBonusCoins(void) {
	free(levelCoins.coinArray);
}