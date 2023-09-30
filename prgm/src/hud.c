#include "hud.h"

#include <graphx.h>
#include <math.h>

#include "gfx/gfx.h"
#include "level.h"

#define TEXT_WIDTH	8
#define TEXT_HEIGHT	15

// literally no other file needs access to the hud directly
typedef struct {
	uint8_t livesBackgroundData[TEXT_SIZE*9*TEXT_SIZE + 2];
	uint8_t scoreBackgroundData[TEXT_SIZE*7*TEXT_SIZE + 2];
	uint8_t phaseCardBackgroundData[phase_card_width*phase_card_height*2 + 2];
} hud_t;

static hud_t hudData;

gfx_rletsprite_t* phase_numbers[10] = {phase_number0, phase_number1, phase_number2, phase_number3, phase_number4, phase_number5, phase_number6, phase_number7, phase_number8, phase_number9};

void DrawPhaseText(void);

void InitHud(player_t* player) {
	hudData.livesBackgroundData[0] = TEXT_SIZE*9;
	hudData.livesBackgroundData[1] = TEXT_SIZE;
	hudData.scoreBackgroundData[0] = TEXT_SIZE*7;
	hudData.scoreBackgroundData[1] = TEXT_SIZE;
	hudData.phaseCardBackgroundData[0] = phase_card_width*2;
	hudData.phaseCardBackgroundData[1] = phase_card_height;
	
}

// lives text width TEXT_SIZE*9
void HudGetBackground(void) {
	// lives
	gfx_GetSprite((gfx_sprite_t*)hudData.livesBackgroundData, 0, 0);
	
	// score
	gfx_GetSprite((gfx_sprite_t*)hudData.scoreBackgroundData, 272, 0);
	
	// phase card (only when round starts)
	gfx_GetSprite((gfx_sprite_t*)hudData.phaseCardBackgroundData, 100, 100);
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
		gfx_RLETSprite_NoClip(phase_card, 100, 100);
		DrawPhaseText();
	}
	
	// draw level end card
	if (gameFrame - game_data.levelEndTime < 150) {
		gfx_RLETSprite_NoClip(phase_clear, 100, 100);
		
	}
}

void HudRefresh(void) {
	gfx_Sprite_NoClip((gfx_sprite_t*)hudData.livesBackgroundData, 0, 0);
	gfx_Sprite_NoClip((gfx_sprite_t*)hudData.scoreBackgroundData, 272, 0);
	gfx_Sprite((gfx_sprite_t*)hudData.phaseCardBackgroundData, 100, 100);
}

void DrawPhaseText(void) {
	uint8_t numDigits = floor(log10(game_data.level) + 0.0000001); // get number of digits
	for (uint8_t i = 0; i < numDigits + 1; i++) { // for every digit
		gfx_RLETSprite_NoClip(phase_numbers[((uint8_t)(game_data.level/pow(10, i)))%10], 160 - TEXT_WIDTH*i, 100); // display the digit there at correct x coord
	}
}