#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_PIPES 2

typedef struct {
	bool dir, redraw : 1;
	int16_t x;
	uint8_t y;
	uint8_t sprite, sprite_old;
	unsigned int redrawTime;
	uint8_t backgroundData[64*38 + 2]; // img width = 64, img height = 32
} pipe_t;

extern pipe_t pipes[NUM_OF_PIPES];

typedef struct enemy enemy_t;

void InitPipeBackgroundData(void);
void RedrawPipesWithNewSprite(uint8_t pipeIndex, uint8_t newSprite, unsigned int gameFrame);
void AddEnemyToPipeQueue(enemy_t *enemy, uint8_t pipe);
enemy_t *PopEnemyFromPipeQueue(uint8_t pipe);