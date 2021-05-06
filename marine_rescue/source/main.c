#include "marine_rescue.h"

/* Globals */

/* Socoreboard Variables */
bool GAME_START = false;
int points;
int level;
int lb_speedometer;
double diff_t = 0;

/* Element Counters */
int castawaycount;
int sharkcount;
int castawaysaved;

/*Structures & Data Structures Declaratation*/
static Castaway castaways[MAX_CASTAWAY];
static Shark sharks[MAX_SHARKS];
static Sea sea;
static Lifeboat lifeboat;
static CoastGuardShip coastguardship;

Lifeboat *lboat = &lifeboat;
CoastGuardShip *cgship = &coastguardship;

/* Spritesheets Declaratation */
static C2D_SpriteSheet castaways_spriteSheet;
static C2D_SpriteSheet coastguard_spriteSheet;
static C2D_SpriteSheet sharks_spriteSheet;
static C2D_SpriteSheet sea_spriteSheet;

/* C2D_Text Declaration Variables */
C2D_TextBuf g_staticBuf, g_dynamicBuf;	  // Buffers Declaratation
C2D_Text g_staticText[STATIC_TEXT_COUNT]; // Array for Static Text

/* Initializer Functions */
static void init_sprites()
{
	castaways_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/castaways.t3x");
	if (!castaways_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	coastguard_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/coastguard.t3x");
	if (!sea_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	sharks_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sharks.t3x");
	if (!sharks_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	sea_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sea.t3x");
	if (!sea_spriteSheet)
		svcBreak(USERBREAK_PANIC);
}

static void init_sea()
{
	Sea *sprite = &sea;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, sea_spriteSheet, 0);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, 240.0f);
	sprite->dy = 1.0f;
}

static void init_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
	{
		Castaway *castaway = &castaways[i];
		// Random image, position, rotation and SPEED
		C2D_SpriteFromSheet(&castaway->spr, castaways_spriteSheet, rand() % 1);
		C2D_SpriteSetCenter(&castaway->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&castaway->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&castaway->spr, C3D_Angle(rand() / (float)RAND_MAX));
		castaway->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		castaway->dy = rand() * 4.0f / RAND_MAX - 2.0f;
		castaway->alive = true;
		castaway->picked_up = false;
	}
}

static void init_sharks()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *shark = &sharks[i];
		// Random image, position, rotation and SPEED
		C2D_SpriteFromSheet(&shark->spr, sharks_spriteSheet, rand() % 1);
		C2D_SpriteSetCenter(&shark->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&shark->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&shark->spr, C3D_Angle(rand() / (float)RAND_MAX));
		shark->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		shark->dy = rand() * 4.0f / RAND_MAX - 2.0f;
		shark->id = i;
	}
}

static void init_lifeboat()
{
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&lboat->spr, coastguard_spriteSheet, 0);
	C2D_SpriteSetCenter(&lboat->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&lboat->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
	C2D_SpriteSetRotationDegrees(&lboat->spr, 0);
	lboat->dx = rand() * 4.0f / RAND_MAX - 2.0f;
	lboat->dy = rand() * 4.0f / RAND_MAX - 2.0f;
	lboat->speed = BOAT_SPEED;
	lboat->alive = true;
	lboat->seatcount = BOAT_SEAT_COUNT;

	if ((GAME_START == false))
	{
		GAME_START = true;
		lboat->lifes = BOAT_LIFES;
	}
}

static void init_coastguardship()
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
static void moveSprites_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
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

static void moveSprites_sharks()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *shark = &sharks[i];
		C2D_SpriteMove(&shark->spr, shark->dx, shark->dy);
		C2D_SpriteRotateDegrees(&shark->spr, 1.0f);

		// Check for collision with the screen boundaries
		if ((shark->spr.params.pos.x < shark->spr.params.pos.w / 2.0f && shark->dx < 0.0f) ||
			(shark->spr.params.pos.x > (TOP_SCREEN_WIDTH - (shark->spr.params.pos.w / 2.0f)) && shark->dx > 0.0f))
			shark->dx = -shark->dx;

		if ((shark->spr.params.pos.y < shark->spr.params.pos.h / 2.0f && shark->dy < 0.0f) ||
			(shark->spr.params.pos.y > (TOP_SCREEN_HEIGHT - (shark->spr.params.pos.h / 2.0f)) && shark->dy > 0.0f))
			shark->dy = -shark->dy;
	}
}

