#include "draw.h"

#include <compression.h>
#include <math.h>

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

gfx_rletsprite_t* mario_sprites[2][8] = {{stand_right, walk1_right, walk2_right, walk3_right, jump_right, dead, slide_right, blank_stare}, {stand_left, walk1_left, walk2_left, walk3_left, jump_left, dead, slide_left, blank_stare}};
gfx_sprite_t* platform_bump_sprites[5][3] = {{level1_block_bump1, level1_block_bump2, level1_block_bump3}, {lava_block_bump1, lava_block_bump2, lava_block_bump3}, {castle_block_bump1, castle_block_bump2, castle_block_bump3}, {snowy_normal_block_bump1, snowy_normal_block_bump2, snowy_normal_block_bump3}, {snowy_iced_block_bump1, snowy_iced_block_bump2, snowy_iced_block_bump3}};
gfx_rletsprite_t* enemy_sprites[4][2][8] = 
{
	{{spike_walk1_right, spike_walk2_right, spike_walk3_right, spike_upsidedown1_right}, {spike_walk1_left, spike_walk2_left, spike_walk3_left, spike_upsidedown1_left}},
	{{crab_walk1_right, crab_walk2_right, crab_walk3_right, crab_upsidedown1_right, crab_walk1_mad_right, crab_walk2_mad_right, crab_walk3_mad_right}, {crab_walk1_left, crab_walk2_left, crab_walk3_left, crab_upsidedown1_left, crab_walk1_mad_left, crab_walk2_mad_left, crab_walk3_mad_left}},
	{{fly_ground, fly_wing1, fly_wing2, fly_dead_right}, {fly_ground, fly_wing1, fly_wing2, fly_dead_left}},
	{{freezie_walk1_right, freezie_walk2_right, freezie_walk3_right, freezie_die1_right, freezie_die2_right, freezie_die3_right, freezie_die4_right, freezie_die5_right}, {freezie_walk1_left, freezie_walk2_left, freezie_walk3_left, freezie_die1_left, freezie_die2_left, freezie_die3_left, freezie_die4_left, freezie_die5_left}}
};
gfx_rletsprite_t* pow_sprites[3] = {pow_full, pow_medium, pow_low};
gfx_rletsprite_t* pipe_sprites[2][1] = {{pipe_stationary_right}, {pipe_stationary_left}};
gfx_rletsprite_t* fireball_sprites[2][8] = {{fireball_green_big_rot1, fireball_green_big_rot2, fireball_green_big_rot3, fireball_green_big_rot4, fireball_green_small_rot1, fireball_green_small_rot2, fireball_green_small_rot3, fireball_green_small_rot4}, {fireball_red_big_rot1, fireball_red_big_rot2, fireball_red_big_rot3, fireball_red_big_rot4, fireball_red_small_rot1, fireball_red_small_rot2, fireball_red_small_rot3, fireball_red_small_rot4}};
gfx_rletsprite_t* coin_sprites[5] = {coin1, coin2, coin3, coin4, coin5};
gfx_rletsprite_t* icicle_sprites[6] = {icicle_forming1, icicle_forming2, icicle_forming3, icicle_full1, icicle_full2, icicle_full3};
gfx_rletsprite_t* particle_sprites[] = {dust_cloud_big, dust_cloud_medium, dust_cloud_small, star_hit, score_single, score_double, score_triple, score_quad, score_1up, coin_pick_1, coin_pick_2, coin_pick_3, coin_pick_4, coin_pick_5};

uint8_t platform_bump_sprite_sheet[5] = {0, 1, 2, 1, 0};

