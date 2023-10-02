#pragma once

#include <stdint.h>
#include <graphx.h>

#include "player.h"

enum BACKGROUND_IDS {
	BG_PIPES,
	BG_LAVA,
	BG_CASTLE,
	BG_SNOWY
};

void DrawScene(player_t* player, uint8_t backgroundType, unsigned int gameFrame);
void DrawBackground(uint8_t backgroundId);
void DecompressSprites(void);