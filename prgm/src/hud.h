#pragma once
#include "player.h"

#define TEXT_SIZE 8

void InitHud(void);
void HudGetBackground(void);
void HudDraw(player_t* player, unsigned int gameFrame);
void HudRefresh(void);
void TitleCardSetNumDigits(uint8_t numDigits);