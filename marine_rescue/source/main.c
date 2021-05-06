#include "marine_rescue.h"

/* Globals */

/*Structures & Data Structures Declaratation*/
static Castaway castaways[MAX_CASTAWAY];
static Shark sharks[MAX_SHARKS];
static Lifeboat lifeboat;
static Sea sea;

/* Spritesheets Declaratation */
static C2D_SpriteSheet castaways_spriteSheet;
static C2D_SpriteSheet sharks_spriteSheet;
static C2D_SpriteSheet lifeboat_spriteSheet;
static C2D_SpriteSheet sea_spriteSheet;

/* Initializer Functions */
static void init_sprites()
{
	castaways_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/castaways.t3x");
	if (!castaways_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	sharks_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sharks.t3x");
	if (!sharks_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	lifeboat_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/lifeboat.t3x");
	if (!lifeboat_spriteSheet)
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
		Castaway *sprite = &castaways[i];
		// Random image, position, rotation and SPEED
		C2D_SpriteFromSheet(&sprite->spr, castaways_spriteSheet, rand() % 1);
		C2D_SpriteSetCenter(&sprite->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&sprite->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&sprite->spr, C3D_Angle(rand() / (float)RAND_MAX));
		sprite->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		sprite->dy = rand() * 4.0f / RAND_MAX - 2.0f;
		sprite->alive = true;
	}
}

static void init_sharks()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *sprite = &sharks[i];
		// Random image, position, rotation and SPEED
		C2D_SpriteFromSheet(&sprite->spr, sharks_spriteSheet, rand() % 1);
		C2D_SpriteSetCenter(&sprite->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&sprite->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&sprite->spr, C3D_Angle(rand() / (float)RAND_MAX));
		sprite->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		sprite->dy = rand() * 4.0f / RAND_MAX - 2.0f;
	}
}

static void init_lifeboat()
{
	Lifeboat *lboat = &lifeboat;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&lboat->spr, lifeboat_spriteSheet, 0);
	C2D_SpriteSetCenter(&lboat->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&lboat->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
	C2D_SpriteSetRotationDegrees(&lboat->spr, 0);
	lboat->dx = rand() * 4.0f / RAND_MAX - 2.0f;
	lboat->dy = rand() * 4.0f / RAND_MAX - 2.0f;
	lboat->speed = 1;
	lboat->alive = true;
}

/* Motion Functions */
static void moveSprites_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAY; i++)
	{
		Castaway *sprite = &castaways[i];
		if ((sprite->alive == true))
		{
			C2D_SpriteMove(&sprite->spr, sprite->dx, sprite->dy);
			C2D_SpriteRotateDegrees(&sprite->spr, 1.0f);
		}

		// Check for collision with the screen boundaries
		if ((sprite->spr.params.pos.x < sprite->spr.params.pos.w / 2.0f && sprite->dx < 0.0f) ||
			(sprite->spr.params.pos.x > (TOP_SCREEN_WIDTH - (sprite->spr.params.pos.w / 2.0f)) && sprite->dx > 0.0f))
			sprite->dx = -sprite->dx;

		if ((sprite->spr.params.pos.y < sprite->spr.params.pos.h / 2.0f && sprite->dy < 0.0f) ||
			(sprite->spr.params.pos.y > (TOP_SCREEN_HEIGHT - (sprite->spr.params.pos.h / 2.0f)) && sprite->dy > 0.0f))
			sprite->dy = -sprite->dy;
	}
}

static void moveSprites_sharks()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *sprite = &sharks[i];
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

static void moveLifeboat_sprite()
{
	Lifeboat *lboat = &lifeboat;
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
	Lifeboat *lboat = &lifeboat;
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

/* Life Controllers */
static void lifeboatDeath(Lifeboat *lboat)
{
	lboat->lifes = BOAT_LIFES - 1;
	lboat->fuel = 60;
	if ((lboat->lifes > 0))
	{
		init_lifeboat();
	}
	else
	{
		lboat->alive = false;
		//TODO: Call gameover function
	}
}

/* Collision Functions */
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
			// C2D_SpriteSetPos(&castaway->spr, TOP_SCREEN_WIDTH / 2, -500.0f);
		}
	}
}

static void collisionShark_lifeboat()
{
	for (size_t i = 0; i < MAX_SHARKS; i++)
	{
		Shark *shark = &sharks[i];
		Lifeboat *lboat = &lifeboat;

		if (abs(shark->spr.params.pos.x - lboat->spr.params.pos.x) < 20.0f &&
			abs(shark->spr.params.pos.y - lboat->spr.params.pos.y) < 20.0f)
		{
			lifeboatDeath(lboat);
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
		Castaway *sprite = &castaways[i];
		if ((sprite->alive == true))
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
	Lifeboat *lboat = &lifeboat;
	if ((lboat->alive == true))
	{
		C2D_DrawSprite(&lifeboat.spr);
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

	// Create screens
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	//C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load graphics Spritesheets
	init_sprites();

	// Initialize sprites for Structures
	init_sea();
	init_castaways();
	init_sharks();
	init_lifeboat();

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		// Interface Control Logic
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

		// Collision Detectors
		collisionShark_Castaway();
		collisionShark_lifeboat();

		// Start render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		//TOP Screen
		C2D_TargetClear(top, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(top);

		//Drawer Sprites
		drawer_sea();
		drawer_castaways();
		drawer_sharks();
		drawer_lifeboat();

		C3D_FrameEnd(0); // Finish render the scene
	}

	// Delete graphics
	C2D_SpriteSheetFree(castaways_spriteSheet);
	C2D_SpriteSheetFree(sharks_spriteSheet);
	C2D_SpriteSheetFree(lifeboat_spriteSheet);
	C2D_SpriteSheetFree(sea_spriteSheet);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
