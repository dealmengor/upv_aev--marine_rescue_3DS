#include "marine_rescue.h"

/* Globals */

/* Socoreboard Variables */
int game_status = MENU_GAMESTATE;
int points = START_POINTS;
int level = START_LEVEL;
int lb_speedometer = START_SPEEDOMETER;

/* Time Variables */
time_t current_epoch_time, initial_second, current_initial_paused_time, last_paused_time, game_time, next_spawn, next_fuel_consumption;
int diff_t[TIME_DIFFERENCE_QUANTITY]; //Save time differences
struct tm ts;						  //Time Structure
char time_buf[TIME_BUFFER_SIZE];	  //Buffer Convert from epoch to human-readable date

/* Element Counters */
int castawaycount = 0;
int sharpedocount = 0;
int castawaysaved = 0;

/*Structures & Data Structures Declaratation*/
static Castaway castaways[MAX_CASTAWAYS];
static Sharpedo sharpedos[MAX_SHARPEDOS];
static Sea sea;
static Lifeboat lifeboat;
static CoastGuardShip coastguardship;

Lifeboat *lboat = &lifeboat;
CoastGuardShip *cgship = &coastguardship;

/* Spritesheets Declaratation */
static C2D_SpriteSheet castaways_spriteSheet;
static C2D_SpriteSheet coastguard_spriteSheet;
static C2D_SpriteSheet sharpedo_spriteSheet;
static C2D_SpriteSheet sea_spriteSheet;

/* C2D_Text Declaration Variables */
C2D_TextBuf g_staticBuf, g_dynamicBuf;	  // Buffers Declaratation
C2D_Text g_staticText[STATIC_TEXT_COUNT]; // Array for Static Text

/* Initializer Functions */
void init_sprites()
{
	castaways_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/castaways.t3x");
	if (!castaways_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	coastguard_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/coastguard.t3x");
	if (!sea_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	sharpedo_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sharpedos.t3x");
	if (!sharpedo_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	sea_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sea.t3x");
	if (!sea_spriteSheet)
		svcBreak(USERBREAK_PANIC);
}

void init_sea()
{
	Sea *sprite = &sea;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, sea_spriteSheet, 0);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, 240.0f);
	sprite->dy = 1.0f;
}

void init_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[castawaycount];
		// SPEED, Random image, position & rotation
		C2D_SpriteFromSheet(&castaway->spr, castaways_spriteSheet, rand() % 2);
		C2D_SpriteSetCenter(&castaway->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&castaway->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&castaway->spr, C3D_Angle(rand() / (float)RAND_MAX));
		castaway->speed = MAX_CASTAWAYS_SPEED;
		castaway->dx = castaway->speed;
		castaway->dy = castaway->speed;
		castaway->visible = false;
		castawaycount += 1;
	}
}

void init_sharpedo()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sprite = &sharpedos[sharpedocount];
		if (i >= MEGA_SHARPEDO_SPAWN_LEVEL - 1)
		{
			C2D_SpriteFromSheet(&sprite->spr, sharpedo_spriteSheet, 1);
			sprite->speed = MAX_MEGA_SHARPEDOS_SPEED;
			sprite->dx = sprite->speed;
			sprite->dy = sprite->speed;
		}
		else
		{
			C2D_SpriteFromSheet(&sprite->spr, sharpedo_spriteSheet, 0);
			sprite->speed = MAX_SHARPEDOS_SPEED;
			sprite->dx = sprite->speed;
			sprite->dy = sprite->speed;
		}
		// Sprite, SPEED, Random position & rotation
		C2D_SpriteSetCenter(&sprite->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&sprite->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&sprite->spr, C3D_Angle(rand() / (float)RAND_MAX));
		sharpedocount += 1;
		if (i == 0)
		{
			sprite->stalking = false; // One Sharpedo for first level
		}
		else
		{
			sprite->stalking = true; // The rest in stalking mode
		}
	}
}

