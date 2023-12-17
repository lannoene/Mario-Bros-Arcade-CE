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

#define SET_OLD_TO_NEW_COORDS(varName) \
(varName)->x_old = (varName)->x; (varName)->y_old = (varName)->y;

gfx_rletsprite_t* mario_sprites[2][7] = {{stand_right, walk1_right, walk2_right, walk3_right, jump_right, dead, slide_right}, {stand_left, walk1_left, walk2_left, walk3_left, jump_left, dead, slide_left}};
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
gfx_rletsprite_t* fireball_sprites[2][4] = {{fireball_green_big_rot1, fireball_green_big_rot2, fireball_green_big_rot3, fireball_green_big_rot4}, {fireball_red_big_rot1, fireball_red_big_rot2, fireball_red_big_rot3, fireball_red_big_rot4}};
gfx_rletsprite_t* coin_sprites[5] = {coin1, coin2, coin3, coin4, coin5};
gfx_rletsprite_t* icicle_sprites[6] = {icicle_forming1, icicle_forming2, icicle_forming3, icicle_full1, icicle_full2, icicle_full3};
gfx_rletsprite_t* particle_sprites[] = {dust_cloud_big, dust_cloud_medium, dust_cloud_small, star_hit, score_single, score_double, score_triple, score_quad, score_1up, coin_pick_1, coin_pick_2, coin_pick_3, coin_pick_4, coin_pick_5};

uint8_t platform_bump_sprite_sheet[5] = {0, 1, 2, 1, 0};

void DrawScene(player_t* player, uint8_t backgroundType, unsigned int gameFrame) {
	uint8_t i;
	// get bgs under sprites before 
	// player
	gfx_GetSprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old + player->verSpriteOffset_old);
	
	// enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, levelEnemies.enemyArray[i].x_old + levelEnemies.enemyArray[i].horSpriteOffset_old, levelEnemies.enemyArray[i].y_old + levelEnemies.enemyArray[i].verSpriteOffset_old);
	}
	
	// hud
	HudGetBackground();
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelPows.powArray[i].backgroundData, levelPows.powArray[i].x, levelPows.powArray[i].y);
	}
	
	// coins
	for (i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive)
			gfx_GetSprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, levelCoins.coinArray[i].x_old, levelCoins.coinArray[i].y_old);
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
		if (levelPlatforms.platformArray[i].invisible)
			continue;
		if (levelPlatforms.platformArray[i].needsRefresh || (gameFrame - game_data.levelStartTime) == 1) { // if gameFrame is 1 because we draw the platform once on frame 0, but we need to draw it on the other buffer on frame 1
			gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y - PLATFORM_HEIGHT);
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y);
		}
		if (!levelPlatforms.platformArray[i].beingBumped) {// if the platform isn't being bumped, show every tile
			continue;
		} else if (levelPlatforms.platformArray[i].beingBumped) {
			
			int16_t flooredXpos = (levelPlatforms.platformArray[i].bumpedTileXpos/BLOCK_SIZE)*BLOCK_SIZE; // integers get floored automatically
			// get bg around bump
			uint8_t bgFill[BLOCK_SIZE*3*BLOCK_SIZE + 2]; // only fill bottom row with bg because top row always starts cleared
			bgFill[0] = BLOCK_SIZE*3;
			bgFill[1] = BLOCK_SIZE;
			gfx_GetSprite((gfx_sprite_t*)bgFill, flooredXpos, levelPlatforms.platformArray[i].y);
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y);
			gfx_Sprite((gfx_sprite_t*)bgFill, flooredXpos, levelPlatforms.platformArray[i].y); // fill in 3 blocks of platform that you bump
			
			if (levelPlatforms.platformArray[i].bumpedTileXpos - levelPlatforms.platformArray[i].x >= levelPlatforms.platformArray[i].width - 2*BLOCK_SIZE) { // if we are near the edge, floor the block's x pos to two blocks behind the edge, then cutoff animation at the edge
				bgFill[0] = BLOCK_SIZE;
				bgFill[1] = BLOCK_SIZE*3;
				gfx_GetSprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_TransparentSprite(platform_bump_sprites[(levelPlatforms.platformArray[i].icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 5]], levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width - (2*BLOCK_SIZE), levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_Sprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
			} else if (flooredXpos - levelPlatforms.platformArray[i].x < 0) {
				bgFill[0] = BLOCK_SIZE;
				bgFill[1] = BLOCK_SIZE*3;
				gfx_GetSprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x - BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_TransparentSprite(platform_bump_sprites[(levelPlatforms.platformArray[i].icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 5]], levelPlatforms.platformArray[i].x - BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_Sprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x - BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
			} else { // otherwise, play animation as normal
				gfx_TransparentSprite(platform_bump_sprites[(levelPlatforms.platformArray[i].icy) ? 4 : backgroundType][platform_bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 5]], flooredXpos, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
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
		gfx_RLETSprite(enemy_sprites[levelEnemies.enemyArray[i].type][levelEnemies.enemyArray[i].dir][levelEnemies.enemyArray[i].sprite], levelEnemies.enemyArray[i].x + levelEnemies.enemyArray[i].horSpriteOffset, levelEnemies.enemyArray[i].y + levelEnemies.enemyArray[i].verSpriteOffset);
	}
	
	// coins
	for (i = 0; i < levelCoins.numCoins; i++) {
		if (levelCoins.coinArray[i].alive)
			gfx_RLETSprite(coin_sprites[levelCoins.coinArray[i].sprite], levelCoins.coinArray[i].x, levelCoins.coinArray[i].y);
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
			gfx_RLETSprite(pow_sprites[levelPows.powArray[i].state], levelPows.powArray[i].x, levelPows.powArray[i].y);
	}
	
	// fireballs
	for (i = 0; i < levelFireballs.numFireballs; i++) {
		if (levelFireballs.fireballArray[i].alive)
			gfx_RLETSprite(fireball_sprites[levelFireballs.fireballArray[i].type][levelFireballs.fireballArray[i].sprite], levelFireballs.fireballArray[i].x, levelFireballs.fireballArray[i].y);
	}
	
	// draw player over everything physical
	gfx_RLETSprite(mario_sprites[player->dir][player->sprite], player->x, player->y + player->verSpriteOffset);
	
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
	gfx_Sprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old + player->verSpriteOffset_old);
	SET_OLD_TO_NEW_COORDS(player);
	player->verSpriteOffset_old = player->verSpriteOffset; 
	
	// platforms
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 == 5 && !levelPlatforms.platformArray[i].invisible) {
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y - PLATFORM_HEIGHT);
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y);
		}
		if (levelPlatforms.platformArray[i].beingBumped) {
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x_old, levelPlatforms.platformArray[i].y_old - PLATFORM_HEIGHT);
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
		gfx_Sprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, levelEnemies.enemyArray[i].x_old + levelEnemies.enemyArray[i].horSpriteOffset_old, levelEnemies.enemyArray[i].y_old + levelEnemies.enemyArray[i].verSpriteOffset_old);
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
	HudRefresh(gameFrame);
	
	// pows
	for (i = 0; i < levelPows.numPows; i++) {
		gfx_Sprite_NoClip((gfx_sprite_t*)levelPows.powArray[i].backgroundData, levelPows.powArray[i].x, levelPows.powArray[i].y);
	}
	
	// coins
	for (i = 0; i < levelCoins.numCoins; i++) {
		gfx_Sprite((gfx_sprite_t*)levelCoins.coinArray[i].backgroundData, levelCoins.coinArray[i].x_old, levelCoins.coinArray[i].y_old);
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