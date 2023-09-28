#include "draw.h"

#include <compression.h>
#include <math.h>

#include "gfx/gfx.h"

#include "platforms.h"
#include "enemies.h"
#include "pipes.h"
#include "hud.h"

#define SET_OLD_TO_NEW_COORDS(varName) \
(varName)->x_old = (varName)->x; (varName)->y_old = (varName)->y;


gfx_rletsprite_t* mario_sprites[2][6] = {{stand_right, walk1_right, walk2_right, walk3_right, jump_right, dead}, {stand_left, walk1_left, walk2_left, walk3_left, jump_left, dead}};
gfx_sprite_t* platform_bump_sprites[3] = {level1_block_bump1, level1_block_bump2, level1_block_bump3};
gfx_rletsprite_t* enemy_sprites[2][2][7] = 
{
	{{spike_walk1_right, spike_walk2_right, spike_walk3_right, spike_upsidedown1_right}, {spike_walk1_left, spike_walk2_left, spike_walk3_left, spike_upsidedown1_left}},
	{{crab_walk1_right, crab_walk2_right, crab_walk3_right, crab_upsidedown1_right, crab_walk1_mad_right, crab_walk2_mad_right, crab_walk3_mad_right}, {crab_walk1_left, crab_walk2_left, crab_walk3_left, crab_upsidedown1_left, crab_walk1_mad_left, crab_walk2_mad_left, crab_walk3_mad_left}}
};
gfx_rletsprite_t* pipe_sprites[2][1] = {{pipe_stationary_right}, {pipe_stationary_left}};

uint8_t platform_bump_sprite_sheet[4] = {0, 1, 2, 1};

