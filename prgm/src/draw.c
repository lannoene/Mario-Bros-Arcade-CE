#include "draw.h"

#include "gfx/gfx.h"

#include "platforms.h"

#define SET_OLD_TO_NEW_COORDS(varName) \
(varName)->x_old = (varName)->x; (varName)->y_old = (varName)->y;


gfx_sprite_t* mario_sprites[2][5] = {{stand_right, walk1_right, walk2_right, walk3_right, jump_right}, {stand_left, walk1_left, walk2_left, walk3_left, jump_left}};
gfx_sprite_t* platform_bump_sprites[3] = {level1_block_bump1, level1_block_bump2, level1_block_bump3};
uint8_t bump_sprite_sheet[4] = {0, 1, 2, 1};

uint8_t mario_walking_sprite_table[4] = {2, 3, 2, 1};

void DrawScene(player_t* player, int gameFrame) {
	uint8_t i;
	// get bgs under sprites before drawing
	
	gfx_GetSprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old);
	
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		// get bg as big as the width * height of the platform (set compile time)
		gfx_GetSprite((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x_old, levelPlatforms.platformArray[i].y_old - BLOCK_SIZE);
	}
	
	
	// draw sprites over bgs
	gfx_TransparentSprite(mario_sprites[player->dir][player->sprite], player->x, player->y);
	
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		for (uint8_t j = 0; j < levelPlatforms.platformArray[i].width/BLOCK_SIZE; j++) {// display block tiles for platform (might be slow, so I'll look into optamizing this later)
			if (!levelPlatforms.platformArray[i].beingBumped) // if the platform isn't being bumped, show every tile
				gfx_TransparentSprite(level1_block, levelPlatforms.platformArray[i].x + j*BLOCK_SIZE, levelPlatforms.platformArray[i].y);
			else if (levelPlatforms.platformArray[i].beingBumped && (levelPlatforms.platformArray[i].x + j*BLOCK_SIZE <= levelPlatforms.platformArray[i].bumpedTileXpos - BLOCK_SIZE - 1 || levelPlatforms.platformArray[i].x + j*BLOCK_SIZE >= levelPlatforms.platformArray[i].bumpedTileXpos + BLOCK_SIZE*2)) // if the platform is being bumped, show the tiles that aren't in the shockwave range
				gfx_TransparentSprite(level1_block, levelPlatforms.platformArray[i].x + j*BLOCK_SIZE, levelPlatforms.platformArray[i].y);
			else { // otherwise, don't show them
				if (levelPlatforms.platformArray[i].x + j*BLOCK_SIZE >= levelPlatforms.platformArray[i].bumpedTileXpos - BLOCK_SIZE - 1 && levelPlatforms.platformArray[i].x + j*BLOCK_SIZE < levelPlatforms.platformArray[i].bumpedTileXpos) { // make sure we are only using the first block's xpos to animate the bump
					gfx_TransparentSprite(platform_bump_sprites[bump_sprite_sheet[(gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 % 4]], levelPlatforms.platformArray[i].x + j*BLOCK_SIZE, levelPlatforms.platformArray[i].y - BLOCK_SIZE);
					if ((gameFrame - levelPlatforms.platformArray[i].timeOfLastBump)/4 > 3)
						levelPlatforms.platformArray[i].beingBumped = false;
				}
			}
		}
	}
	// finish drawing
	gfx_SwapDraw();
	
	// replace sprites with bgs in second layer
	gfx_Sprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old);
	SET_OLD_TO_NEW_COORDS(player);
	
	for (i = 0; i < levelPlatforms.numPlatforms; i++) {
		// replace entire platform with bg. this wont work correctly if PLATFORM_WIDTH isn't a multiple of 8. so REMEMBER THAT!!!!
		gfx_Sprite((gfx_sprite_t*)levelPlatforms.platformArray[i].backgroundData, levelPlatforms.platformArray[i].x_old, levelPlatforms.platformArray[i].y_old - BLOCK_SIZE);
		SET_OLD_TO_NEW_COORDS(&levelPlatforms.platformArray[i]);
	}
	
}

void DrawBackground(void) {
	// draw bg in buffer
	gfx_FillScreen(2);
	for (uint8_t i = 0; i < 20; i++)
		gfx_TransparentSprite_NoClip(level1_ground, i*level1_ground_width, 224);
	
	// copy it to the screen
	gfx_SetDrawScreen();
	gfx_BlitBuffer();
	// go back to offscreen buffer
	gfx_SetDrawBuffer();
}