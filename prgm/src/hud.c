#include "hud.h"

#include <graphx.h>
#include <math.h>

#include "gfx/gfx.h"
#include "level.h"
#include "bonus.h"
#include "defines.h"
#include "draw.h"

#define TEXT_WIDTH	8
#define TEXT_HEIGHT	15

#define PAUSE_WIDTH 100
#define PAUSE_HEIGHT 100

#define NUM_PAUSE_OPTIONS 4

// literally no other file needs access to the hud directly
typedef struct {
	uint8_t livesBackgroundData[TEXT_SIZE*9*TEXT_SIZE + 2];
	uint8_t scoreBackgroundData[TEXT_SIZE*7*TEXT_SIZE + 2];
	uint8_t phaseCardBackgroundData[phase_card_width*phase_card_height*2 + 2];
	uint8_t phaseCardNumDigits;
	uint8_t bonusTimerBackgroundData[TEXT_SIZE*4*TEXT_SIZE + 2];
#ifdef DEBUG_TIMER
	uint8_t gameFrameBGData[TEXT_SIZE*9*TEXT_SIZE + 2];
#endif
	uint8_t pauseScrBackgroundData[100*100 + 2];
	uint8_t respawnPlatformBgData[15*7 + 2];
} hud_t;

static hud_t hudData;

typedef struct {
	uint16_t x;
	uint8_t y, selectedOption;
} puaseScreen_t;

static puaseScreen_t pauseData;

gfx_rletsprite_t* phase_numbers[10] = {
	phase_number0,
	phase_number1,
	phase_number2,
	phase_number3,
	phase_number4,
	phase_number5,
	phase_number6,
	phase_number7,
	phase_number8,
	phase_number9
};

gfx_rletsprite_t* respawn_platforms[3] = {
	respawn_platform_full,
	respawn_platform_med,
	respawn_platform_empty,
};

static inline void DrawPhaseText(void);
static void DrawPauseScreen(void);

void InitHud(void) {
	hudData.livesBackgroundData[0] = TEXT_SIZE*9;
	hudData.livesBackgroundData[1] = TEXT_SIZE;
	hudData.scoreBackgroundData[0] = TEXT_SIZE*7;
	hudData.scoreBackgroundData[1] = TEXT_SIZE;
	hudData.phaseCardBackgroundData[0] = phase_card_width*2;
	hudData.phaseCardBackgroundData[1] = phase_card_height;
	hudData.bonusTimerBackgroundData[0] = TEXT_SIZE*4;
	hudData.bonusTimerBackgroundData[1] = TEXT_SIZE;
	hudData.pauseScrBackgroundData[0] = PAUSE_WIDTH;
	hudData.pauseScrBackgroundData[1] = PAUSE_HEIGHT;
	hudData.respawnPlatformBgData[0] = 15;
	hudData.respawnPlatformBgData[1] = 7;
	
#ifdef DEBUG_TIMER
	hudData.gameFrameBGData[0] = TEXT_SIZE*9;
	hudData.gameFrameBGData[1] = TEXT_SIZE;
#endif
}
bool pausedLastFrame = false; // stupid hacky solution. i don't really care though. i'm super tired rn

// lives text width TEXT_SIZE*9
void HudGetBackground(player_t* player) {
	// lives
	gfx_GetSprite((gfx_sprite_t*)hudData.livesBackgroundData, 0, 0);
	
	// score
	gfx_GetSprite((gfx_sprite_t*)hudData.scoreBackgroundData, 272, 0);
	
	// phase card (only when round starts)
	gfx_GetSprite((gfx_sprite_t*)hudData.phaseCardBackgroundData, 120, 100);
	
	gfx_GetSprite((gfx_sprite_t*)hudData.bonusTimerBackgroundData, 100, 0);
#ifdef DEBUG_TIMER
	gfx_GetSprite((gfx_sprite_t*)hudData.gameFrameBGData, 0, 100);
#endif
	if (game_data.paused)
		pausedLastFrame = true;
	if (game_data.paused || pausedLastFrame)
		gfx_GetSprite((gfx_sprite_t*)hudData.pauseScrBackgroundData, 160 - (PAUSE_WIDTH/2), 120 - (PAUSE_WIDTH/2));
	
	if (player->state == PLAYER_RESPAWNING) { // i'm planning on changing this later, but i don't know what i'd do that makes sense
		gfx_GetSprite((gfx_sprite_t*)hudData.respawnPlatformBgData, FIXED_POINT_TO_INT(player->x), FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT);
	}
}