void DrawScene(player_t* player, unsigned int gameFrame) {
	uint8_t i;
	// get bgs under sprites before 
	// player
	gfx_GetSprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old + player->verSpriteOffset_old);
	
	// enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_GetSprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, levelEnemies.enemyArray[i].x_old, levelEnemies.enemyArray[i].y_old + levelEnemies.enemyArray[i].verSpriteOffset_old);
	}
	
	// hud
	HudGetBackground();
	
	// draw sprites over bgs
	// draw platforms
	// platforms are pre rendered, and a mask is put under the bump when it is needed
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		if (levelPlatforms.platformArray[i].needsRefresh || gameFrame == 1) { // if gameFrame is 1 because we draw the platform once on frame 0, but we need to draw it on the other buffer on frame 1
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y - PLATFORM_HEIGHT);
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
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].processedTileImage, levelPlatforms.platformArray[i].x, levelPlatforms.platformArray[i].y - PLATFORM_HEIGHT);
			gfx_Sprite((gfx_sprite_t*)bgFill, flooredXpos, levelPlatforms.platformArray[i].y);
			
			if (levelPlatforms.platformArray[i].bumpedTileXpos - levelPlatforms.platformArray[i].x >= levelPlatforms.platformArray[i].width - 2*BLOCK_SIZE) { // if we are near the edge, floor the block's x pos to two blocks behind the edge, then cutoff animation at the edge
				bgFill[0] = BLOCK_SIZE;
				bgFill[1] = BLOCK_SIZE*3;
				gfx_GetSprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_TransparentSprite(platform_bump_sprites[platform_bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 4]], levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width - (2*BLOCK_SIZE), levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_Sprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x + levelPlatforms.platformArray[i].width, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
			} else if (flooredXpos - levelPlatforms.platformArray[i].x < 0) {
				bgFill[0] = BLOCK_SIZE;
				bgFill[1] = BLOCK_SIZE*3;
				gfx_GetSprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x - BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_TransparentSprite(platform_bump_sprites[platform_bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 4]], levelPlatforms.platformArray[i].x - BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
				gfx_Sprite((gfx_sprite_t*)bgFill, levelPlatforms.platformArray[i].x - BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
			} else { // otherwise, play animation as normal
				gfx_TransparentSprite(platform_bump_sprites[platform_bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 4]], flooredXpos, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
			}
			
		}
	}
	
	
	// draw enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_RLETSprite(enemy_sprites[levelEnemies.enemyArray[i].type][levelEnemies.enemyArray[i].dir][levelEnemies.enemyArray[i].sprite], levelEnemies.enemyArray[i].x, levelEnemies.enemyArray[i].y + levelEnemies.enemyArray[i].verSpriteOffset);
	}
	
	// make sure pipes draw over enemies
	for (i = 0; i < NUM_OF_PIPES; i++) {
		if (pipes[i].redraw) {
			gfx_RLETSprite((gfx_rletsprite_t*)pipe_sprites[pipes[i].dir][pipes[i].sprite], pipes[i].x, pipes[i].y);
			if (gameFrame - pipes[i].redrawTime == 2) // only redraw twice to fill both buffers
				pipes[i].redraw = false;
		}
	}
	
	// draw player over pipes
	gfx_RLETSprite(mario_sprites[player->dir][player->sprite], player->x, player->y + player->verSpriteOffset);
	
	// draw hud over everything
	HudDraw(player);
	
	// finish drawing
	gfx_SwapDraw();
	
	// replace sprites with bgs in second layer
	
	// player
	gfx_Sprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old + player->verSpriteOffset_old);
	SET_OLD_TO_NEW_COORDS(player);
	player->verSpriteOffset_old = player->verSpriteOffset; 
	
	// platforms
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		if (levelPlatforms.platformArray[i].beingBumped) {
			gfx_Sprite_NoClip((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x_old, levelPlatforms.platformArray[i].y_old - PLATFORM_HEIGHT);
			SET_OLD_TO_NEW_COORDS(&levelPlatforms.platformArray[i]);
			if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 > 3) { // it needs to be greater 3 but it also needs 1 more frame for the other buffer
				levelPlatforms.platformArray[i].beingBumped = false;
				levelPlatforms.platformArray[i].needsRefresh = true;
			}
		} else if (levelPlatforms.platformArray[i].needsRefresh) {
			if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 > 4) {
				levelPlatforms.platformArray[i].needsRefresh = false;
			}
		}
	}
	
	// enemies
	for (i = 0; i < levelEnemies.numEnemies; i++) {
		gfx_Sprite((gfx_sprite_t*)levelEnemies.enemyArray[i].backgroundData, levelEnemies.enemyArray[i].x_old, levelEnemies.enemyArray[i].y_old + levelEnemies.enemyArray[i].verSpriteOffset_old);
		SET_OLD_TO_NEW_COORDS(&levelEnemies.enemyArray[i]);
		levelEnemies.enemyArray[i].verSpriteOffset_old = levelEnemies.enemyArray[i].verSpriteOffset;
	}
	
	// pipes
	for (i = 0; i < NUM_OF_PIPES; i++) {
		if (pipes[i].redraw) {
			gfx_Sprite_NoClip((gfx_sprite_t*)pipes[i].backgroundData, pipes[i].x, pipes[i].y);
		}
	}
	
	// hud
	HudRefresh();
	
}

void DrawBackground(void) {
	// draw bg in buffer
	gfx_sprite_t* levelBg = gfx_MallocSprite(127, 120);
	zx7_Decompress(levelBg->data, bg_pipes_top_left_compressed);
	gfx_Sprite(levelBg, 0, 0);
	gfx_Sprite(levelBg, 255, 0);
	zx7_Decompress(levelBg->data, bg_pipes_bottom_left_compressed);
	gfx_Sprite(levelBg, 0, 120);
	gfx_Sprite(levelBg, 255, 120);
	free(levelBg);
	levelBg = gfx_MallocSprite(128, 120);
	zx7_Decompress(levelBg->data, bg_pipes_top_right_compressed);
	gfx_Sprite(levelBg, 127, 0);
	zx7_Decompress(levelBg->data, bg_pipes_bottom_right_compressed);
	gfx_Sprite(levelBg, 127, 120);
	free(levelBg);
	
	for (uint8_t i = 0; i < 20; i++)
		gfx_TransparentSprite(level1_ground, i*level1_ground_width, 224);
	
	// copy it to the screen
	gfx_SetDrawScreen();
	gfx_BlitBuffer();
	// go back to offscreen buffer
	gfx_SetDrawBuffer();
}

void DecompressSprites(void) {
	
	
}