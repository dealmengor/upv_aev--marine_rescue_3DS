#include "marine_rescue.h"

/* Globals */

/* Socoreboard Variables */
int GAME_STATUS = START_GAMESTATE;
int points = START_POINTS;
int level = START_LEVEL;
int lb_speedometer = START_SPEEDOMETER;
double diff_t = 0;

/* Element Counters */
int castawaycount = 0;
int sharpedocount = 0;
int castawaysaved = 0;

/*Structures & Data Structures Declaratation*/
static Castaway castaways[MAX_CASTAWAYS];
static Sharpedo sharpedos[MAX_SHARPEDOS];
static Sea sea;
static Lifeboat lifeboat;
Lifeboat *lboat = &lifeboat;
static CoastGuardShip coastguardship;
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
		// Random image, position, rotation and SPEED
		C2D_SpriteFromSheet(&castaway->spr, castaways_spriteSheet, rand() % 2);
		C2D_SpriteSetCenter(&castaway->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&castaway->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&castaway->spr, C3D_Angle(rand() / (float)RAND_MAX));
		castaway->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		castaway->dy = rand() * 4.0f / RAND_MAX - 2.0f;
		castaway->alive = false;
		castaway->picked_up = false;
		castawaycount += 1;
	}
}

void init_sharpedo()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sprite = &sharpedos[sharpedocount];
		if (i >= MEGA_SHARPEDO_LEVEL_SPAWN - 1)
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

void init_lifeboat(int lifes)
{
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&lboat->spr, coastguard_spriteSheet, 0);
	C2D_SpriteSetCenter(&lboat->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&lboat->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
	C2D_SpriteSetRotationDegrees(&lboat->spr, 0);
	lboat->dx = 0;
	lboat->dy = 0;
	lboat->speed = BOAT_SPEED;
	lboat->alive = true;
	lboat->seatcount = BOAT_SEAT_COUNT;
	lboat->fuel = 60;
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

/* Motion Functions */
void moveSprites_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if ((castaway->alive == true))
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
		Sharpedo *sprite = &sharpedos[i];
		C2D_SpriteMove(&sprite->spr, sprite->dx, sprite->dy);
		C2D_SpriteRotateDegrees(&sprite->spr, 1.0f);

		// Check for collision with the screen boundaries
		if ((sprite->spr.params.pos.x < sprite->spr.params.pos.w / 2.0f && sprite->dx < 0.0f) ||
			(sprite->spr.params.pos.x > (TOP_SCREEN_WIDTH - (sprite->spr.params.pos.w / 2.0f)) && sprite->dx > 0.0f))
			sprite->dx = -sprite->dx;

		if ((sprite->spr.params.pos.y < sprite->spr.params.pos.h / 2.0f && sprite->dy < 0.0f) ||
			(sprite->spr.params.pos.y > (TOP_SCREEN_HEIGHT - (sprite->spr.params.pos.h / 2.0f)) && sprite->dy > 0.0f))
			sprite->dy = -sprite->dy;
	}
}

void moveSprite_coastguardship()
{
	C2D_SpriteMove(&cgship->spr, cgship->dx, cgship->dy);
	//Check for collision with the screen boundaries

	// Left Boundarie
	if (cgship->spr.params.pos.x < cgship->spr.params.pos.w - 300 && cgship->dx < 0.0f)
	{
		cgship->dx = -cgship->dx;
		cgship->spr.params.pos.x = TOP_SCREEN_WIDTH + 100;
	}
	// Right Boundarie
	else if (cgship->spr.params.pos.x > (TOP_SCREEN_WIDTH - (cgship->spr.params.pos.w / 2.0f)) && cgship->dx > 0.0f)
	{
		cgship->dx = -cgship->dx;
	}
}

void moveLifeboat_sprite()
{

	if ((lboat->alive == true))
	{
		//LEFT BORDER
		if (lboat->spr.params.pos.x < TOP_SCREEN_WIDTH && lboat->spr.params.pos.x > 0)
		{
			C2D_SpriteMove(&lboat->spr, lboat->dx, 0);
		}
		//RIGHT BORDER
		else
		{
			if (lboat->spr.params.pos.x >= TOP_SCREEN_WIDTH)
				C2D_SpriteMove(&lboat->spr, -lboat->speed, 0);
			else
				C2D_SpriteMove(&lboat->spr, lboat->speed, 0);
		}

		if (lboat->spr.params.pos.y <= TOP_SCREEN_HEIGHT && lboat->spr.params.pos.y > 0)
		{
			C2D_SpriteMove(&lboat->spr, 0, lboat->dy);
		}
		else
		{
			if (lboat->spr.params.pos.y > TOP_SCREEN_HEIGHT)
			{
				C2D_SpriteMove(&lboat->spr, 0, -lboat->speed);
			}
			else
				C2D_SpriteMove(&lboat->spr, 0, lboat->speed);
		}
		lboat->dx = 0;
		lboat->dy = 0;
	}
}