void init_lifeboat(int lifes, bool alive, int pos_x, int pos_y)
{
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&lboat->spr, coastguard_spriteSheet, 0);
	C2D_SpriteSetCenter(&lboat->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&lboat->spr, pos_x, pos_y);
	C2D_SpriteSetRotationDegrees(&lboat->spr, 0);
	lboat->dx = 0;
	lboat->dy = 0;
	lboat->speed = BOAT_SPEED;
	lboat->seatcount = BOAT_SEAT_COUNT;
	lboat->fuel = BOAT_FUEL_RECHARGE;
	lboat->alive = alive;
	lboat->lifes = lifes;
}

void init_coastguardship()
{
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&cgship->spr, coastguard_spriteSheet, 8);
	C2D_SpriteSetCenter(&cgship->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&cgship->spr, TOP_SCREEN_WIDTH + 100, TOP_SCREEN_HEIGHT - 30);
	C2D_SpriteSetRotationDegrees(&cgship->spr, 0);
	cgship->speed = 1;
	cgship->dx = cgship->speed;
	cgship->dy = 0;
}

/* Sprites Controller */
void controllerSprites_lifeboat(int sprite_id)
{
	// Saved last lifeboat position
	int last_pos_x, last_pos_y;
	last_pos_x = lboat->spr.params.pos.x;
	last_pos_y = lboat->spr.params.pos.y;

	// Sprite
	C2D_SpriteFromSheet(&lboat->spr, coastguard_spriteSheet, sprite_id);
	C2D_SpriteSetCenter(&lboat->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&lboat->spr, last_pos_x, last_pos_y);
	C2D_SpriteSetRotationDegrees(&lboat->spr, 0);
}

/* Motion Functions */
void moveSprites_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if ((castaway->visible == true))
		{
			C2D_SpriteMove(&castaway->spr, castaway->dx, castaway->dy);
			C2D_SpriteRotateDegrees(&castaway->spr, 1.0f);
		}

		// Check for collision with the screen boundaries
		if ((castaway->spr.params.pos.x < castaway->spr.params.pos.w / 2.0f && castaway->dx < 0.0f) ||
			(castaway->spr.params.pos.x > (TOP_SCREEN_WIDTH - (castaway->spr.params.pos.w / 2.0f)) && castaway->dx > 0.0f))
			castaway->dx = -castaway->dx;

		if ((castaway->spr.params.pos.y < castaway->spr.params.pos.h / 2.0f && castaway->dy < 0.0f) ||
			(castaway->spr.params.pos.y > (TOP_SCREEN_HEIGHT - (castaway->spr.params.pos.h / 2.0f)) && castaway->dy > 0.0f))
			castaway->dy = -castaway->dy;
	}
}

void moveSprites_sharpedos()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		C2D_SpriteMove(&sharpedo->spr, sharpedo->dx, sharpedo->dy);
		C2D_SpriteRotateDegrees(&sharpedo->spr, 1.0f);

		// Check for collision with the screen boundaries
		if ((sharpedo->spr.params.pos.x < sharpedo->spr.params.pos.w / 2.0f && sharpedo->dx < 0.0f) ||
			(sharpedo->spr.params.pos.x > (TOP_SCREEN_WIDTH - (sharpedo->spr.params.pos.w / 2.0f)) && sharpedo->dx > 0.0f))
			sharpedo->dx = -sharpedo->dx;

		if ((sharpedo->spr.params.pos.y < sharpedo->spr.params.pos.h / 2.0f && sharpedo->dy < 0.0f) ||
			(sharpedo->spr.params.pos.y > (TOP_SCREEN_HEIGHT - (sharpedo->spr.params.pos.h / 2.0f)) && sharpedo->dy > 0.0f))
			sharpedo->dy = -sharpedo->dy;
	}
}

