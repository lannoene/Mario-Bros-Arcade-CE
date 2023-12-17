#pragma once
#include "player.h"

#include <stdint.h>

#define TEXT_SIZE 8

void InitHud(void);
void HudGetBackground(void);
void HudDraw(player_t* player, unsigned int gameFrame);
void HudRefresh(unsigned int gameFrame);
void TitleCardSetNumDigits(uint8_t numDigits);
int8_t PauseScreenInputEvent(uint8_t input);
void PauseScreenResetCursorPos(void);