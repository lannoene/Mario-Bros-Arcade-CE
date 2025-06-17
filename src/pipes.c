#include "pipes.h"

#include <graphx.h>

#include "player.h"
#include "defines.h"
#include "enemies.h"

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

static struct pQueue {
	enemy_t *slots[30];
	uint8_t len;
} pipeQueue[2];

void AddEnemyToPipeQueue(enemy_t *enemy, uint8_t pipe) {
	FOR_EACH(pipeQueue[pipe].slots, ARR_LEN(pipeQueue[pipe].slots), slot) {
		if (!*slot) {
			*slot = enemy;
			break;
		}
	} FOR_ELSE {
		dbg_printf("There are no slots open in pipe queue.\n");
	}
}

enemy_t *PopEnemyFromPipeQueue(uint8_t pipe) {
	struct pQueue *pq = &pipeQueue[pipe];
	enemy_t *result = pq->slots[0];
	// shift everything over
	for (uint8_t i = 0; i < ARR_LEN(pq->slots) - 1; i++) {
		pq->slots[i] = pq->slots[i + 1];
	}
	// replace final with null
	pq->slots[ARR_LEN(pq->slots) - 1] = NULL;
	return result;
}