void HudDraw(player_t* player, unsigned int gameFrame) {
	// draw lives counter
	gfx_SetTextXY(0, 0);
	gfx_SetTextFGColor(1);
	gfx_PrintString("Lives: ");
	gfx_SetTextXY(48, 0);
	gfx_PrintUInt(player->lives, 2);
	
	// draw score counter
	gfx_SetTextXY(272, 0);
	gfx_PrintUInt(player->score, 6);
	
	// draw phase card
	if (gameFrame - game_data.levelStartTime < 150) {
		gfx_RLETSprite_NoClip(phase_card, 120, 100);
		DrawPhaseText();
	}
	
	if (game_data.isBonusLevel) {
		gfx_SetTextXY(100 + TEXT_SIZE, 0);
		gfx_PrintUInt((unsigned int)ceil((float)levelCoins.bonusTimer/60), 2); // get 1s place and on
		
		gfx_PrintStringXY(":", 100 + (TEXT_SIZE*3), 0);
		
		gfx_SetTextXY(100 + (TEXT_SIZE*4), 0);
		gfx_PrintUInt(((levelCoins.bonusTimer*10)/60) % 10, 1); // get tenths place
	}
	
	if (game_data.paused)
		DrawPauseScreen();
	
	// draw level end card
	if (gameFrame - game_data.levelEndTime < 150 && !game_data.isBonusLevel) {
		gfx_RLETSprite_NoClip(phase_clear, 130, 100);
	}
	
	if (player->state == PLAYER_RESPAWNING) { // this was honestly the easiest way to do this
		gfx_RLETSprite(respawn_platforms[(((gameFrame - player->spawnTime < PLAYER_RESP_FALL_DURATION) ? 0 : gameFrame - player->spawnTime - PLAYER_RESP_FALL_DURATION)*3)/(PLAYER_RESP_WAIT_MAX)], FIXED_POINT_TO_INT(player->x), FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT);
	}
	
#ifdef DEBUG_TIMER
	gfx_SetTextXY(0, 100);
	gfx_PrintUInt(gameFrame, 10);
#endif
}

void HudRefresh(player_t* player, unsigned int gameFrame) {
	gfx_Sprite_NoClip((gfx_sprite_t*)hudData.livesBackgroundData, 0, 0);
	gfx_Sprite_NoClip((gfx_sprite_t*)hudData.scoreBackgroundData, 272, 0);
	if (gameFrame - game_data.levelStartTime <= 150)
		gfx_Sprite((gfx_sprite_t*)hudData.phaseCardBackgroundData, 120, 100);
	
	if (game_data.isBonusLevel)
		gfx_Sprite((gfx_sprite_t*)hudData.bonusTimerBackgroundData, 100 + TEXT_SIZE, 0);
#ifdef DEBUG_TIMER
	gfx_Sprite((gfx_sprite_t*)hudData.gameFrameBGData, 0, 100);
#endif
	if (game_data.paused || pausedLastFrame) {
		gfx_Sprite((gfx_sprite_t*)hudData.pauseScrBackgroundData, 160 - (PAUSE_WIDTH/2), 120 - (PAUSE_WIDTH/2));
		if (!game_data.paused)
			pausedLastFrame = false;
	}
	
	if (player->state == PLAYER_RESPAWNING) {
		gfx_Sprite((gfx_sprite_t*)hudData.respawnPlatformBgData, FIXED_POINT_TO_INT(player->x), FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT);
	}
}

