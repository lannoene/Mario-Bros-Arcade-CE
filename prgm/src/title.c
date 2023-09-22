#include "title.h"
#include "screens.h"
#include "gfx/gfx.h"
#include <graphx.h>
#include <compression.h>
#include <keypadc.h>

bool LoadTitle(void) {
	gfx_FillScreen(2);
	// draw title card
	gfx_Sprite_NoClip(mario_title_card, 10, 80);
	gfx_SwapDraw();
	ChangeScreen(SCR_TITLE);
	return true;
}

bool RunTitle(void) {
	// title should be simple, so no need to split into seperate functions
	// draw

	kb_Scan();
	if (kb_Data[6] & kb_Enter) {
		ChangeScreen(SCR_GAME_LOAD);
		gfx_FillScreen(0);
	}
	if (kb_Data[6] & kb_Clear) {
		return false;
	}
	// copy over last screen
	gfx_BlitScreen();
	
	gfx_SwapDraw();
	
	return true;
}