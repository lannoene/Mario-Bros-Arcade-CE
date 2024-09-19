#include "draw.h"

#include <compression.h>
#include <math.h>
#include <string.h>

#include "gfx/gfx.h"

#include "platforms.h"
#include "enemies.h"
#include "pipes.h"
#include "hud.h"
#include "pow.h"
#include "bonus.h"
#include "level.h"
#include "fireballs.h"
#include "icicles.h"
#include "particles.h"
#include "defines.h"

#define SET_OLD_TO_NEW_COORDS(varName) \
(varName)->x_old = (varName)->x; (varName)->y_old = (varName)->y;

gfx_rletsprite_t* mario_sprites[2][8] = {
	{
		stand_right,
		walk1_right,
		walk2_right,
		walk3_right,
		jump_right,
		dead,
		slide_right,
		blank_stare
	},
	{
		0,
		0,
		0,
		0,
		0,
		dead,
		slide_left,
		blank_stare
	}
};

gfx_sprite_t* platform_bump_sprites[][3] = {
	{
		level1_block_bump1,
		level1_block_bump2,
		level1_block_bump3
	},
	{
		lava_block_bump1,
		lava_block_bump2,
		lava_block_bump3
	},
	{
		castle_block_bump1,
		castle_block_bump2,
		castle_block_bump3
	},
	{
		snowy_normal_block_bump1,
		snowy_normal_block_bump2,
		snowy_normal_block_bump3
	},
	{
		snowy_iced_block_bump1,
		snowy_iced_block_bump2,
		snowy_iced_block_bump3
	}
};

gfx_rletsprite_t* enemy_sprites[][2][8] = {
	{
		{
			spike_walk1_right,
			spike_walk2_right,
			spike_walk3_right,
			spike_upsidedown1_right
		},
		{
		}
	},
	{
		{
			crab_walk1_right,
			crab_walk2_right,
			crab_walk3_right,
			crab_upsidedown1_right,
			crab_walk1_mad_right,
			crab_walk2_mad_right,
			crab_walk3_mad_right
		},
	},
	{
		{
			fly_ground,
			fly_wing1,
			fly_wing2,
			fly_dead_right
		},
		{
			fly_ground,
			fly_wing1,
			fly_wing2,
		}
	},
	{
		{
			freezie_walk1_right,
			freezie_walk2_right,
			freezie_walk3_right,
			freezie_die1_right,
			freezie_die2_right, 
			freezie_die3_right,
			freezie_die4_right,
			freezie_die5_right
		},
	},
	{
		{
			coin1,
			coin2,
			coin3,
			coin4,
			coin5
		},
		{
			coin1,
			coin2,
			coin3,
			coin4,
			coin5
		}
	}
};

gfx_rletsprite_t* pow_sprites[3] = {
	pow_full,
	pow_medium,
	pow_low
};

gfx_rletsprite_t* pipe_sprites[2][1] = {
	{pipe_stationary_right},
	{0}
};

gfx_rletsprite_t* fireball_sprites[2][8] = {
	{
		fireball_green_big_rot1,
		fireball_green_big_rot2,
		fireball_green_big_rot3,
		fireball_green_big_rot4,
		fireball_green_small_rot1,
		fireball_green_small_rot2,
		fireball_green_small_rot3,
		fireball_green_small_rot4
	},
	{
		fireball_red_big_rot1,
		fireball_red_big_rot2,
		fireball_red_big_rot3,
		fireball_red_big_rot4,
		fireball_red_small_rot1,
		fireball_red_small_rot2,
		fireball_red_small_rot3,
		fireball_red_small_rot4
	}
};

gfx_rletsprite_t* icicle_sprites[6] = {
	icicle_forming1,
	icicle_forming2,
	icicle_forming3,
	icicle_full1,
	icicle_full2,
	icicle_full3
};

gfx_rletsprite_t* particle_sprites[] = {
	dust_cloud_big,
	dust_cloud_medium, 
	dust_cloud_small, 
	star_hit, 
	score_single, 
	score_double,
	score_triple, 
	score_quad, 
	score_1up,
	coin_pick_1,
	coin_pick_2,
	coin_pick_3,
	coin_pick_4,
	coin_pick_5
};

