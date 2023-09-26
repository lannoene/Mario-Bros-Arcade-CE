#include <graphx.h>
#include <keypadc.h>
#include <tice.h>

#include "screens.h"
#include "draw.h"

#include "gfx/gfx.h"

/* Lag producers top 5:
1	Pipes
2.	Enemies
3.	Ground
4.	Player
5.	Colision in general
That's literally everything i've added to the game lol */

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
