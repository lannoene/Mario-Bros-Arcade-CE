// PRETTY MUCH UNUSED NOW

#include "bonus.h"

#include <stdlib.h>
#include <graphx.h>
#include <string.h>

#include "defines.h"
#include "platforms.h"
#include "level.h"
#include "pipes.h"
#include "enemies.h"
#include "particles.h"

bonusLevel_t levelCoins;

void InitBonusData(void) {
	//levelCoins.coinArray = malloc(0);
	levelCoins.bonusTimer = 1200;
}