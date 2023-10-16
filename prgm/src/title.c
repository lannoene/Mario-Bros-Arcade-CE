#include "title.h"
#include "screens.h"
#include "gfx/gfx.h"
#include "font/font.h"
#include <graphx.h>
#include <compression.h>
#include <keypadc.h>

bool LoadTitle(void) {
	gfx_FillScreen(2);
	
	gfx_SetFontData(font);
	gfx_SetMonospaceFont(8);
	
	zx7_Decompress(gfx_vbuffer, bg_title_compressed);
	
	gfx_SetTextFGColor(1);
	gfx_PrintStringXY("Press Enter", 110, 190);
	
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