void moveSprite_coastguardship()
{
	C2D_SpriteMove(&cgship->spr, cgship->dx, cgship->dy);
	//Check for collision with the screen boundaries

	// Left Boundary
	if (cgship->spr.params.pos.x < cgship->spr.params.pos.w - 300 && cgship->dx < 0.0f)
	{
		cgship->dx = -cgship->dx;
		cgship->spr.params.pos.x = TOP_SCREEN_WIDTH + 100;
	}
	// Right Boundary
	else if (cgship->spr.params.pos.x > (TOP_SCREEN_WIDTH - (cgship->spr.params.pos.w / 2.0f)) && cgship->dx > 0.0f)
	{
		cgship->dx = -cgship->dx;
	}

	// Check if a new lifeboat is needed as long as the coast guard ship is in the visible sea.
	if ((lboat->alive == false) && ((cgship->spr.params.pos.x / 2.0f <= TOP_SCREEN_WIDTH - cgship->spr.params.pos.x / 2.0f) && cgship->spr.params.pos.x / 2.0f >= 0.0f))
	{
		init_lifeboat(lboat->lifes, true, cgship->spr.params.pos.x, cgship->spr.params.pos.y - lboat->spr.params.pos.h);
	}
}

void moveSprite_Lifeboat()
{
	if ((lboat->alive == true))
	{
		// Move sprite on X axis while is on the screen
		if (lboat->spr.params.pos.x < BOAT_TOP_SCREEN_WIDTH && lboat->spr.params.pos.x >= 20)
		{
			C2D_SpriteMove(&lboat->spr, lboat->dx, 0);
		}
		else
		{
			//RIGTH CORNER
			if (lboat->spr.params.pos.x >= BOAT_TOP_SCREEN_WIDTH)
				C2D_SpriteMove(&lboat->spr, -lboat->speed, 0);
			//LEFT CORNER
			else
				C2D_SpriteMove(&lboat->spr, lboat->speed, 0);
		}

		// Move sprite on Y axis while is on the screen
		if (lboat->spr.params.pos.y <= BOAT_TOP_SCREEN_HEIGHT && lboat->spr.params.pos.y >= 20)
		{
			C2D_SpriteMove(&lboat->spr, 0, lboat->dy);
		}
		else
		{
			//BOTTOM CORNER
			if (lboat->spr.params.pos.y >= BOAT_TOP_SCREEN_HEIGHT)
			{
				C2D_SpriteMove(&lboat->spr, 0, -lboat->speed);
			}
			//UPPER CORNER
			else
			{
				C2D_SpriteMove(&lboat->spr, 0, lboat->speed);
			}
		}
		lboat->dx = 0;
		lboat->dy = 0;
	}
}

void moveLifeboatController(u32 kHeld)
{
	if (lboat->alive == true && lboat->fuel > 0)
	{
		// NORTH
		if (kHeld & KEY_UP)
		{
			lboat->dy += -lboat->speed;
			controllerSprites_lifeboat(NORTH_LIFEBOAT1);
		}
		// SOUTH
		if (kHeld & KEY_DOWN)
		{
			lboat->dy += lboat->speed;
			controllerSprites_lifeboat(SOUTH_LIFEBOAT5);
		}
		// WEST
		if (kHeld & KEY_LEFT)
		{
			lboat->dx += -lboat->speed;
			controllerSprites_lifeboat(WEST_LIFEBOAT7);
		}
		// EAST
		if (kHeld & KEY_RIGHT)
		{
			lboat->dx += lboat->speed;
			controllerSprites_lifeboat(EAST_LIFEBOAT3);
		}

		// NORTHEAST
		if ((kHeld & KEY_UP) && (kHeld & KEY_RIGHT))
		{
			controllerSprites_lifeboat(NORTHEAST_LIFEBOAT2);
		}
		// NORTHWEST
		if ((kHeld & KEY_UP) && (kHeld & KEY_LEFT))
		{
			controllerSprites_lifeboat(NORTHWEST_LIFEBOAT8);
		}
		// SOUTEAST
		if ((kHeld & KEY_DOWN) && (kHeld & KEY_RIGHT))
		{
			controllerSprites_lifeboat(SOUTHEAST_LIFEBOAT4);
		}
		// SOUTHWEST
		if ((kHeld & KEY_DOWN) && (kHeld & KEY_LEFT))
		{
			controllerSprites_lifeboat(SOUTHWEST_LIFEBOAT6);
		}
	}
}

