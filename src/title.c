#include "title.h"

#include <graphx.h>
#include <compression.h>
#include <keypadc.h>

#include "screens.h"
#include "gfx/gfx.h"
#include "font/font.h"
#include "save.h"
#include "level.h"
#include "defines.h"

bool LoadTitle(void) {
	gfx_FillScreen(2);
	
	gfx_SetFontData(font);
	gfx_SetMonospaceFont(8);
	
	zx0_Decompress(gfx_vbuffer, bg_title_compressed);
	
	gfx_SetTextFGColor(1);
	gfx_PrintStringXY("Press Enter", 110, 190);
	
	save_t saveData = GetSaveData();
	gfx_PrintStringXY("High Score: ", 80, 140);
	gfx_SetTextXY(168, 140);
	gfx_PrintUInt(saveData.highScore, 6);
	gfx_PrintStringXY("Top Phase: ", 80, 150);
	gfx_SetTextXY(168, 150);
	gfx_PrintUInt(saveData.highLevel, 2);
	//gfx_PrintStringXY("Record Holder: ", 80, 160);
	//gfx_PrintStringXY(saveData.highScoreName, 200, 160);
	
	gfx_SwapDraw();
	gfx_BlitScreen();
	
	ChangeScreen(SCR_TITLE);
	return true;
}

void EnterSequenceValue(uint8_t value) {
	static uint8_t sequencePlace = 0;
	static uint8_t sequence[] = {
		kb_Right,
		kb_Right,
		kb_Left,
		kb_Left,
		kb_Up,
		kb_Down,
		kb_Down,
	};
	if (kb_Data[7] & sequence[sequencePlace]) {
		sequencePlace++;
		dbg_printf("got it\n");
		if (sequencePlace == ARR_LEN(sequence)) {
			game_data.numPlayers = 2;
			sequencePlace = 0;
			dbg_printf("2 player mode activated\n");
		}
	} else {
		dbg_printf("failed\n");
		sequencePlace = 0;
		game_data.numPlayers = 1;
	}
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
	
	{
		static bool pressedLastFrame = false;
		if (kb_Data[7]) {
			if (!pressedLastFrame) {
				EnterSequenceValue(kb_Data[7]);
				pressedLastFrame = true;
			}
		} else {
			pressedLastFrame = false;
		}
	}
	
	gfx_SwapDraw();
	
	return true;
}