gfx_rletsprite_t* respawn_platforms[3] = {
	respawn_platform_full,
	respawn_platform_med,
	respawn_platform_empty,
};

uint8_t platform_bump_sprite_sheet[5] = {0, 1, 2, 1, 0};

void DrawScene(player_t *player, uint8_t backgroundType, unsigned int gameFrame) {
	uint8_t i;
	// get bgs under sprites before 
	// player
	for (i = 0; i < game_data.numPlayers; i++) {
		gfx_GetSprite((gfx_sprite_t*)player[i].backgroundData, FIXED_POINT_TO_INT(player[i].x_old), FIXED_POINT_TO_INT(player[i].y_old) + player[i].verSpriteOffset_old);
		if (player[i].state == PLAYER_RESPAWNING) {
			gfx_GetSprite((gfx_sprite_t*)player[i].respawnPlatformBgData, FIXED_POINT_TO_INT(player[i].x), FIXED_POINT_TO_INT(player[i].y) + PLAYER_HEIGHT);
		}
	}
	
	// enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].x_old) + levelEnemies.enemyArray[i].horSpriteOffset_old, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].y_old) + levelEnemies.enemyArray[i].verSpriteOffset_old);
	}
	
	// hud
	HudGetBackground(&player[0]);
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelPows.powArray[i].backgroundData, FIXED_POINT_TO_INT(levelPows.powArray[i].x), FIXED_POINT_TO_INT(levelPows.powArray[i].y));
	}
	
	// fireballs
	for (i = 0; i < levelFireballs.numFireballs; i++) {
		if (levelFireballs.fireballArray[i].alive)
			gfx_GetSprite((gfx_sprite_t*)levelFireballs.fireballArray[i].backgroundData, levelFireballs.fireballArray[i].x_old, levelFireballs.fireballArray[i].y_old);
	}
	
	// icicles
	for (i = 0; i < levelIcicles.numIcicles; i++) {
		if (levelIcicles.icicleArray[i].state != ICICLE_DEAD)
			gfx_GetSprite((gfx_sprite_t*)levelIcicles.icicleArray[i].backgroundData, levelIcicles.icicleArray[i].x_old, levelIcicles.icicleArray[i].y_old);
	}
	
	// particles
	for (i = 0; i < levelParticles.numParticles; i++) {
		if (levelParticles.particleArray[i].alive)
			gfx_GetSprite((gfx_sprite_t*)levelParticles.particleArray[i].backgroundData, levelParticles.particleArray[i].x_old, levelParticles.particleArray[i].y_old);
	}
	
	// draw sprites over bgs
	// draw platforms
	// platforms are pre rendered, and a mask is put under the bump when it is needed
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		platform_t* platform = &levelPlatforms.platformArray[i];
		uint16_t platFxToIntX = FIXED_POINT_TO_INT(platform->x);
		uint8_t platFxToIntY = FIXED_POINT_TO_INT(platform->y);
		uint8_t platFxToIntWidth = FIXED_POINT_TO_INT(platform->width);
		if (platform->invisible)
			continue;
		if (platform->needsRefresh || (gameFrame - game_data.levelStartTime) == 1) { // if gameFrame is 1 because we draw the platform once on frame 0, but we need to draw it on the other buffer on frame 1
			gfx_Sprite((gfx_sprite_t*)platform->backgroundData, platFxToIntX, platFxToIntY - PLATFORM_HEIGHT);
			gfx_Sprite_NoClip((gfx_sprite_t*)platform->processedTileImage, platFxToIntX, platFxToIntY);
		}
		if (!platform->beingBumped) {// if the platform isn't being bumped, show every tile
			continue;
		} else if (platform->beingBumped) {
			
			int16_t flooredXpos = ((platform->bumpedTileXpos - PLAYER_WIDTH/4)/BLOCK_SIZE)*BLOCK_SIZE; // integers get floored automatically
			// get bg around bump
			uint8_t bgFill[BLOCK_SIZE*3*BLOCK_SIZE + 2]; // only fill bottom row with bg because top row always starts cleared
			bgFill[0] = BLOCK_SIZE*3;
			bgFill[1] = BLOCK_SIZE;
			gfx_GetSprite((gfx_sprite_t*)bgFill, flooredXpos, platFxToIntY);
			gfx_Sprite_NoClip((gfx_sprite_t*)platform->processedTileImage, platFxToIntX, platFxToIntY);
			gfx_Sprite((gfx_sprite_t*)bgFill, flooredXpos, platFxToIntY); // fill in 3 blocks of platform that you bump
			
			if (flooredXpos - platFxToIntX >= platFxToIntWidth - 2*BLOCK_SIZE) { // if we are near the edge, floor the block's x pos to two blocks behind the edge, then cutoff animation at the edge
				bgFill[0] = BLOCK_SIZE;
				bgFill[1] = BLOCK_SIZE*3;
				gfx_GetSprite((gfx_sprite_t*)bgFill, platFxToIntX + platFxToIntWidth, platFxToIntY - BLOCK_SIZE);
				gfx_TransparentSprite(platform_bump_sprites[(platform->icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - platform->timeOfLastBump)/4 % 5]], platFxToIntX + platFxToIntWidth - (2*BLOCK_SIZE), platFxToIntY - BLOCK_SIZE);
				gfx_Sprite((gfx_sprite_t*)bgFill, platFxToIntX + platFxToIntWidth, platFxToIntY - BLOCK_SIZE);
			} else if (flooredXpos - platFxToIntX < 0) {
				bgFill[0] = BLOCK_SIZE;
				bgFill[1] = BLOCK_SIZE*3;
				gfx_GetSprite((gfx_sprite_t*)bgFill, platFxToIntX - BLOCK_SIZE, platFxToIntY - BLOCK_SIZE);
				gfx_TransparentSprite(platform_bump_sprites[(platform->icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - platform->timeOfLastBump)/4 % 5]], platFxToIntX - BLOCK_SIZE, platFxToIntY - BLOCK_SIZE);
				gfx_Sprite((gfx_sprite_t*)bgFill, platFxToIntX - BLOCK_SIZE, platFxToIntY - BLOCK_SIZE);
			} else { // otherwise, play animation as normal
				gfx_TransparentSprite(platform_bump_sprites[(platform->icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - platform->timeOfLastBump)/4 % 5]], flooredXpos, platFxToIntY - BLOCK_SIZE);
			}
			
		}
	}
	
	// draw particles
	for (i = 0; i < levelParticles.numParticles; i++) {
		if (levelParticles.particleArray[i].alive)
			gfx_RLETSprite(particle_sprites[levelParticles.particleArray[i].sprite], levelParticles.particleArray[i].x, levelParticles.particleArray[i].y);
	}
	
	// draw enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_RLETSprite(enemy_sprites[levelEnemies.enemyArray[i].type][levelEnemies.enemyArray[i].dir][levelEnemies.enemyArray[i].sprite], FP2I(levelEnemies.enemyArray[i].x) + levelEnemies.enemyArray[i].horSpriteOffset, FP2I(levelEnemies.enemyArray[i].y) + levelEnemies.enemyArray[i].verSpriteOffset);
	}
	
	// make sure pipes draw over enemies & coins
	for (i = 0; i < NUM_OF_PIPES; i++) {
		if (pipes[i].redraw) {
			gfx_RLETSprite((gfx_rletsprite_t*)pipe_sprites[pipes[i].dir][pipes[i].sprite], pipes[i].x, pipes[i].y);
			if (gameFrame - pipes[i].redrawTime == 2) // only redraw twice to fill both buffers
				pipes[i].redraw = false;
		}
	}
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		if (levelPows.powArray[i].state != POW_EMPTY)
			gfx_RLETSprite(pow_sprites[levelPows.powArray[i].state], FIXED_POINT_TO_INT(levelPows.powArray[i].x), FIXED_POINT_TO_INT(levelPows.powArray[i].y));
	}
	
	// fireballs
	for (i = 0; i < levelFireballs.numFireballs; i++) {
		if (levelFireballs.fireballArray[i].alive)
			gfx_RLETSprite(fireball_sprites[levelFireballs.fireballArray[i].type][levelFireballs.fireballArray[i].sprite], levelFireballs.fireballArray[i].x, levelFireballs.fireballArray[i].y);
	}
	
	// draw player over everything physical
	for (i = 0; i < game_data.numPlayers; i++) {
		gfx_RLETSprite(mario_sprites[player[i].dir][player[i].sprite], FIXED_POINT_TO_INT(player[i].x), FIXED_POINT_TO_INT(player[i].y) + player[i].verSpriteOffset);
		if (player[i].state == PLAYER_RESPAWNING) {
			gfx_RLETSprite(respawn_platforms[(((gameFrame - player[i].spawnTime < PLAYER_RESP_FALL_DURATION) ? 0 : gameFrame - player[i].spawnTime - PLAYER_RESP_FALL_DURATION)*3)/(PLAYER_RESP_WAIT_MAX)], FIXED_POINT_TO_INT(player[i].x), FIXED_POINT_TO_INT(player[i].y) + PLAYER_HEIGHT);
		}
	}
	
	// icicles
	for (i = 0; i < levelIcicles.numIcicles; i++) {
		if (levelIcicles.icicleArray[i].state != ICICLE_DEAD)
			gfx_RLETSprite(icicle_sprites[levelIcicles.icicleArray[i].sprite], levelIcicles.icicleArray[i].x, levelIcicles.icicleArray[i].y);
	}
	
	// draw hud over everything
	HudDraw(&player[0], gameFrame);
	
	
	// finish drawing
	gfx_SwapDraw();
	
	// replace sprites with bgs in second layer
	
	// player
	for (i = 0; i < game_data.numPlayers; i++) {
		gfx_Sprite((gfx_sprite_t*)player[i].backgroundData, FIXED_POINT_TO_INT(player[i].x_old), FIXED_POINT_TO_INT(player[i].y_old) + player[i].verSpriteOffset_old);
		if (player[i].state == PLAYER_RESPAWNING) {
			gfx_Sprite((gfx_sprite_t*)player[i].respawnPlatformBgData, FIXED_POINT_TO_INT(player[i].x), FIXED_POINT_TO_INT(player[i].y) + PLAYER_HEIGHT);
		}
		SET_OLD_TO_NEW_COORDS(&player[i]);
		player[i].verSpriteOffset_old = player[i].verSpriteOffset; 
	}
	
	// platforms
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 == 5 && !levelPlatforms.platformArray[i].invisible) {
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].y) - PLATFORM_HEIGHT);
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].x), FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].y));
		}
		if (levelPlatforms.platformArray[i].beingBumped) {
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].x_old), FIXED_POINT_TO_INT(levelPlatforms.platformArray[i].y_old) - PLATFORM_HEIGHT);
			SET_OLD_TO_NEW_COORDS(&levelPlatforms.platformArray[i]);
			if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 > 4) {
				levelPlatforms.platformArray[i].beingBumped = false;
				levelPlatforms.platformArray[i].needsRefresh = true;
			}
		} else if (levelPlatforms.platformArray[i].needsRefresh) {
			if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 > 5) {
				levelPlatforms.platformArray[i].needsRefresh = false;
				//gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y - PLATFORM_HEIGHT);
				//gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y);
			}
		}
	}
	
	// enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_Sprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, FP2I(levelEnemies.enemyArray[i].x_old) + levelEnemies.enemyArray[i].horSpriteOffset_old, FP2I(levelEnemies.enemyArray[i].y_old) + levelEnemies.enemyArray[i].verSpriteOffset_old);
		SET_OLD_TO_NEW_COORDS(&levelEnemies.enemyArray[i]);
		levelEnemies.enemyArray[i].verSpriteOffset_old = levelEnemies.enemyArray[i].verSpriteOffset;
		levelEnemies.enemyArray[i].horSpriteOffset_old = levelEnemies.enemyArray[i].horSpriteOffset;
	}
	
	// pipes
	for (i = 0; i < NUM_OF_PIPES; i++) {
		if (pipes[i].redraw) {
			//gfx_Sprite_NoClip((gfx_sprite_t*)pipes[i].backgroundData, pipes[i].x, pipes[i].y);
		}
	}
	
	// hud
	HudRefresh(&player[0], gameFrame);
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		gfx_Sprite_NoClip((gfx_sprite_t*)levelPows.powArray[i].backgroundData, FP2I(levelPows.powArray[i].x), FP2I(levelPows.powArray[i].y));
	}
	
	for (i = 0; i < levelFireballs.numFireballs; i++) {
		if (levelFireballs.fireballArray[i].alive) {
			gfx_Sprite((gfx_sprite_t*)levelFireballs.fireballArray[i].backgroundData, levelFireballs.fireballArray[i].x_old, levelFireballs.fireballArray[i].y_old);
			SET_OLD_TO_NEW_COORDS(&levelFireballs.fireballArray[i]);
		}
	}
	
	// icicles
	for (i = 0; i < levelIcicles.numIcicles; i++) {
		if (levelIcicles.icicleArray[i].state != ICICLE_DEAD) {
			gfx_Sprite((gfx_sprite_t*)levelIcicles.icicleArray[i].backgroundData, levelIcicles.icicleArray[i].x_old, levelIcicles.icicleArray[i].y_old);
			SET_OLD_TO_NEW_COORDS(&levelIcicles.icicleArray[i]);
		}
	}
	
	for (i = 0; i < levelParticles.numParticles; i++) {
		if (levelParticles.particleArray[i].alive) {
			gfx_Sprite((gfx_sprite_t*)levelParticles.particleArray[i].backgroundData, levelParticles.particleArray[i].x_old, levelParticles.particleArray[i].y_old);
			SET_OLD_TO_NEW_COORDS(&levelParticles.particleArray[i]);
		}
	}
}