/* Bounce Controllers */
void bounceCastaway_Coastguardship(Castaway *castaway)
{
	if (castaway->visible == true)
	{
		castaway->dx = -castaway->dx;
		castaway->dy = -castaway->dy;
	}
}

void bounceSharpedo_Coastguardship(Sharpedo *sharpedo)
{
	sharpedo->dx = -sharpedo->dx;
	sharpedo->dy = -sharpedo->dy;
}

void bounceCoastGuardShip_Lifeboat()
{
	//Lifeboat Fuel Recharge
	lboat->fuel = BOAT_FUEL_RECHARGE;

	// Validate if there're passengers on the boat
	if (lboat->seatcount > 0)
	{
		//Increase Points
		for (size_t i = 0; i < lboat->seatcount; i++)
		{
			points += RESCUE_POINTS;
			castawaysaved += 1;
			if (points % NEXT_LEVEL == 0)
			{
				gameStatusController(LEVEL_UP_GAMESTATE, TIME_CONTINUITY); //LEVEL UP!
			}
		}
		lboat->seatcount = 0;
	}
}

/* Lifeboat Controllers */
void lifeboatpickUp(Lifeboat *lboat, Castaway *castaway)
{
	if ((lboat->alive == true) && (lboat->seatcount < 3) && (castaway->visible == true))
	{
		lboat->seatcount += 1;
		castaway->visible = false;
	}
}

void lifeboatDeath(Lifeboat *lboat)
{
	if (lboat->lifes > 0)
	{
		lboat->lifes -= 1;
		lboat->alive = false;
		// GAMEOVER
		if (lboat->lifes == 0)
		{
			lboat->seatcount = BOAT_SEAT_COUNT;
			gameStatusController(GAMEOVER_GAMESTATE, STOP_TIME_CONTINUITY);
		}
	}
}

/* Spawn Controllers */
void spawnNewCastaway()
{
	//Check in the castaway array to restore the status of the elements.
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if (castaway->visible == false)
		{
			castaway->visible = true;
			break;
		}
	}
}

void spawnNewSharpedo()
{
	if (game_status == LEVEL_UP_GAMESTATE)
	{
		//Check in the castaway array to restore the status of the elements.
		for (size_t i = 0; i < MAX_SHARPEDOS; i++)
		{
			Sharpedo *sharpedo = &sharpedos[i];
			if (sharpedo->stalking == true)
			{
				sharpedo->stalking = false;
				break;
			}
		}
		gameStatusController(START_GAMESTATE, TIME_CONTINUITY);
	}
}

/* Collision Functions */
// void collisionSharpedo_Sharpedo()
// {
// 	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
// 	{
// 		Shark *shark = &sharpedos[i];
// 		Shark *shark2 = &sharpedos[i];

// 		if (abs(shark->spr.params.pos.x - shark2[i].spr.params.pos.x) < 20.0f &&
// 			abs(shark->spr.params.pos.y - shark2[i].spr.params.pos.y) < 20.0f)
// 		// &&shark->id != shark2[i].id
// 		{
// 			shark->dx = -shark->dx;
// 			shark->dy = -shark->dy;
// 		}
// 	}
// }

void collisionSharpedo_Castaway()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		Castaway *castaway = &castaways[i];
		if ((sharpedo->stalking == false) && (castaway->visible == true))
		{
			if (abs(sharpedo->spr.params.pos.x - castaways[i].spr.params.pos.x) < 25.0f &&
				abs(sharpedo->spr.params.pos.y - castaways[i].spr.params.pos.y) < 25.0f)
			{
				castaway->visible = false;
			}
		}
	}
}