void moveLifeboatController(u32 kHeld)
{

	//UP
	if (kHeld & KEY_UP)
	{
		lboat->dy += -lboat->speed;
	}
	// //DOWN
	if (kHeld & KEY_DOWN)
	{
		lboat->dy += lboat->speed;
	}
	//LEFT
	if (kHeld & KEY_LEFT)
	{
		lboat->dx += -lboat->speed;
	}
	//RIGHT
	if (kHeld & KEY_RIGHT)
	{
		lboat->dx += lboat->speed;
	}
}

/* Lifeboat Controllers */
void lifeboatpickUp(Lifeboat *lboat, Castaway *castaway)
{
	if ((lboat->seatcount < 3) && (castaway->picked_up == false) && (lboat->alive = true))
	{
		lboat->seatcount = lboat->seatcount + 1;
		castaway->picked_up = true;
	}
}

void lifeboatDeath(Lifeboat *lboat)
{
	if (lboat->alive == true && lboat->lifes > 0)
	{
		lboat->lifes -= 1;
		init_lifeboat(lboat->lifes);
		// GAMEOVER
		if (lboat->lifes == 0)
		{
			lboat->alive = false;
			lboat->seatcount = BOAT_SEAT_COUNT;
			gameStatusController(GAMEOVER_GAMESTATE);
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
		if ((castaway->alive == false) || (castaway->picked_up == true))
		{
			castaway->alive = true;
			castaway->picked_up = false;
			break;
		}
	}
}

void spawnNewSharpedo()
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
}

/* Collision Functions */
void collisionsharpedo_Castaway()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		Castaway *castaway = &castaways[i];
		if ((sharpedo->stalking == false) || ((castaway->alive == true) && (castaway->picked_up == false)))
		{
			if (abs(sharpedo->spr.params.pos.x - castaways[i].spr.params.pos.x) < 10.0f &&
				abs(sharpedo->spr.params.pos.y - castaways[i].spr.params.pos.y) < 10.0f)
			{
				castaway->alive = false;
			}
		}
	}
}

void collisionsharpedo_Lifeboat()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sharpedo = &sharpedos[i];
		if (sharpedo->stalking == false)
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
		if ((castaway->alive == true) && (castaway->picked_up == false))
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
	if (lboat->alive == true)
	{
		if (abs(cgship->spr.params.pos.x - lboat->spr.params.pos.x) < 40.0f &&
			abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) < 40.0f)
		{
			//Lifeboat Fuel Recharge
			lboat->fuel = FUEL_RECHARGE;

			//Increase Points
			if (lboat->seatcount > 0)
			{
				for (size_t i = 0; i < lboat->seatcount; i++)
				{
					points += 10;
					castawaysaved += 1;
					if (points % 100 == 0)
					{
						gameStatusController(LEVEL_UP_GAMESTATE); //LEVEL UP!
					}
				}
				lboat->seatcount = 0;
			}
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
		if ((castaway->alive == true && castaway->picked_up == false))
			C2D_DrawSprite(&castaways[i].spr);
	}
}

