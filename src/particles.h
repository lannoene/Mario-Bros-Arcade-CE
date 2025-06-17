#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint16_t x, x_old;
	uint8_t y, y_old;
	uint8_t type, width, height, layer, sprite; // layer: 0 = behind everything, 1 in front
	bool alive, shouldDie : 1;
	unsigned int spawnTime;
	uint8_t* backgroundData;
	uint8_t opt1; // misc arguments
} particle_t;

typedef struct {
	particle_t* particleArray;
	uint8_t numParticles, numAllocated;
} levelParticles_t;

enum PARTICLE_IDS {
	PARTICLE_STAR = 0,
	PARTICLE_DUST,
	PARTICLE_SCORE_800 = 4, // lul idk
	PARTICLE_SCORE_DUB,
	PARTICLE_SCORE_TRP,
	PARTICLE_SCORE_QDP,
	PARTICLE_SCORE_1UP,
	PARTICLE_COIN_PICK,
	PARTICLE_SCORE_500,
};

extern levelParticles_t levelParticles;

void InitParticles(void);
particle_t *SpawnParticle(int16_t x, uint8_t y, uint8_t type, unsigned int gameFrame);
void FreeParticles(void);
void UpdateParticles(unsigned int gameFrame);
void CleanUpParticleArray(void);
void ResetParticleArray(void);