#pragma once

#define MAX_WAVES	10

typedef struct {
	bool isBonus, hasFreezies : 1;
	uint8_t bg; // background
	uint8_t fireballFlags;
	uint8_t platformFlags;
	//uint8_t maxFreezies;
	uint8_t maxIcicles;
	uint8_t waveDuration; // default seconds 'till next wave. offset by random value
	uint8_t defaultSpawnFlags; // if wave spawn flags are absent, then these will be used
	struct enemy_wave_t {
		int8_t totalPoints;
		uint8_t enemySpawnFlags; // which enemies can be spawned this wave
		uint8_t enemySpawnWait;
	} enemyWaves[MAX_WAVES]; // number of points in each wave.
} game_level_t;

game_level_t levelLog[] = {
	{false, false, BG_PIPES, HAS_NO_FIREBALLS, NONE_ICY, 0, 10, ENEMY_SPIKE,
		{
			{2, 0, 1},
			{1, 0, 1},
			{-1, 0, 0}
		},
	},
	{false, false, BG_PIPES, HAS_NO_FIREBALLS, NONE_ICY, 0, 10, ENEMY_SPIKE,
		{
			{2, 0, 1},
			{2, 0, 1},
			{-1, 0, 0}
		},
	},
	{false, false, BG_PIPES, HAS_NO_FIREBALLS, NONE_ICY, 0, 10, ENEMY_SPIKE,
		{
			{2, 0, 1},
			{2, 0, 1},
			{2, 0, 0},
			{-1, 0, 0}
		},
	},
	{true, false, BG_SNOWY, HAS_NO_FIREBALLS, NONE_ICY, 0, 0, 0,
		{
			{-1, 0, 0}
		},
	},
	{false, false, BG_LAVA, HAS_NO_FIREBALLS, NONE_ICY, 0, 10, ENEMY_CRAB,
		{
			{2, 0, 1},
			{2, 0, 1},
			{-1, 0, 0}
		},
	},
	{false, false, BG_LAVA, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_CRAB,
		{
			{2, 0, 1},
			{2, ENEMY_SPIKE, 1},
			{2, 0, 1},
			{-1, 0, 0}
		},
	},
	{false, false, BG_CASTLE, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_FLY,
		{
			{2, 0, 1},
			{-1, 0, 0}
		},
	},
	{false, false, BG_CASTLE, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_FLY,
		{
			{2, 0, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{-1, 0, 0}
		},
	},
	{true, false, BG_SNOWY, HAS_NO_FIREBALLS, TOP_ICY | MIDDLE_ICY | BOTTOM_ICY, 0, 0, 0,
		{
			{-1, 0, 0}
		},
	},
	// phase 10
	{false, true, BG_SNOWY, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_SPIKE, 1},
			{2, ENEMY_SPIKE | ENEMY_FLY, 1},
			{1, ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_FLY | ENEMY_CRAB, 1},
			{2, ENEMY_FLY | ENEMY_CRAB, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_FLY | ENEMY_CRAB, 5},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_SPIKE, 1},
			{2, ENEMY_FLY | ENEMY_SPIKE, 1},
			{1, ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_FLY | ENEMY_CRAB, 1},
			{2, ENEMY_CRAB, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN, NONE_ICY, 0, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{-1, 0, 0}
		},
	},
	{true, false, BG_SNOWY, HAS_NO_FIREBALLS, PLATFORMS_ARE_INVISIBLE, 0, 0, 0,
		{
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 1, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_SPIKE, 1},
			{2, ENEMY_CRAB | ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 1, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 1, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{-1, 0, 0}
		},
	},
	// phase 20
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_SPIKE, 1},
			{2, 0, 1},
			{1, ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, 0, 1},
			{-1, 0, 0}
		},
	},
	//isbonus, freezieflag, background, fireballg flag | fireballr flag, icyplatform flag, iciclecount, wavetimer, enemyspike flag | enemycrab flag | enemyfly flag
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			//roundpoints, enemyflag, spawntimer
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{-1, 0, 0}
		},
	},
	{true, false, BG_SNOWY, HAS_NO_FIREBALLS, PLATFORMS_ARE_INVISIBLE, 0, 0, 0,
		{
			{-1, 0, 0}
		},
	},
	//phase 24
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			//getup timer decreased
			{2, ENEMY_SPIKE, 1},
			{2, ENEMY_SPIKE | ENEMY_FLY, 1},
			{1, ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{1, ENEMY_CRAB, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{-1, 0, 0}
		},
	},
	//phase 27
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 7, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			//wavetimer decreased from 10 to 7
			{2, ENEMY_SPIKE, 1},
			{2, ENEMY_SPIKE | ENEMY_FLY, 1},
			{1, ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{1, ENEMY_CRAB, 1},
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{2, ENEMY_CRAB, 1},
			{2, ENEMY_CRAB | ENEMY_FLY, 1},
			{-1, 0, 0}
		},
	},
	//phase 30
	{true, false, BG_SNOWY, HAS_NO_FIREBALLS, PLATFORMS_ARE_INVISIBLE, 0, 0, 0,
		{
			{-1, 0, 0}
		},
	},
	{false, true, BG_SNOWY, FIREBALL_GREEN | FIREBALL_RED, TOP_ICY, 2, 10, ENEMY_SPIKE | ENEMY_CRAB | ENEMY_FLY,
		{
			//getup timer decreased
			{2, ENEMY_SPIKE, 1},
			{2, ENEMY_SPIKE | ENEMY_FLY, 1},
			{1, ENEMY_SPIKE, 1},
			{-1, 0, 0}
		},
	},
};