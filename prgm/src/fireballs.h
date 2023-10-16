#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "player.h"

#define FIREBALL_SIZE	16

// fireball types
#define FIREBALL_GREEN	0
#define FIREBALL_RED	1

typedef struct {
	int16_t x, x_old;
	uint8_t y, y_old, original_y;
	uint8_t type, verDir, horDir, sprite, state : 4;
	bool alive;
	unsigned int spawnTime;
	uint8_t backgroundData[FIREBALL_SIZE*FIREBALL_SIZE + 2];
} fireball_t;

typedef struct {
	uint8_t numFireballs;
	fireball_t* fireballArray;
	int fireballSpawnWeight;
} levelFireballInfo_t;

enum fireball_states {
	FIREBALL_MOVING,
	FIREBALL_SPAWNING,
	FIREBALL_DESPAWNING
};

enum fireball_settings {
	HAS_FIREBALL_GREEN = 0x1,
	HAS_FIREBALL_RED = 0x2
};

extern levelFireballInfo_t levelFireballs;

void InitFireballs(void);
void CreateFireball(uint8_t y, bool dir, uint8_t type, unsigned int gameFrame);
void UpdateFireballs(player_t* player, unsigned int gameFrame);
void FreeFireballs(void);
void ManageFireballSpawning(player_t* player, unsigned int gameFrame);
void ResetFireballs(void);