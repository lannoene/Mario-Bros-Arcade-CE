#include "pipes.h"

#include "graphx.h"

#include "player.h"

pipe_t pipes[NUM_OF_PIPES] = {{RIGHT, true, 256, 25, 0, 1, 0, {64, 38}}, {LEFT, true, 0, 25, 0, 1, 0, {64, 38}}};

void InitPipeBackgroundData(void) {
	for (uint8_t i = 0; i < NUM_OF_PIPES; i++) {
		gfx_GetSprite_NoClip((gfx_sprite_t*)pipes[i].backgroundData, pipes[i].x, pipes[i].y);
	}
}

void RedrawPipesWithNewSprite(uint8_t pipeIndex, uint8_t newSprite, unsigned int gameFrame) {
	pipes[pipeIndex].redraw = true;
	pipes[pipeIndex].redrawTime = gameFrame;
	pipes[pipeIndex].sprite = newSprite;
}