void DrawBackground(uint8_t backgroundId) {
	// draw bg in buffer
	switch (backgroundId) {
		case BG_PIPES:
			zx0_Decompress(gfx_vbuffer, bg_pipes_compressed);
			break;
		case BG_LAVA:
			zx0_Decompress(gfx_vbuffer, bg_lava_compressed);
			break;
		case BG_CASTLE:
			zx0_Decompress(gfx_vbuffer, bg_castle_compressed);
			break;
		case BG_SNOWY:
			zx0_Decompress(gfx_vbuffer, bg_snowy_compressed);
			break;
	}
	
	for (uint8_t i = 0; i < 20; i++)
		gfx_TransparentSprite(pipes_ground, i*level1_ground_width, GROUND_HEIGHT);
	
	// copy it to the screen
	gfx_SetDrawScreen();
	gfx_BlitBuffer();
	// go back to offscreen buffer
	gfx_SetDrawBuffer();
}

void DrawPhaseCard(int16_t x, uint8_t y) {
	gfx_RLETSprite_NoClip(phase_card, x, y);
}

void GetPhaseCardBackground(int16_t x, uint8_t y) {
	
}

void DecompressSprites(void) {
	
}

uint16_t paletteDark[8][256] = {0};

void SetUpPalettes(void) {
	for (uint8_t i = 0; i < 8; i++) {
		for (uint16_t j = 0; j < sizeof_global_palette; j++) {
			paletteDark[i][j] = gfx_Darken(gfx_palette[j], 255 - (i)*32);
		}
	}
}

