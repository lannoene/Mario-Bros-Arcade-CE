#include <graphx.h>
#include <keypadc.h>
#include <tice.h>

#include "screens.h"
#include "draw.h"

#include "gfx/gfx.h"

/* 
 * i am sorry you are reading this
 * i tried to make the code a bit more readable,
 * but with the new optimizations i added
 * i took 1 step forward and 1 step back in raedablility
 */

// imagine if this was multiplayer? that would be awesome
// the only problem is that i just have one calc

int main(void) {
	srand(rtc_Time());
	
	// Initialize graphics drawing
	gfx_Begin();
	gfx_SetDrawBuffer();
	
	// Set the palette for sprites
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	SetUpPalettes();
	
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
