#include <graphx.h>
#include <keypadc.h>
#include <tice.h>

#include "screens.h"
#include "draw.h"

#include "gfx/gfx.h"

int main(void) {
	srand(rtc_Time());
	
	// Initialize graphics drawing
	gfx_Begin();
	gfx_SetDrawBuffer();
	
	// Set the palette for sprites
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	
	DecompressSprites();
	
	// These were set in the image conversion file
	gfx_SetTransparentColor(3);
	gfx_FillScreen(2);
	do {
		// scan keypresses
		kb_Scan();
	} while (RunScreen());
	
	// End graphics drawing
	gfx_End();
	
	return 0;
}