void SetDarkness(uint8_t darkness) {
	if (darkness < 8) {
		gfx_SetPalette(paletteDark[darkness], sizeof_global_palette, 0);
	} else { // turn screen black
		uint16_t zeroArray[256] = {0};
		gfx_SetPalette(zeroArray, sizeof_global_palette, 0);
	}
}

#define RLET_SIZE_MAX(x) ((((char*)x)[0] + 1) * ((char*)x)[1] * 3 / 2)
#define SPRITE_SIZE_MAX(x) (((((char*)x)[0]) * ((char*)x)[1]) + 2)

#define FLIP_SPRITE_SIZE_MAX (500 + 2)

int rlet_size_necessary(const gfx_rletsprite_t *image) {
	const uint8_t *_img = (const uint8_t*)image;
	int offset = 2;
	uint8_t width = _img[0], height = _img[1];
	for (unsigned int i = 0; i < height; i++) {
        int left = width;
		while (left) {
			// get transparent pixels counter
			left -= _img[offset++];
			if (!left)
				break;
			// get num opaque pixels
			uint8_t np = _img[offset++];
			left -= np;
			offset += np;
		}
	}
	return offset;
}

int rlet_size_necessary_flip(const gfx_rletsprite_t *image) {
    const uint8_t *_img = (const uint8_t*)image;
    int offset = 2, adjust = 0;
    uint8_t width = _img[0], height = _img[1];
	
    for (uint8_t i = 0; i < height; i++) {
        uint8_t left = width;
        if (_img[offset])
            adjust++;
        while (left) {
            // get transparent pixels counter
            left -= _img[offset++];
            if (!left) {
                adjust--;
                break;
            }
            // get num opaque pixels
            uint8_t np = _img[offset++];
            left -= np;
            offset += np;
        }
    }
    return offset + adjust;
}

