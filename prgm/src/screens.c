#include "screens.h"
#include "title.h"
#include "level.h"

int currentScreen = SCR_TITLE_LOAD;
bool (*cur_scr_r[])(void) = {RunTitle, LoadTitle, GameLoop, LoadGame};

bool RunScreen(void) {
	return (cur_scr_r[currentScreen]());
}

void ChangeScreen(uint8_t newScreen) {
	currentScreen = newScreen;
}