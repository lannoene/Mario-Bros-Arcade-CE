#include <graphx.h>
#include "screens.h"
#include "gfx/gfx.h"
#include <keypadc.h>

int main(void) {
	// Initialize graphics drawing
	gfx_Begin();
	gfx_SetDrawBuffer();

	// Set the palette for sprites
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);

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
