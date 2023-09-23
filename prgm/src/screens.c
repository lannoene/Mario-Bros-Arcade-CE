#include "screens.h"
#include "title.h"
#include "level.h"

static int currentScreen = SCR_TITLE_LOAD;
static bool (*cur_scr_r[])(void) = {RunTitle, LoadTitle, LevelLoop, LoadLevel};

bool RunScreen(void) {
	return (cur_scr_r[currentScreen]());
}

void ChangeScreen(uint8_t newScreen) {
	currentScreen = newScreen;
}