int rlet_sprite_flipY(gfx_rletsprite_t *image, gfx_rletsprite_t *out) {
	#define OPP_OFF ((offset) - ((offset) % width))
	const uint8_t *_img = (const uint8_t*)image;
	int offset = 2;
	uint8_t width = _img[0], height = _img[1];
	for (unsigned int i = 0; i < height; i++) {
        int left = width;
		while (left) {
			// get transparent pixels counter
			left -= _img[offset++];
			if (!left)
				break;
			// get num opaque pixels
			uint8_t np = _img[offset++];
			left -= np;
			offset += np;
		}
	}
    return 0;
}

// we can only malloc the return sprite because otherwise it would cause heap fragmentation
__attribute__((noinline))
static void FlipRletSpriteY(const void *in, gfx_rletsprite_t *out) {
	const int spriteSize = SPRITE_SIZE_MAX(in);
	gfx_sprite_t *_tempSprite = malloc(spriteSize), *_flipTempSprite = malloc(spriteSize);
	
	gfx_ConvertFromRLETSprite(in, _tempSprite);
	// flip across the y axis
	gfx_FlipSpriteY(_tempSprite, _flipTempSprite);
	free(_tempSprite);
	
	gfx_ConvertToRLETSprite(_flipTempSprite, out);
	free(_flipTempSprite);
	
	
	//memcpy(*out, _out, rlet_size_necessary(_out));
	//free(_out);
}

