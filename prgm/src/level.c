#include "level.h"


#include <keypadc.h>
#include <graphx.h>

#include "screens.h"
#include "draw.h"
#include "player.h"
#include "platforms.h"

int gameFrame = 0;
player_t mario1;
gameData_t game_data = {};

bool GameLoop(void) {
	
	mario1.moving = false;
	// check for button presses
	if (kb_Data[6] & kb_Clear) {
		return false;
	}
	if (kb_Data[7] & kb_Right) {
		PlayerMove(&mario1, RIGHT);
	}
	if (kb_Data[7] & kb_Left) {
		PlayerMove(&mario1, LEFT);
	}
	if (kb_Data[1] & kb_2nd) {
		PlayerMove(&mario1, UP); // jump
	}
	
	// update player
	UpdatePlayer(&mario1, gameFrame);
	
	// draw updated player
	DrawScene(&mario1, gameFrame);
	++gameFrame;
	
	return true;
}

bool LoadGame(void) {
	// we only need to draw the background once because we perform non-destructive sprite placement during the game loop in order to save on resources
	DrawBackground();
	
	// change windows
	ChangeScreen(SCR_GAME);
	
	// init player
	PlayerInit(&mario1);
	
	numPlatforms = 1;
	
	// return
	return true;
}