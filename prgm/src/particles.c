#include "particles.h"

#include <stdlib.h>

#include "gfx/gfx.h"
#include "level.h"

#define PARTICLE_LOOKUP_WIDTH	0
#define PARTICLE_LOOKUP_HEIGHT	1
#define PARTICLE_LOOKUP_LAYER	2

levelParticles_t levelParticles;

uint8_t particleLookupTable[][3] = {
	{7, 6, 0}, // star
	{dust_cloud_big_width, dust_cloud_big_height, 0}, // dust cloud big
	{dust_cloud_medium_width, dust_cloud_medium_height, 0}, // dust cloud medium
	{3, 3, 0}, // dust cloud small
	{score_single_width, score_single_height, 1}, // score
	{score_double_width, score_double_height, 1}, // score
	{score_triple_width, score_triple_height, 1}, // score
	{score_quad_width, score_quad_height, 1}, // score
	{score_1up_width, score_1up_height, 1}, // score
	{11, 11, 1}, // coin particle width (all of them)
};

static inline void KillParticle(uint8_t particleIndex);

void InitParticles(void) {
	levelParticles.numParticles = 0;
	levelParticles.particleArray = malloc(0);
}

void SpawnParticle(int16_t x, uint8_t y, uint8_t type, unsigned int gameFrame) {
	uint8_t i;
	for (i = 0; i < levelParticles.numParticles; i++) {
		if (!levelParticles.particleArray[i].alive)
			break;
	}
	if (i == levelParticles.numParticles) {
		++levelParticles.numParticles;
		levelParticles.particleArray = realloc(levelParticles.particleArray, levelParticles.numParticles*sizeof(particle_t));
	}
	
	levelParticles.particleArray[i].alive = true;
	levelParticles.particleArray[i].shouldDie = false;
	levelParticles.particleArray[i].x = levelParticles.particleArray[i].x_old = x;
	levelParticles.particleArray[i].y = levelParticles.particleArray[i].y_old = y;
	levelParticles.particleArray[i].type = type;
	levelParticles.particleArray[i].width = particleLookupTable[type][PARTICLE_LOOKUP_WIDTH];
	levelParticles.particleArray[i].height = particleLookupTable[type][PARTICLE_LOOKUP_HEIGHT];
	levelParticles.particleArray[i].layer = particleLookupTable[type][PARTICLE_LOOKUP_LAYER];
	levelParticles.particleArray[i].spawnTime = gameFrame;
	levelParticles.particleArray[i].backgroundData = malloc(levelParticles.particleArray[i].width*levelParticles.particleArray[i].height + 2);
	levelParticles.particleArray[i].backgroundData[0] = levelParticles.particleArray[i].width;
	levelParticles.particleArray[i].backgroundData[1] = levelParticles.particleArray[i].height;
	switch (levelParticles.particleArray[i].type) {
		case PARTICLE_DUST:
			levelParticles.particleArray[i].sprite = 0;
			break;
		case PARTICLE_SCORE_REG:
			levelParticles.particleArray[i].sprite = 4;
			break;
		case PARTICLE_SCORE_DUB:
			levelParticles.particleArray[i].sprite = 5;
			break;
		case PARTICLE_SCORE_TRP:
			levelParticles.particleArray[i].sprite = 6;
			break;
		case PARTICLE_SCORE_QDP:
			levelParticles.particleArray[i].sprite = 7;
			break;
		case PARTICLE_SCORE_1UP:
			levelParticles.particleArray[i].sprite = 8;
			break;
		case PARTICLE_COIN_PICK:
			levelParticles.particleArray[i].sprite = 10;
			break;
	}
}

void UpdateParticles(unsigned int gameFrame) {
	for (int i = 0; i < levelParticles.numParticles; i++) {
		particle_t* particle = &levelParticles.particleArray[i];
		if (!particle->alive)
			continue;
		if (particle->shouldDie) {
			KillParticle(i);
			continue;
		}
		switch (particle->type) {
			case PARTICLE_DUST:
				if (gameFrame - particle->spawnTime == 18) {
					KillParticle(i);
				} else if (gameFrame - particle->spawnTime == 14) {
					particle->sprite = 2;
					particle->y += 2;
				} else if (gameFrame - particle->spawnTime == 9) {
					particle->sprite = 1;
					particle->y += 1;
				}
				break;
			case PARTICLE_SCORE_REG:
			case PARTICLE_SCORE_DUB:
			case PARTICLE_SCORE_TRP:
			case PARTICLE_SCORE_QDP:
			case PARTICLE_SCORE_1UP:
				if (gameFrame % 3 == 0)
					--particle->y;
				if (gameFrame - particle->spawnTime > 40)
					KillParticle(i);
				break;
			case PARTICLE_COIN_PICK:
				if (gameFrame - particle->spawnTime == 42) {
					if (!game_data.isBonusLevel)
						SpawnParticle(particle->x, particle->y, PARTICLE_SCORE_REG, gameFrame);
					KillParticle(i);
					continue;
				} else if (gameFrame - particle->spawnTime == 32) {
					particle->sprite = 13;
				} else if (gameFrame - particle->spawnTime == 24) {
					particle->sprite = 12;
				} else if (gameFrame - particle->spawnTime == 16) {
					particle->sprite = 11;
				} else if (gameFrame - particle->spawnTime == 8) {
					particle->sprite = 10;
				}
				if (gameFrame % 3 == 0)
					--particle->y;
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
		free(levelParticles.particleArray[particleIndex].backgroundData);
	}
}

void FreeParticles(void) {
	for (int i = 0; i < levelParticles.numParticles; i++) {
		if (levelParticles.particleArray[i].alive)
			KillParticle(i); // don't want to free already free'd particles
	}
	free(levelParticles.particleArray);
}

void CleanUpParticleArray(void) {
	int i = 0;
	for (int i = 0; i < levelParticles.numParticles; i++) {
		if (levelParticles.particleArray[i].alive)
			break;
	}
	if (i == levelParticles.numParticles)
		levelParticles.numParticles = 0;
}

void ResetParticleArray(void) {
	for (int i = 0; i < levelParticles.numParticles; i++) {
		if (levelParticles.particleArray[i].alive)
			KillParticle(i);
	}
	levelParticles.numParticles = 0;
}