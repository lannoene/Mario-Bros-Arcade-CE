#include "title.h"
#include "screens.h"
#include "gfx/gfx.h"
#include <graphx.h>
#include <compression.h>
#include <keypadc.h>

bool LoadTitle(void) {
	gfx_FillScreen(2);
	
	
	zx7_Decompress(gfx_vbuffer, bg_title_compressed);
	gfx_sprite_t* tmp_title_card = gfx_MallocSprite(mario_title_card_width, mario_title_card_height);
	zx7_Decompress(tmp_title_card->data, mario_title_card_compressed);
	
	gfx_Sprite_NoClip(tmp_title_card, 10, 80);
	free(tmp_title_card);
	gfx_SwapDraw();
	gfx_BlitScreen();
	ChangeScreen(SCR_TITLE);
	return true;
}

bool RunTitle(void) {
	// title should be simple, so no need to split into seperate functions
	// draw
	
	if (kb_Data[6] & kb_Enter) {
		ChangeScreen(SCR_LEVEL_LOAD);
	}
	if (kb_Data[6] & kb_Clear) {
		return false;
	}
	
	gfx_SwapDraw();
	
	return true;
}