void collisionSharpedo_Lifeboat()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		if ((sharpedo->stalking == false) && (lboat->alive == true))
		{
			if (abs(sharpedo->spr.params.pos.x - lboat->spr.params.pos.x) < 20.0f &&
				abs(sharpedo->spr.params.pos.y - lboat->spr.params.pos.y) < 20.0f)
			{
				lifeboatDeath(lboat);
			}
		}
	}
}

void collisionCastaway_Lifeboat()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if ((castaway->visible == true) && (lboat->alive == true))
		{
			if (abs(castaway->spr.params.pos.x - lboat->spr.params.pos.x) < 20.0f &&
				abs(castaway->spr.params.pos.y - lboat->spr.params.pos.y) < 20.0f)
			{
				lifeboatpickUp(lboat, castaway);
			}
		}
	}
}

void collisionCoastGuardShip_Lifeboat()
{
	// Lifeboat current status Check
	if (lboat->alive == true)
	{
		// Collision Check
		if (abs(cgship->spr.params.pos.x - lboat->spr.params.pos.x) == 60.0f &&
			abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) < 13.0f)
		{
			// Bounce
			bounceCoastGuardShip_Lifeboat();
			//LEFT SHIP
			C2D_SpriteMove(&lboat->spr, -lboat->speed, 0);
		}
		else if (cgship->spr.params.pos.x - lboat->spr.params.pos.x == -60.0f &&
				 abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) < 13.0f)
		{
			// Bounce
			bounceCoastGuardShip_Lifeboat();
			//RIGTH SHIP
			C2D_SpriteMove(&lboat->spr, lboat->speed, 0);
		}
		else if (abs(cgship->spr.params.pos.x - lboat->spr.params.pos.x) <= 50.0f &&
				 abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) <= 30.0f)
		{
			// Bounce
			bounceCoastGuardShip_Lifeboat();
			//UPPER SHIP
			C2D_SpriteMove(&lboat->spr, 0, -lboat->speed);
		}
	}
}

void collisionCastaway_Coastguardship()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if (abs(castaway->spr.params.pos.x - cgship->spr.params.pos.x) < 40.0f &&
			abs(castaway->spr.params.pos.y - cgship->spr.params.pos.y) < 30.0f)
		{
			bounceCastaway_Coastguardship(castaway);
		}
	}
}

void collisionSharpedo_Coastguardship()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		if (abs(sharpedo->spr.params.pos.x - cgship->spr.params.pos.x) < 40.0f &&
			abs(sharpedo->spr.params.pos.y - cgship->spr.params.pos.y) < 30.0f)
		{
			bounceSharpedo_Coastguardship(sharpedo);
		}
	}
}

/* Drawer Functions */
void drawer_sea()
{
	C2D_DrawSprite(&sea.spr);
}

void drawer_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if (castaway->visible == true)
			C2D_DrawSprite(&castaways[i].spr);
	}
}

void drawer_sharpedos()
{
	for (size_t i = 0; i < sharpedocount; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		if (sharpedo->stalking == false)
			C2D_DrawSprite(&sharpedos[i].spr);
	}
}

void drawer_lifeboat()
{
	if (lboat->alive == true)
		C2D_DrawSprite(&lifeboat.spr);
}

void drawer_coastguardship()
{
	C2D_DrawSprite(&coastguardship.spr);
}

