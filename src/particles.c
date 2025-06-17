#include "particles.h"

#include <stdlib.h>

#include "gfx/gfx.h"
#include "level.h"
#include "defines.h"

#define PARTICLE_LOOKUP_WIDTH	0
#define PARTICLE_LOOKUP_HEIGHT	1
#define PARTICLE_LOOKUP_LAYER	2

levelParticles_t levelParticles;
particle_t particleArray[30];

uint8_t particleLookupTable[][3] = {
	{star_hit_width, star_hit_height, 0}, // star
	{dust_cloud_big_width, dust_cloud_big_height, 0}, // dust cloud big
	{dust_cloud_medium_width, dust_cloud_medium_height, 0}, // dust cloud medium
	{3, 3, 0}, // dust cloud small
	{score_800_width, score_800_height, 1}, // score
	{score_double_width, score_double_height, 1}, // score
	{score_triple_width, score_triple_height, 1}, // score
	{score_quad_width, score_quad_height, 1}, // score
	{score_1up_width, score_1up_height, 1}, // score
	{11, 11, 1}, // coin particle width (all of them)
	{score_500_width, score_500_height, 1}
};

static inline void KillParticle(uint8_t particleIndex);

void InitParticles(void) {
	levelParticles.numParticles = 0;
	levelParticles.numAllocated = ARR_LEN(particleArray);
	levelParticles.particleArray = particleArray;
}

particle_t *SpawnParticle(int16_t x, uint8_t y, uint8_t type, unsigned int gameFrame) {
	particle_t *particle = NULL;
	// stuff goes here
	FOR_EACH(levelParticles.particleArray, levelParticles.numAllocated, _particle) {
		if (!_particle->alive) {
			particle = _particle;
			break;
		}
	} FOR_ELSE {
		dbg_printf("The maximum number of particles has been reached.\n");
		return NULL;
	}
	particle->alive = true;
	particle->shouldDie = false;
	particle->x = particle->x_old = x;
	particle->y = particle->y_old = y;
	particle->type = type;
	particle->width = particleLookupTable[type][PARTICLE_LOOKUP_WIDTH];
	particle->height = particleLookupTable[type][PARTICLE_LOOKUP_HEIGHT];
	particle->layer = particleLookupTable[type][PARTICLE_LOOKUP_LAYER];
	particle->spawnTime = gameFrame;
	particle->backgroundData = malloc(particle->width*particle->height + 2);
	if (!particle->backgroundData) {
		dbg_printf("Could not allocate particle background data.\n");
		particle->alive = false;
		return NULL;
	}
	particle->backgroundData[0] = particle->width;
	particle->backgroundData[1] = particle->height;
	switch (particle->type) {
		case PARTICLE_DUST:
			particle->sprite = 0;
			break;
		case PARTICLE_SCORE_800:
			particle->sprite = 4;
			break;
		case PARTICLE_SCORE_DUB:
			particle->sprite = 5;
			break;
		case PARTICLE_SCORE_TRP:
			particle->sprite = 6;
			break;
		case PARTICLE_SCORE_QDP:
			particle->sprite = 7;
			break;
		case PARTICLE_SCORE_1UP:
			particle->sprite = 8;
			break;
		case PARTICLE_COIN_PICK:
			particle->sprite = 10;
			break;
		case PARTICLE_STAR:
			particle->sprite = 3;
			break;
		case PARTICLE_SCORE_500:
			particle->sprite = 14;
			break;
	}
	levelParticles.numParticles++;
	return particle;
}

void UpdateParticles(unsigned int gameFrame) {
	for (int i = 0; i < levelParticles.numAllocated; i++) {
		particle_t *particle = &levelParticles.particleArray[i];
		if (!particle->alive)
			continue;
		if (particle->shouldDie) {
			KillParticle(i);
			continue;
		}
		unsigned int dtSpawn = gameFrame - particle->spawnTime;
		switch (particle->type) {
			case PARTICLE_DUST:
				if (dtSpawn == 18) {
					KillParticle(i);
				} else if (dtSpawn == 14) {
					particle->sprite = 2;
					particle->y += 2;
				} else if (dtSpawn == 9) {
					particle->sprite = 1;
					particle->y += 1;
				}
				break;
			case PARTICLE_SCORE_500:
			case PARTICLE_SCORE_800:
			case PARTICLE_SCORE_DUB:
			case PARTICLE_SCORE_TRP:
			case PARTICLE_SCORE_QDP:
			case PARTICLE_SCORE_1UP:
				if (gameFrame % 3 == 0)
					--particle->y;
				if (dtSpawn > 40)
					KillParticle(i);
				break;
			case PARTICLE_COIN_PICK:
				if (dtSpawn == 42) {
					if (particle->opt1)
						SpawnParticle(particle->x, particle->y, PARTICLE_SCORE_800, gameFrame);
					KillParticle(i);
					continue;
				} else if (dtSpawn == 32) {
					particle->sprite = 13;
				} else if (dtSpawn == 24) {
					particle->sprite = 12;
				} else if (dtSpawn == 16) {
					particle->sprite = 11;
				} else if (dtSpawn == 8) {
					particle->sprite = 10;
				}
				if (gameFrame % 3 == 0)
					--particle->y;
				break;
			case PARTICLE_STAR:
				if (dtSpawn == 11) {
					KillParticle(i);
					continue;
				}
				if (dtSpawn != 5 && dtSpawn != 7 && dtSpawn != 8 && dtSpawn != 10) {
					switch (particle->opt1) {
						case 0: // diagonal top right
							particle->x += 2;
							particle->y += 2;
							break;
						case 1: // diagonal top left
							particle->x -= 2;
							particle->y -= 2;
							break;
						case 2: // diagonal bottom right
							particle->x += 2;
							particle->y += 2;
							break;
						case 3: // diagonal bottom left
							particle->x -= 2;
							particle->y -= 2;
							break;
					}
				}
				break;
		}
	}
}

static inline void KillParticle(uint8_t particleIndex) {
	if (!levelParticles.particleArray[particleIndex].shouldDie) {
		levelParticles.particleArray[particleIndex].shouldDie = true;
		levelParticles.particleArray[particleIndex].y = 241; // offscreen so the bg can fill the place of the particle before being discarded
	} else {
		levelParticles.particleArray[particleIndex].alive = false;
		levelParticles.numParticles--;
		free(levelParticles.particleArray[particleIndex].backgroundData);
	}
}

// forcefully frees all particles
void FreeParticles(void) {
	for (int i = 0; i < levelParticles.numAllocated; i++) {
		if (levelParticles.particleArray[i].alive) // don't want to free already free'd particles
			free(levelParticles.particleArray[i].backgroundData);
	}
	//free(levelParticles.particleArray);
	levelParticles.numParticles = 0;
}

void CleanUpParticleArray(void) {
	int i = 0;
	for (int i = 0; i < levelParticles.numAllocated; i++) {
		if (levelParticles.particleArray[i].alive)
			break;
	}
	if (i == levelParticles.numAllocated)
		levelParticles.numParticles = 0;
}

void ResetParticleArray(void) {
	for (int i = 0; i < levelParticles.numAllocated; i++) {
		if (levelParticles.particleArray[i].alive)
			KillParticle(i);
	}
	levelParticles.numParticles = 0;
}