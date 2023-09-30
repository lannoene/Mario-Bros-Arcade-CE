#pragma once
#include "player.h"

#define TEXT_SIZE 8

void InitHud(player_t* player);
void HudGetBackground(void);
void HudDraw(player_t* player, unsigned int gameFrame);
void HudRefresh(void);