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

gfx_rletsprite_t* mario_sprites[2][2][8] = {
	{ // mario
		{
			red_stand_right,
			red_walk1_right,
			red_walk2_right,
			mario_placeholder,
			red_jump_right,
			red_dead,
			red_slide_right,
			red_blank_stare
		},
		{
			0,
			0,
			0,
			0,
			0,
			red_dead,
			0,
			red_blank_stare
		}
	},
	{ // luigi
		{
			grn_stand_right,
			grn_walk1_right,
			grn_walk2_right,
			mario_placeholder,
			grn_jump_right,
			grn_dead,
			grn_slide_right,
			grn_blank_stare
		},
		{
			0,
			0,
			0,
			0,
			0,
			grn_dead,
			0,
			grn_blank_stare
		}
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
	score_800, 
	score_double,
	score_triple, 
	score_quad, 
	score_1up,
	coin_pick_1,
	coin_pick_2,
	coin_pick_3,
	coin_pick_4,
	coin_pick_5,
	score_500,
};

gfx_rletsprite_t* respawn_platforms[3] = {
	respawn_platform_full,
	respawn_platform_med,
	respawn_platform_empty,
};

uint8_t platform_bump_sprite_sheet[5] = {0, 1, 2, 1, 0};

static inline uint8_t EnemyIdToSpriteLoc(int type) {
	switch (type) {
		default: // fallthrough
		case ENEMY_SPIKE: return 0;
		case ENEMY_CRAB: return 1;
		case ENEMY_FLY: return 2;
		case ENEMY_FREEZIE: return 3;
		case ENEMY_COIN: return 4;
	}
}

void DrawScene(player_t *player, uint8_t backgroundType, unsigned int gameFrame) {
	uint8_t i;
	
	FFOR_EACH(levelPlatforms.platformArray, levelPlatforms.numPlatforms, platform) {
		if (platform->invisible)
			continue;
		
		if (platform->needsRefresh || game_data.dtLevelStart == 1) {
			uint16_t platFxToIntX = FP2I(platform->x);
			uint8_t platFxToIntY = FP2I(platform->y);
			gfx_Sprite((gfx_sprite_t*)platform->backgroundData, platFxToIntX, platFxToIntY);
			if ((gameFrame - platform->timeOfLastBump)/4 == 6) {
				platform->needsRefresh = false;
			} else if ((gameFrame - platform->timeOfLastBump)/4 == 5) {
				gfx_Sprite((gfx_sprite_t*)platform->bumpBackgroundData, CalculatePlatformBumpDrawX(platform).x, platFxToIntY - BLOCK_SIZE);
			}
			gfx_Sprite_NoClip((gfx_sprite_t*)platform->processedTileImage, platFxToIntX, platFxToIntY);
		}
		if (!platform->beingBumped)
			continue;
		
		uint16_t platFxToIntX = FP2I(platform->x);
		uint8_t platFxToIntY = FP2I(platform->y);
		// get bg around bump
		platform->bumpBackgroundData[0] = BLOCK_SIZE*3;
		platform->bumpBackgroundData[1] = BLOCK_SIZE*2;
		gfx_Sprite((gfx_sprite_t*)platform->backgroundData, platFxToIntX, platFxToIntY);
		if ((gameFrame - platform->timeOfLastBump) == 0) // only get background before sprites are drawn
			gfx_GetSprite((gfx_sprite_t*)platform->bumpBackgroundData, CalculatePlatformBumpDrawX(platform).x, platFxToIntY - BLOCK_SIZE);
	}
	
	// get bgs under sprites before 
	// player
	for (i = 0; i < game_data.numPlayers; i++) {
		gfx_GetSprite((gfx_sprite_t*)player[i].backgroundData, FP2I(player[i].x_old), FP2I(player[i].y_old) + player[i].verSpriteOffset_old);
		if (player[i].state == PLAYER_RESPAWNING) {
			gfx_GetSprite((gfx_sprite_t*)player[i].respawnPlatformBgData, FP2I(player[i].x), FP2I(player[i].y) + PLAYER_HEIGHT);
		}
	}
	
	// enemies
	FFOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
		if (enemy->lastState != ENEMY_DEAD || enemy->state != ENEMY_DEAD)
			gfx_GetSprite((gfx_sprite_t*)enemy->backgroundData, FP2I(enemy->x_old) + enemy->horSpriteOffset_old, FP2I(enemy->y_old) + enemy->verSpriteOffset_old);
	}
	
	// hud
	HudGetBackground(&player[0]);
	
	// pows
	FFOR_EACH(levelPows.powArray, levelPows.numPows, pow) {
		gfx_GetSprite_NoClip((gfx_sprite_t*)pow->backgroundData, FP2I(pow->x), FP2I(pow->y));
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
	for (i = 0; i < levelParticles.numAllocated; i++) {
		if (levelParticles.particleArray[i].alive)
			gfx_GetSprite((gfx_sprite_t*)levelParticles.particleArray[i].backgroundData, levelParticles.particleArray[i].x_old, levelParticles.particleArray[i].y_old);
	}
	
	// draw sprites over bgs
	// draw platforms
	// platforms are pre rendered, and a mask is put under the bump when it is needed
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		platform_t* platform = &levelPlatforms.platformArray[i];
		if (platform->invisible || !platform->beingBumped)
			continue;
		uint16_t platFxToIntX = FP2I(platform->x);
		uint8_t platFxToIntY = FP2I(platform->y);
		uint8_t platFxToIntWidth = FP2I(platform->width);
		struct platform_bump_draw platformBDraw = CalculatePlatformBumpDrawX(platform);
		
		gfx_Sprite_NoClip((gfx_sprite_t*)platform->processedTileImage, platFxToIntX, platFxToIntY);
		gfx_Sprite((gfx_sprite_t*)platform->bumpBackgroundData, platformBDraw.x, platFxToIntY - BLOCK_SIZE); // fill in 3 blocks of platform that you bump
		
		uint8_t bgFill[BLOCK_SIZE*BLOCK_SIZE*3 + 2];
		bgFill[0] = BLOCK_SIZE;
		bgFill[1] = BLOCK_SIZE*3;
		uint16_t overflowMaskX = (platformBDraw.overflowLR == RIGHT) ? platFxToIntX + platFxToIntWidth : platFxToIntX - BLOCK_SIZE;
		if (platformBDraw.overflowLR != NONE)
			gfx_GetSprite((gfx_sprite_t*)bgFill, overflowMaskX, platFxToIntY - BLOCK_SIZE);
		gfx_TransparentSprite(platform_bump_sprites[(platform->icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - platform->timeOfLastBump)/4 % 5]], platformBDraw.x, platFxToIntY - BLOCK_SIZE);
		if (platformBDraw.overflowLR != NONE)
			gfx_Sprite((gfx_sprite_t*)bgFill, overflowMaskX, platFxToIntY - BLOCK_SIZE);
	}
	
	// draw particles (bottom layer)
	for (i = 0; i < levelParticles.numAllocated; i++) {
		if (levelParticles.particleArray[i].alive && !levelParticles.particleArray[i].layer)
			gfx_RLETSprite(particle_sprites[levelParticles.particleArray[i].sprite], levelParticles.particleArray[i].x, levelParticles.particleArray[i].y);
	}
	
	// draw enemies
	FFOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
		if (enemy->state != ENEMY_DEAD) {
			gfx_RLETSprite(
				enemy_sprites[EnemyIdToSpriteLoc(enemy->type)][enemy->dir][enemy->sprite],
				FP2I(enemy->x) + enemy->horSpriteOffset,
				FP2I(enemy->y) + enemy->verSpriteOffset
			);
		}
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
	FFOR_EACH(levelPows.powArray, levelPows.numPows, pow) {
		if (pow->state != POW_EMPTY)
			gfx_RLETSprite(pow_sprites[pow->state], FP2I(pow->x), FP2I(pow->y));
	}
	
	// fireballs
	for (i = 0; i < levelFireballs.numFireballs; i++) {
		if (levelFireballs.fireballArray[i].alive)
			gfx_RLETSprite(fireball_sprites[levelFireballs.fireballArray[i].type][levelFireballs.fireballArray[i].sprite], levelFireballs.fireballArray[i].x, levelFireballs.fireballArray[i].y);
	}
	
	// draw player over everything physical
	for (i = 0; i < game_data.numPlayers; i++) {
		gfx_RLETSprite(mario_sprites[i][player[i].dir][player[i].sprite], FP2I(player[i].x), FP2I(player[i].y) + player[i].verSpriteOffset);
		if (player[i].state == PLAYER_RESPAWNING) {
			gfx_RLETSprite(respawn_platforms[(((gameFrame - player[i].spawnTime < PLAYER_RESP_FALL_DURATION) ? 0 : gameFrame - player[i].spawnTime - PLAYER_RESP_FALL_DURATION)*3)/(PLAYER_RESP_WAIT_MAX)], FP2I(player[i].x), FP2I(player[i].y) + PLAYER_HEIGHT);
		}
	}
	
	// icicles
	for (i = 0; i < levelIcicles.numIcicles; i++) {
		if (levelIcicles.icicleArray[i].state != ICICLE_DEAD)
			gfx_RLETSprite(icicle_sprites[levelIcicles.icicleArray[i].sprite], levelIcicles.icicleArray[i].x, levelIcicles.icicleArray[i].y);
	}
	
	// draw particles (top layer)
	for (i = 0; i < levelParticles.numAllocated; i++) {
		if (levelParticles.particleArray[i].alive && levelParticles.particleArray[i].layer)
			gfx_RLETSprite(particle_sprites[levelParticles.particleArray[i].sprite], levelParticles.particleArray[i].x, levelParticles.particleArray[i].y);
	}
	
	// draw hud over everything
	HudDraw(&player[0], gameFrame);
	
	// finish drawing
	gfx_SwapDraw();
	
	// replace sprites with bgs in second layer
	
	// player
	for (i = 0; i < game_data.numPlayers; i++) {
		gfx_Sprite((gfx_sprite_t*)player[i].backgroundData, FP2I(player[i].x_old), FP2I(player[i].y_old) + player[i].verSpriteOffset_old);
		if (player[i].state == PLAYER_RESPAWNING) {
			gfx_Sprite((gfx_sprite_t*)player[i].respawnPlatformBgData, FP2I(player[i].x), FP2I(player[i].y) + PLAYER_HEIGHT);
		}
		SET_OLD_TO_NEW_COORDS(&player[i]);
		player[i].verSpriteOffset_old = player[i].verSpriteOffset; 
	}
	
	// enemies
	FFOR_EACH(levelEnemies.enemyArray, levelEnemies.numAllocated, enemy) {
		if (enemy->lastState != ENEMY_DEAD || enemy->state != ENEMY_DEAD) {
			gfx_Sprite(
				(gfx_sprite_t*)enemy->backgroundData,
				FP2I(enemy->x_old) + enemy->horSpriteOffset_old,
				FP2I(enemy->y_old) + enemy->verSpriteOffset_old
			);
			SET_OLD_TO_NEW_COORDS(enemy);
			enemy->verSpriteOffset_old = enemy->verSpriteOffset;
			enemy->horSpriteOffset_old = enemy->horSpriteOffset;
		}
	}
	
	// pipes
	/*for (i = 0; i < NUM_OF_PIPES; i++) {
		if (pipes[i].redraw) {
			gfx_Sprite_NoClip((gfx_sprite_t*)pipes[i].backgroundData, pipes[i].x, pipes[i].y);
		}
	}*/
	
	// hud
	HudRefresh(&player[0], gameFrame);
	
	// pows
	FFOR_EACH(levelPows.powArray, levelPows.numPows, pow) {
		gfx_Sprite_NoClip((gfx_sprite_t*)pow->backgroundData, FP2I(pow->x), FP2I(pow->y));
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
	
	// particles
	for (i = 0; i < levelParticles.numAllocated; i++) {
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

__attribute__((noinline))
static void FlipRletSpriteY(const void *in, gfx_rletsprite_t *out) {
	const int spriteSize = SPRITE_SIZE_MAX(in);
	// hijack lvl_s_alloc for our own uses before the level is created
	gfx_sprite_t *_tempSprite = lvl_s_alloc(spriteSize), *_flipTempSprite = lvl_s_alloc(spriteSize);
	
	if (_tempSprite && _flipTempSprite) {
		gfx_ConvertFromRLETSprite(in, _tempSprite);
		// flip across the y axis
		gfx_FlipSpriteY(_tempSprite, _flipTempSprite);
		
		gfx_ConvertToRLETSprite(_flipTempSprite, out);
	}
	lvl_s_reset();
}

#define FREE_SPRITE_ARR(arr, max)\
do {\
for (int __i = 0; __i < max; __i++)\
	if (arr[__i] != NULL)\
		free(arr[__i]);\
} while(0)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

DEF_ALLOC_TABLE(sprite, 8500)

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
	// flip mario sprites
	{mario_sprites[0][RIGHT], mario_sprites[0][LEFT], 5},
	{&mario_sprites[0][RIGHT][6], &mario_sprites[0][LEFT][6], 1},
	// flip luigi sprites
	{mario_sprites[1][RIGHT], mario_sprites[1][LEFT], 5},
	{&mario_sprites[1][RIGHT][6], &mario_sprites[1][LEFT][6], 1},
	{pipe_sprites[RIGHT], pipe_sprites[LEFT], 1},
};

void LoadExtraSprites(void) {
	for (size_t i = 0; i < ARR_LEN(convSprites); i++) {
		for (int j = 0; j < convSprites[i].num; j++) {
			if (convSprites[i].in[j] != NULL) {
				int s = rlet_size_necessary_flip(convSprites[i].in[j]);
				convSprites[i].out[j] = sprite_s_alloc(s);
				if (convSprites[i].out[j])
					FlipRletSpriteY(convSprites[i].in[j], convSprites[i].out[j]);
			}
		}
	}
}

void FreeExtraSprites(void) {
	sprite_s_reset();
}
#pragma clang diagnostic pop