void drawer_sharpedo()
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
	if ((lboat->alive == true))
	{
		C2D_DrawSprite(&lifeboat.spr);
	}
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
	char buf[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[BUFFER_SIZE], buf4[BUFFER_SIZE], buf5[BUFFER_SIZE];
	C2D_Text dynText_lifes, dynText_points, dynText_levels, dynText_passengers, dynText_fuel;

	snprintf(buf, sizeof(buf), "Vidas: %d ", lboat->lifes);
	snprintf(buf2, sizeof(buf2), "Puntos: %d ", points);
	snprintf(buf3, sizeof(buf3), "Nivel: %d ", level);
	snprintf(buf4, sizeof(buf4), "Pasajeros: %d / 3", lboat->seatcount);
	snprintf(buf5, sizeof(buf5), "Combustible: %d / 60", lboat->fuel);

	C2D_TextParse(&dynText_lifes, g_dynamicBuf, buf);
	C2D_TextParse(&dynText_points, g_dynamicBuf, buf2);
	C2D_TextParse(&dynText_levels, g_dynamicBuf, buf3);
	C2D_TextParse(&dynText_passengers, g_dynamicBuf, buf4);
	C2D_TextParse(&dynText_fuel, g_dynamicBuf, buf5);

	C2D_TextOptimize(&dynText_lifes);
	C2D_TextOptimize(&dynText_points);
	C2D_TextOptimize(&dynText_levels);
	C2D_TextOptimize(&dynText_passengers);
	C2D_TextOptimize(&dynText_fuel);

	C2D_DrawText(&dynText_levels, C2D_AtBaseline | C2D_WithColor, 16.0f, 130.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_points, C2D_AtBaseline | C2D_WithColor, 16.0f, 150.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_lifes, C2D_AtBaseline | C2D_WithColor, 16.0f, 170.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_passengers, C2D_AtBaseline | C2D_WithColor, 16.0f, 190.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_fuel, C2D_AtBaseline | C2D_WithColor, 16.0f, 210.0f, 0.5f, size, size, WHITE);

	//TESTING
	char testbuf[BUFFER_SIZE], testbuf2[BUFFER_SIZE], testbuf3[BUFFER_SIZE], testbuf4[BUFFER_SIZE], testbuf5[BUFFER_SIZE];
	C2D_Text t, t2, t3, t4, t5;

	snprintf(testbuf, sizeof(testbuf), "cgship.x: %f ", lboat->spr.params.pos.x);
	snprintf(testbuf2, sizeof(testbuf2), "cgship.y: %f ", lboat->spr.params.pos.y);
	snprintf(testbuf3, sizeof(testbuf3), "cgship.dx: %f ", lboat->dx);
	snprintf(testbuf4, sizeof(testbuf4), "cgship.dx: %f ", lboat->dy);
	snprintf(testbuf5, sizeof(testbuf5), "sharpedocount: %d ", sharpedocount);

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

	C2D_DrawText(&t, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 300.0f, 130.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t2, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 300.0f, 150.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t3, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 300.0f, 170.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t4, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 300.0f, 190.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&t5, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 300.0f, 210.0f, 0.5f, size, size, WHITE);
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

/* Game Controller */
void gameStatusController(int sentinel)
{
	switch (sentinel)
	{
	case GAMEOVER_GAMESTATE: //Lose Game
		GAME_STATUS = GAMEOVER_GAMESTATE;
		break;
	case START_GAMESTATE: //Start Game
		//TODO
		break;
	case LEVEL_UP_GAMESTATE: //Level Up
		level += 1;
		spawnNewSharpedo();
		GAME_STATUS = LEVEL_UP_GAMESTATE;
		break;
	case NEW_GAMESTATE: //New Game
		//TODO
		break;
	case WIN_GAMESTATE: //Win Game
		//TODO
		break;
	}
}

int main(int argc, char *argv[])
{
	// Init libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	srand(time(NULL)); // Sets a seed for random numbers

	// Timer
	time_t current_epoch_time, next_spawn;
	time(&current_epoch_time);
	next_spawn = current_epoch_time + 10;
	diff_t = difftime(next_spawn, current_epoch_time);

	// Create screens
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load graphics Spritesheets
	init_sprites();

	// Initialize sprites for Structures
	init_sea();
	init_castaways();
	init_sharpedo();
	init_lifeboat(BOAT_LIFES);
	init_coastguardship();

	// Initialize the scene for Scoreboard
	sceneInit_bottom();

	// Main loop
	while (aptMainLoop())
	{
		if (GAME_STATUS == GAMEOVER_GAMESTATE)
		{
			getchar(); // Pause program
		}
		else
		{
			hidScanInput();

			// Respond to user input
			u32 kDown = hidKeysDown();
			u32 kHeld = hidKeysHeld();

			//Timer
			time(&current_epoch_time);						   // Get current EPOCH time from System
			diff_t = difftime(next_spawn, current_epoch_time); // Time Difference
			if (diff_t == 0)
			{
				next_spawn = current_epoch_time + 10;
				spawnNewCastaway();
			}

			/* Control Interface Logic */
			// break in order to return to hbmenu
			if (kDown & KEY_START)
				break;

			// D-PAD Controller
			if (kHeld & KEY_UP || kHeld & KEY_DOWN || kHeld & KEY_LEFT || kHeld & KEY_RIGHT)
				moveLifeboatController(kHeld);

			// Move sprites
			moveSprites_castaways();
			moveSprites_sharpedos();
			moveLifeboat_sprite();
			moveSprite_coastguardship();

			// Collision Detectors
			collisionsharpedo_Castaway();
			collisionsharpedo_Lifeboat();
			collisionCastaway_Lifeboat();
			collisionCoastGuardShip_Lifeboat();

			/* Start Render the scene */
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

			/* TOP Screen */
			C2D_TargetClear(top, BLACK);
			C2D_SceneBegin(top);

			//Drawer Sprites
			drawer_sea();
			drawer_castaways();
			drawer_sharpedo();
			drawer_lifeboat();
			drawer_coastguardship();

			C2D_Flush(); // Ensures all 2D objects so far have been drawn.

			/* Bottom Screen */
			C2D_TargetClear(bottom, BLACK);
			C2D_SceneBegin(bottom);
			drawer_scoreboard(FONT_SIZE);

			C3D_FrameEnd(0); // Finish render the scene
		}
	}

	// Deinitialize the scene
	scenesExit();

	// Deinitialize libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