static void moveSprite_coastguardship()
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

static void moveLifeboat_sprite()
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

static void moveLifeboatController(u32 kHeld)
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

/* Bounce Controllers */
static void bounceCastaway_Coastguardship(Castaway *castaway)
{
	if ((castaway->picked_up == false) && (castaway->alive = true))
	{
		castaway->dx = -castaway->dx;
		castaway->dy = -castaway->dy;
	}
}

/* Life Controllers */
static void lifeboatpickUp(Lifeboat *lboat, Castaway *castaway)
{
	if ((lboat->seatcount < 3) && (castaway->picked_up == false) && (lboat->alive = true))
	{
		lboat->seatcount = lboat->seatcount + 1;
		castaway->picked_up = true;
	}
}

static void lifeboatDeath(Lifeboat *lboat)
{
	if ((lboat->alive == true))
	{
		lboat->lifes = lboat->lifes - 1;
		lboat->fuel = 60;
	}
	if ((lboat->lifes > 0))
	{
		init_lifeboat();
	}
	else
	{
		lboat->alive = false;
		lboat->seatcount = BOAT_SEAT_COUNT;
		//TODO: Call gameover function
	}
}

/* Spawn Controllers */
static void spawnNewCastaway()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
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

/* Collision Functions */
static void collisionShark_Shark()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *shark = &sharks[i];
		Shark *shark2 = &sharks[i];

		if (abs(shark->spr.params.pos.x - shark2[i].spr.params.pos.x) < 20.0f &&
			abs(shark->spr.params.pos.y - shark2[i].spr.params.pos.y) < 20.0f)
		// &&shark->id != shark2[i].id
		{
			shark->dx = -shark->dx;
			shark->dy = -shark->dy;
		}
	}
}

static void collisionShark_Castaway()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *shark = &sharks[i];
		Castaway *castaway = &castaways[i];

		if (abs(shark->spr.params.pos.x - castaways[i].spr.params.pos.x) < 20.0f &&
			abs(shark->spr.params.pos.y - castaways[i].spr.params.pos.y) < 20.0f)
		{
			castaway->alive = false;
		}
	}
}

static void collisionShark_Lifeboat()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *shark = &sharks[i];

		if (abs(shark->spr.params.pos.x - lboat->spr.params.pos.x) < 20.0f &&
			abs(shark->spr.params.pos.y - lboat->spr.params.pos.y) < 20.0f)
		{
			lifeboatDeath(lboat);
		}
	}
}

static void collisionCastaway_Lifeboat()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
	{
		Castaway *castaway = &castaways[i];
		if (abs(castaway->spr.params.pos.x - lboat->spr.params.pos.x) < 20.0f &&
			abs(castaway->spr.params.pos.y - lboat->spr.params.pos.y) < 20.0f)
		{
			lifeboatpickUp(lboat, castaway);
		}
	}
}

static void collisionCastaway_Coastguardship()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
	{
		Castaway *castaway = &castaways[i];
		if (abs(castaway->spr.params.pos.x - cgship->spr.params.pos.x) < 40.0f &&
			abs(castaway->spr.params.pos.y - cgship->spr.params.pos.y) < 30.0f)
		{
			bounceCastaway_Coastguardship(castaway);
		}
	}
}

/* Drawer Functions */
static void drawer_sea()
{
	C2D_DrawSprite(&sea.spr);
}

static void drawer_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
	{
		Castaway *castaway = &castaways[i];
		if ((castaway->alive == true && castaway->picked_up == false))
		{
			C2D_DrawSprite(&castaways[i].spr);
		}
	}
}

static void drawer_sharks()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
		C2D_DrawSprite(&sharks[i].spr);
}

static void drawer_lifeboat()
{
	if ((lboat->alive == true))
	{
		C2D_DrawSprite(&lifeboat.spr);
	}
}

static void drawer_coastguardship()
{
	C2D_DrawSprite(&coastguardship.spr);
}