void drawer_scoreboard(float size)
{
	// Clear the dynamic text buffer
	C2D_TextBufClear(g_dynamicBuf);

	// Draw static text strings
	C2D_DrawText(&g_staticText[0], C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, 150.0f, 25.0f, 0.5f, size, size, WHITE);

	// Generate and draw dynamic text
	char buf[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[BUFFER_SIZE], buf4[BUFFER_SIZE], buf5[BUFFER_SIZE], buf6[BUFFER_SIZE];
	C2D_Text dynText_lifes, dynText_points, dynText_levels, dynText_passengers, dynText_fuel, dynText_time;

	snprintf(buf, sizeof(buf), "Lifes: %d ", lboat->lifes);
	snprintf(buf2, sizeof(buf2), "Points: %d ", points);
	snprintf(buf3, sizeof(buf3), "Level: %d ", level);
	snprintf(buf4, sizeof(buf4), "Passengers: %d / 3", lboat->seatcount);
	snprintf(buf5, sizeof(buf5), "Fuel: %d / 15", lboat->fuel);
	snprintf(buf6, sizeof(buf6), "Time: %s ", time_buf);

	C2D_TextParse(&dynText_lifes, g_dynamicBuf, buf);
	C2D_TextParse(&dynText_points, g_dynamicBuf, buf2);
	C2D_TextParse(&dynText_levels, g_dynamicBuf, buf3);
	C2D_TextParse(&dynText_passengers, g_dynamicBuf, buf4);
	C2D_TextParse(&dynText_fuel, g_dynamicBuf, buf5);
	C2D_TextParse(&dynText_time, g_dynamicBuf, buf6);

	C2D_TextOptimize(&dynText_lifes);
	C2D_TextOptimize(&dynText_points);
	C2D_TextOptimize(&dynText_levels);
	C2D_TextOptimize(&dynText_passengers);
	C2D_TextOptimize(&dynText_fuel);
	C2D_TextOptimize(&dynText_time);

	C2D_DrawText(&dynText_levels, C2D_AtBaseline | C2D_WithColor, 16.0f, 110.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_points, C2D_AtBaseline | C2D_WithColor, 16.0f, 130.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_lifes, C2D_AtBaseline | C2D_WithColor, 16.0f, 150.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_passengers, C2D_AtBaseline | C2D_WithColor, 16.0f, 170.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_fuel, C2D_AtBaseline | C2D_WithColor, 16.0f, 190.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_time, C2D_AtBaseline | C2D_WithColor, 16.0f, 210.0f, 0.5f, size, size, WHITE);

	//TESTING
	char testbuf[BUFFER_SIZE], testbuf2[BUFFER_SIZE], testbuf3[BUFFER_SIZE], testbuf4[BUFFER_SIZE], testbuf5[BUFFER_SIZE];
	C2D_Text t, t2, t3, t4, t5;

	snprintf(testbuf, sizeof(testbuf), "diff1: %d, diff2:  %d", diff_t[1], diff_t[2]);
	snprintf(testbuf2, sizeof(testbuf2), "current_epoch: %lld ", current_epoch_time);
	snprintf(testbuf3, sizeof(testbuf3), "initial_paused: %lld ", current_initial_paused_time);
	snprintf(testbuf4, sizeof(testbuf4), "initial_second: %lld ", initial_second);
	snprintf(testbuf5, sizeof(testbuf5), "total_time_pause: %d ", diff_t[0]);

	C2D_TextParse(&t, g_dynamicBuf, testbuf);
	C2D_TextParse(&t2, g_dynamicBuf, testbuf2);
	C2D_TextParse(&t3, g_dynamicBuf, testbuf3);
	C2D_TextParse(&t4, g_dynamicBuf, testbuf4);
	C2D_TextParse(&t5, g_dynamicBuf, testbuf5);

	C2D_TextOptimize(&t);
	C2D_TextOptimize(&t2);
	C2D_TextOptimize(&t3);
	C2D_TextOptimize(&t4);
	C2D_TextOptimize(&t5);

	C2D_DrawText(&t, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 305.0f, 130.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t2, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 305.0f, 150.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t3, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 305.0f, 170.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t4, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 305.0f, 190.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t5, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 305.0f, 210.0f, 0.5f, size, size, WHITE);
}

/* System Functions */
void sceneInit_bottom()
{
	// Create two text buffers: one for static text, and another one for
	// dynamic text - the latter will be cleared at each frame.
	g_staticBuf = C2D_TextBufNew(4096); // support up to 4096 glyphs in the buffer
	g_dynamicBuf = C2D_TextBufNew(4096);

	// Parse the static text strings
	C2D_TextParse(&g_staticText[0], g_staticBuf, "Lifeboats!");

	// Optimize the static text strings
	C2D_TextOptimize(&g_staticText[0]);
}

void scenesExit()
{
	// Delete the text buffers
	C2D_TextBufDelete(g_dynamicBuf);
	C2D_TextBufDelete(g_staticBuf);

	// Delete graphics
	C2D_SpriteSheetFree(castaways_spriteSheet);
	C2D_SpriteSheetFree(sharpedo_spriteSheet);
	C2D_SpriteSheetFree(coastguard_spriteSheet);
	C2D_SpriteSheetFree(sea_spriteSheet);
}

/* Game Controllers */
void gameStatusController(int game_sentinel, int time_sentinel)
{
	switch (game_sentinel)
	{
	case EXIT_GAMESTATE:
		game_status = EXIT_GAMESTATE;
		break;

	case START_GAMESTATE:
		game_status = START_GAMESTATE;
		gameTimeController(time_sentinel, game_status); // Start Stopwatch
		break;

	case NEW_GAMESTATE:
		game_status = NEW_GAMESTATE;
		break;

	case PAUSED_GAMESTATE:
		game_status = PAUSED_GAMESTATE;
		gameTimeController(time_sentinel, game_status); // Record Start Pause time
		break;

	case LEVEL_UP_GAMESTATE:
		game_status = LEVEL_UP_GAMESTATE;
		level += 1;
		spawnNewSharpedo();
		break;

	case GAMEOVER_GAMESTATE:
		game_status = GAMEOVER_GAMESTATE;
		break;

	case WIN_GAMESTATE:
		game_status = WIN_GAMESTATE;
		break;

	case MENU_GAMESTATE:
		game_status = MENU_GAMESTATE;
		break;
	}
}

void gameTimeController(int time_sentinel, int game_sentinel)
{
	// Initialize the time variables
	if ((time_sentinel == INITIAL_TIME_STATE) && (game_sentinel == START_GAMESTATE))
	{
		initial_second = time(&current_epoch_time);
		next_spawn = initial_second + CASTAWAY_SPAWN;					// Add 10 seconds to the next spawn
		next_fuel_consumption = initial_second + BOAT_FUEL_CONSUMPTION; // Add 2 seconds to the next fuel consumption
	}
	else if ((time_sentinel == INTIAL_PAUSED_TIME) && (game_sentinel == PAUSED_GAMESTATE))
	{
		current_initial_paused_time = time(&current_epoch_time); // Get current EPOCH time from System
	}
	else
	{
		// Get current EPOCH time from System
		time(&current_epoch_time);

		/* Game Stopwatch */
		if (current_initial_paused_time != 0 && current_initial_paused_time > last_paused_time)
		{
			diff_t[0] += difftime(current_epoch_time, current_initial_paused_time); // Total pause time
			next_spawn = current_epoch_time + CASTAWAY_SPAWN - diff_t[2];
			next_fuel_consumption = current_epoch_time + BOAT_FUEL_CONSUMPTION;
			last_paused_time = current_initial_paused_time;
		}

		// Calculate elapsed time
		game_time = current_epoch_time - initial_second - diff_t[0];
		// Format time, "hh:mm:ss"
		ts = *localtime(&game_time);
		strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &ts);

		/* Spawn mechanics */
		// Time Differences between current epoch time, next_spawn and next_fuel_consumption
		diff_t[1] = difftime(next_spawn, current_epoch_time);
		diff_t[2] = difftime(next_fuel_consumption, current_epoch_time);
	}
}