#define FREE_SPRITE_ARR(arr, max)\
do {\
for (int __i = 0; __i < max; __i++)\
	if (arr[__i] != NULL)\
		free(arr[__i]);\
} while(0)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

void *s_alloc(int s) { // statically allocate something on .data
	static uint8_t _s_heap[7500];
	static unsigned int nAlloc = 0;
	
	if (nAlloc + s > sizeof(_s_heap))
		return NULL;
	void *ret = &_s_heap[nAlloc];
	nAlloc += s; // reserve space
	return ret;
}

typedef struct {
	void **in;
	void **out;
	int num;
} convSprite_t;
convSprite_t convSprites[] = {
	{enemy_sprites[0][RIGHT], enemy_sprites[0][LEFT], 8},
	{enemy_sprites[1][RIGHT], enemy_sprites[1][LEFT], 8},
	{&enemy_sprites[2][RIGHT][3], &enemy_sprites[2][LEFT][3], 1},
	{enemy_sprites[3][RIGHT], enemy_sprites[3][LEFT], 8},
	{mario_sprites[RIGHT], mario_sprites[LEFT], 5},
	{pipe_sprites[RIGHT], pipe_sprites[LEFT], 1},
};

void LoadExtraSprites(void) {
	for (size_t i = 0; i < sizeof(convSprites)/sizeof(convSprite_t); i++) {
		for (int j = 0; j < convSprites[i].num; j++) {
			if (convSprites[i].in[j] != NULL) {
				int s = rlet_size_necessary_flip(convSprites[i].in[j]);
				convSprites[i].out[j] = s_alloc(s);
				FlipRletSpriteY(convSprites[i].in[j], convSprites[i].out[j]);
			}
		}
	}
}

void FreeExtraSprites(void) {
	for (size_t i = 0; i < sizeof(convSprites)/sizeof(convSprite_t); i++) {
		FREE_SPRITE_ARR(convSprites[i].out, convSprites[i].num);
	}
}
#pragma clang diagnostic pop