static inline void DrawPhaseText(void) {
	for (uint8_t i = 0; i < hudData.phaseCardNumDigits + 1; i++) { // for every digit
		gfx_RLETSprite_NoClip(phase_numbers[((uint8_t)(game_data.level/pow(10, i)))%10], 176 - TEXT_WIDTH*i, 100); // display the digit there at correct x coord
	}
}

void TitleCardSetNumDigits(uint8_t numDigits) {
	hudData.phaseCardNumDigits = numDigits;
}

static void DrawPauseScreen(void) {
	/*
	 * this is really stupid and here's why
	 * i'm redrawing everything every frame
	 * even though i only need to redraw things that move
	 * this causes the calc to run super slowly in the menu
	 * i have school though and i just don't have any time to fix this
	 * sorry XD
	 */
	
	for (uint8_t i = 0; i < PAUSE_HEIGHT; i++) {
		gfx_SetColor(2);
		gfx_FillRectangle_NoClip(160 - (PAUSE_WIDTH/2), 120 - (PAUSE_WIDTH/2) + i, PAUSE_WIDTH, 1); // center rectangle
	}
	gfx_SetColor(2);
	gfx_PrintStringXY("Paused:", 160 - (PAUSE_WIDTH/2), 120 - (PAUSE_WIDTH/2));
	gfx_PrintStringXY("Resume", 160 - (PAUSE_WIDTH/2) + TEXT_SIZE, 120 - (PAUSE_WIDTH/2) + TEXT_SIZE);
	gfx_PrintStringXY("Save & Exit", 160 - (PAUSE_WIDTH/2) + TEXT_SIZE, 120 - (PAUSE_WIDTH/2) + (TEXT_SIZE*2));
	gfx_PrintStringXY("Exit", 160 - (PAUSE_WIDTH/2) + TEXT_SIZE, 120 - (PAUSE_WIDTH/2) + (TEXT_SIZE*3));
	gfx_PrintStringXY("Restart", 160 - (PAUSE_WIDTH/2) + TEXT_SIZE, 120 - (PAUSE_WIDTH/2) + (TEXT_SIZE*4));
	gfx_PrintStringXY("Phase:", 160 - (PAUSE_WIDTH/2), 120 - (PAUSE_WIDTH/2) + (TEXT_SIZE*11));
	gfx_SetTextXY(160 - (PAUSE_WIDTH/2) + TEXT_SIZE*7, 120 - (PAUSE_WIDTH/2) + (TEXT_SIZE*11));
	if (hudData.phaseCardNumDigits > 1)
		gfx_PrintUInt(game_data.level, 2);
	else
		gfx_PrintUInt(game_data.level, 1);
	gfx_PrintStringXY(">", 160 - (PAUSE_WIDTH/2), 120 - (PAUSE_WIDTH/2) + (TEXT_SIZE*pauseData.selectedOption) + TEXT_SIZE);
}

bool pressedThisFrame = false;

int8_t PauseScreenInputEvent(uint8_t input) {
	switch (input) {
		case 2: // up
			if (!pressedThisFrame)
				--pauseData.selectedOption;
			break;
		case 3: // down
			if (!pressedThisFrame)
				++pauseData.selectedOption;
			break;
		case 4: // select
			return pauseData.selectedOption;
			break;
		case 5:
			pressedThisFrame = false;
			return -1;
			break;
	}
	
	pauseData.selectedOption %= NUM_PAUSE_OPTIONS;
	pressedThisFrame = true;
	return -1;
}

void PauseScreenResetCursorPos(void) {
	pauseData.selectedOption = 0;
}

void GetRidOfRespawnPlatformRemnants(player_t* player) {
	gfx_Sprite((gfx_sprite_t*)hudData.respawnPlatformBgData, FIXED_POINT_TO_INT(player->x), FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT);
	gfx_SetDrawScreen();
	gfx_Sprite((gfx_sprite_t*)hudData.respawnPlatformBgData, FIXED_POINT_TO_INT(player->x), FIXED_POINT_TO_INT(player->y) + PLAYER_HEIGHT);
	gfx_SetDrawBuffer();
}