void gameInputController(int game_sentinel, u32 kDown, u32 kHeld)
{
	// General GAMESTATE Control
	if ((kDown & KEY_L) && (kDown & KEY_R))
		gameStatusController(EXIT_GAMESTATE, STOP_TIME_CONTINUITY); // Break in order to return to hbmenu

	// Menu GAMESTATE Controls
	if (game_sentinel == MENU_GAMESTATE)
	{
		if (kDown & KEY_START)
			gameStatusController(START_GAMESTATE, INITIAL_TIME_STATE);
	}

	// START GAMESTATE Controls
	if (game_sentinel == START_GAMESTATE)
	{
		// D-PAD Controller
		if (kHeld & KEY_UP || kHeld & KEY_DOWN || kHeld & KEY_LEFT || kHeld & KEY_RIGHT)
			moveLifeboatController(kHeld);
		// Pause Game
		if (kDown & KEY_SELECT)
			gameStatusController(PAUSED_GAMESTATE, INTIAL_PAUSED_TIME);
	}

	// Pause GAMESTATE Controls
	if (game_sentinel == PAUSED_GAMESTATE)
	{
		if (kDown & KEY_SELECT)
			gameStatusController(START_GAMESTATE, INTIAL_PAUSED_TIME);
	}
}

void gameInitController()
{
	init_sea();
	init_castaways();
	init_sharpedo();
	init_coastguardship();
	init_lifeboat(BOAT_LIFES, false, 0, 0);
}