void DrawScene(player_t* player, uint8_t backgroundType, unsigned int gameFrame) {
	uint8_t i;
	// get bgs under sprites before 
	// player
	gfx_GetSprite((gfx_sprite_t*)player->backgroundData, FIXED_POINT_TO_INT(player->x_old), FIXED_POINT_TO_INT(player->y_old) + player->verSpriteOffset_old);
	
	// enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].x_old) + levelEnemies.enemyArray[i].horSpriteOffset_old, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].y_old) + levelEnemies.enemyArray[i].verSpriteOffset_old);
	}
	
	// hud
	HudGetBackground(player);
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelPows.powArray[i].backgroundData, FIXED_POINT_TO_INT(levelPows.powArray[i].x), FIXED_POINT_TO_INT(levelPows.powArray[i].y));
	}
	
	// coins
	for (i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive)
			gfx_GetSprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, FIXED_POINT_TO_INT(levelCoins.coinArray[i].x_old), FIXED_POINT_TO_INT(levelCoins.coinArray[i].y_old));
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
			
			int16_t flooredXpos = (platform->bumpedTileXpos/BLOCK_SIZE)*BLOCK_SIZE; // integers get floored automatically
			// get bg around bump
			uint8_t bgFill[BLOCK_SIZE*3*BLOCK_SIZE + 2]; // only fill bottom row with bg because top row always starts cleared
			bgFill[0] = BLOCK_SIZE*3;
			bgFill[1] = BLOCK_SIZE;
			gfx_GetSprite((gfx_sprite_t*)bgFill, flooredXpos, platFxToIntY);
			gfx_Sprite_NoClip((gfx_sprite_t*)platform->processedTileImage, platFxToIntX, platFxToIntY);
			gfx_Sprite((gfx_sprite_t*)bgFill, flooredXpos, platFxToIntY); // fill in 3 blocks of platform that you bump
			
			if (platform->bumpedTileXpos - platFxToIntX >= platFxToIntWidth - 2*BLOCK_SIZE) { // if we are near the edge, floor the block's x pos to two blocks behind the edge, then cutoff animation at the edge
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
		gfx_RLETSprite(enemy_sprites[levelEnemies.enemyArray[i].type][levelEnemies.enemyArray[i].dir][levelEnemies.enemyArray[i].sprite], FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].x) + levelEnemies.enemyArray[i].horSpriteOffset, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].y) + levelEnemies.enemyArray[i].verSpriteOffset);
	}
	
	// coins
	for (i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive)
			gfx_RLETSprite(coin_sprites[levelCoins.coinArray[i].sprite], FIXED_POINT_TO_INT(levelCoins.coinArray[i].x), FIXED_POINT_TO_INT(levelCoins.coinArray[i].y));
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
	gfx_RLETSprite(mario_sprites[player->dir][player->sprite], FIXED_POINT_TO_INT(player->x), FIXED_POINT_TO_INT(player->y) + player->verSpriteOffset);
	
	// icicles
	for (i = 0; i < levelIcicles.numIcicles; i++) {
		if (levelIcicles.icicleArray[i].state != ICICLE_DEAD)
			gfx_RLETSprite(icicle_sprites[levelIcicles.icicleArray[i].sprite], levelIcicles.icicleArray[i].x, levelIcicles.icicleArray[i].y);
	}
	
	// draw hud over everything
	HudDraw(player, gameFrame);
	
	
	// finish drawing
	gfx_SwapDraw();
	
	// replace sprites with bgs in second layer
	
	// player
	gfx_Sprite((gfx_sprite_t*)player->backgroundData, FIXED_POINT_TO_INT(player->x_old), FIXED_POINT_TO_INT(player->y_old) + player->verSpriteOffset_old);
	SET_OLD_TO_NEW_COORDS(player);
	player->verSpriteOffset_old = player->verSpriteOffset; 
	
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
		gfx_Sprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].x_old) + levelEnemies.enemyArray[i].horSpriteOffset_old, FIXED_POINT_TO_INT(levelEnemies.enemyArray[i].y_old) + levelEnemies.enemyArray[i].verSpriteOffset_old);
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
	HudRefresh(player, gameFrame);
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		gfx_Sprite_NoClip((gfx_sprite_t*)levelPows.powArray[i].backgroundData, FIXED_POINT_TO_INT(levelPows.powArray[i].x), FIXED_POINT_TO_INT(levelPows.powArray[i].y));
	}
	
	// coins
	for (i = 0; i < levelCoins.numCoins; i++) {
		gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, FIXED_POINT_TO_INT(levelCoins.coinArray[i].x_old), FIXED_POINT_TO_INT(levelCoins.coinArray[i].y_old));
		SET_OLD_TO_NEW_COORDS(&levelCoins.coinArray[i]);
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
			zx7_Decompress(gfx_vbuffer, bg_pipes_compressed);
			break;
		case BG_LAVA:
			zx7_Decompress(gfx_vbuffer, bg_lava_compressed);
			break;
		case BG_CASTLE:
			zx7_Decompress(gfx_vbuffer, bg_castle_compressed);
			break;
		case BG_SNOWY:
			zx7_Decompress(gfx_vbuffer, bg_snowy_compressed);
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