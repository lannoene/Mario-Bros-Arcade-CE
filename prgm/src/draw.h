#pragma once
#include "player.h"
#include <graphx.h>

extern gfx_sprite_t* mario_sprites[2][5];
extern uint8_t mario_walking_sprite_table[4];

void DrawScene(player_t* player, int gameFrame);
void DrawBackground(void);