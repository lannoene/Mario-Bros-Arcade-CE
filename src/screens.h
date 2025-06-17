#pragma once
#include <stdbool.h>
#include <stdint.h>

void ChangeScreen(uint8_t newScreen);
bool RunScreen(void);

enum SCREEN_IDS {
	SCR_TITLE = 0,
	SCR_TITLE_LOAD,
	SCR_LEVEL,
	SCR_LEVEL_LOAD,
};