void gameMoveSpritesController()
{
	moveSprites_castaways();
	moveSprites_sharpedos();
	moveSprite_Lifeboat();
	moveSprite_coastguardship();
}

void gameCollisionsController()
{
	// collisionSharpedo_Sharpedo();
	collisionSharpedo_Castaway();
	collisionSharpedo_Lifeboat();
	collisionSharpedo_Coastguardship();
	collisionCastaway_Lifeboat();
	collisionCoastGuardShip_Lifeboat();
	collisionCastaway_Coastguardship();
}

void gameDrawersTopScreenController(int game_sentinel)
{
	if (game_sentinel == START_GAMESTATE || game_sentinel == PAUSED_GAMESTATE)
	{
		drawer_sea();
		drawer_castaways();
		drawer_sharpedos();
		drawer_lifeboat();
		drawer_coastguardship();
	}
}

void gameDrawersBottomScreenController(int game_sentinel)
{
	// if (game_sentinel == START_GAMESTATE || game_sentinel == PAUSED_GAMESTATE)
	drawer_scoreboard(FONT_SIZE);
}

/* Main Function */
int main(int argc, char *argv[])
{
	// Init libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	srand(time(NULL)); // Sets a seed for random numbers

	// Create screens
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load graphics Spritesheets
	init_sprites();

	// Initialize sprites for Structures
	gameInitController();

	// Initialize the scene for Scoreboard
	sceneInit_bottom();

	// Main loop
	while (aptMainLoop())
	{
		/* Control Interface Logic */
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		gameInputController(game_status, kDown, kHeld);

		// General Game Status Validatation
		if (game_status == EXIT_GAMESTATE)
			goto exit_main_loop; // Close game

		if (game_status == START_GAMESTATE)
		{

			/* Time Controller */
			gameTimeController(TIME_CONTINUITY, game_status);

			// Time mechanics
			if (diff_t[1] == 0)
			{
				spawnNewCastaway();
				next_spawn = current_epoch_time + CASTAWAY_SPAWN;
			}
			if (diff_t[2] == 0)
			{
				if (lboat->fuel > 0)
					lboat->fuel -= BOAT_FUEL_CONSUMPTION;
				next_fuel_consumption = current_epoch_time + BOAT_FUEL_CONSUMPTION;
			}

			/* Move sprites */
			gameMoveSpritesController();

			/* Collision Detectors */
			gameCollisionsController();
		}

		/* Start Render the scene for both screens */
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		/* TOP Screen */
		C2D_TargetClear(top, BLACK);
		C2D_SceneBegin(top);

		//Drawer TOP Sprites
		gameDrawersTopScreenController(game_status);

		C2D_Flush(); // Ensures all 2D objects so far have been drawn.

		/* Bottom Screen */
		C2D_TargetClear(bottom, BLACK);
		C2D_SceneBegin(bottom);

		//Drawer BOTTOM Sprites
		gameDrawersBottomScreenController(game_status);

		/* Finish render the scene */
		C3D_FrameEnd(0);
	}

exit_main_loop:
	// Deinitialize the scene
	scenesExit();

	// Deinitialize libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
