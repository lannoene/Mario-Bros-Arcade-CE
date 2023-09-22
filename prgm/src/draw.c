#include "draw.h"

#include "gfx/gfx.h"

#include "platforms.h"

#define SET_OLD_TO_NEW_COORDS(varName) \
varName x_old = varName x; varName y_old = varName y;


gfx_sprite_t* mario_sprites[2][5] = {{stand_right, walk1_right, walk2_right, walk3_right, jump_right}, {stand_left, walk1_left, walk2_left, walk3_left, jump_left}};

uint8_t mario_walking_sprite_table[4] = {2, 3, 2, 1};

void DrawScene(player_t* player, int gameFrame) {
	uint8_t i;
	// get bgs under sprites before drawing
	
	gfx_GetSprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old);
	
	for (i = 0; i < numPlatforms; i++) {
		// get bg as big as the width * height of the platform (set compile time)
		gfx_GetSprite((gfx_sprite_t*)platformArray[i].backgroundData, platformArray[i].x_old, platformArray[i].y_old);
	}
	
	
	// draw sprites over bgs
	gfx_TransparentSprite(mario_sprites[player->dir][player->sprite], player->x, player->y);
	
	for (i = 0; i < numPlatforms; i++) {
		for (uint8_t j = 0; j < PLATFORM_WIDTH/8; j++) // display block tiles for platform (might be slow, so I'll look into optamizing this later)
			gfx_TransparentSprite(level1_block, platformArray[i].x + j*8, platformArray[i].y);
	}
	// finish drawing
	gfx_SwapDraw();
	
	// replace sprites with bgs in second layer
	gfx_Sprite((gfx_sprite_t*)player->backgroundData, player->x_old, player->y_old);
	SET_OLD_TO_NEW_COORDS(player->);
	
	for (i = 0; i < numPlatforms; i++) {
		// replace entire platform with bg. this won't work correctly if PLATFORM_WIDTH isn't a multiple of 8. so REMEMBER THAT!!!!
		gfx_Sprite((gfx_sprite_t*)platformArray[i].backgroundData, platformArray[i].x_old*8, platformArray[i].y_old);
		SET_OLD_TO_NEW_COORDS(platformArray[i].);
	}
	
}

void DrawBackground(void) {
	// draw bg
	gfx_FillScreen(2);
	uint8_t i;
	for (i = 0; i < 25; i++) {
		gfx_TransparentSprite(level1_ground, i*level1_ground_width, 224);
	}
	
	// make sure other buffer also contains bg
	gfx_SwapDraw();
	gfx_BlitScreen();
}