static void sceneInit_bottom(void)
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

static void drawer_scoreboard(float size)
{
	// Clear the dynamic text buffer
	C2D_TextBufClear(g_dynamicBuf);

	// Draw static text strings
	C2D_DrawText(&g_staticText[0], C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, 150.0f, 25.0f, 0.5f, size, size, WHITE);

	// Generate and draw dynamic text
	char buf[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[BUFFER_SIZE], buf4[BUFFER_SIZE];
	C2D_Text dynText_lifes, dynText_points, dynText_levels, dynText_passengers;
	snprintf(buf, sizeof(buf), "Vidas: %d ", lboat->lifes);
	snprintf(buf2, sizeof(buf), "Puntos: %d ", 0);
	snprintf(buf3, sizeof(buf), "Nivel: %d ", 1);
	snprintf(buf4, sizeof(buf), "Pasajeros: %d / 3", lboat->seatcount);

	C2D_TextParse(&dynText_lifes, g_dynamicBuf, buf);
	C2D_TextParse(&dynText_points, g_dynamicBuf, buf2);
	C2D_TextParse(&dynText_levels, g_dynamicBuf, buf3);
	C2D_TextParse(&dynText_passengers, g_dynamicBuf, buf4);

	C2D_TextOptimize(&dynText_lifes);
	C2D_TextOptimize(&dynText_points);
	C2D_TextOptimize(&dynText_levels);
	C2D_TextOptimize(&dynText_passengers);

	C2D_DrawText(&dynText_levels, C2D_AtBaseline | C2D_WithColor, 16.0f, 150.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_points, C2D_AtBaseline | C2D_WithColor, 16.0f, 170.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_lifes, C2D_AtBaseline | C2D_WithColor, 16.0f, 190.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_passengers, C2D_AtBaseline | C2D_WithColor, 16.0f, 210.0f, 0.5f, size, size, WHITE);

	//TEST
	char testbuf2[BUFFER_SIZE];
	C2D_Text posx, posy, dx;
	snprintf(testbuf2, sizeof(testbuf2), "Shark Name: %d ", lboat->seatcount);
	C2D_TextParse(&posy, g_dynamicBuf, testbuf2);
	C2D_TextOptimize(&posx);
	C2D_TextOptimize(&posy);
	C2D_TextOptimize(&dx);
	C2D_DrawText(&posx, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 250.0f, 150.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&posy, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 250.0f, 170.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dx, C2D_AtBaseline | C2D_WithColor | C2D_AlignRight, 250.0f, 190.0f, 0.5f, size, size, WHITE);
}

static void scenesExit(void)
{
	// Delete the text buffers
	C2D_TextBufDelete(g_dynamicBuf);
	C2D_TextBufDelete(g_staticBuf);

	// Delete graphics
	C2D_SpriteSheetFree(castaways_spriteSheet);
	C2D_SpriteSheetFree(sharks_spriteSheet);
	C2D_SpriteSheetFree(coastguard_spriteSheet);
	C2D_SpriteSheetFree(sea_spriteSheet);
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
	init_sharks();
	init_lifeboat();
	init_coastguardship();

	// Initialize the scene for Scoreboard
	sceneInit_bottom();

	// Main loop
	while (aptMainLoop())
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
		moveSprites_sharks();
		moveLifeboat_sprite();
		moveSprite_coastguardship();

		// Collision Detectors
		// collisionShark_Shark();
		collisionShark_Castaway();
		collisionShark_Lifeboat();
		collisionCastaway_Lifeboat();
		collisionCastaway_Coastguardship();

		/* Start Render the scene */
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		/* TOP Screen */
		C2D_TargetClear(top, BLACK);
		C2D_SceneBegin(top);

		//Drawer Sprites
		drawer_sea();
		drawer_castaways();
		drawer_sharks();
		drawer_lifeboat();
		drawer_coastguardship();

		C2D_Flush(); // Ensures all 2D objects so far have been drawn.

		/* Bottom Screen */
		C2D_TargetClear(bottom, BLACK);
		C2D_SceneBegin(bottom);
		drawer_scoreboard(FONT_SIZE);

		C3D_FrameEnd(0); // Finish render the scene
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
