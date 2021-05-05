// Simple citro2d sprite drawing example
// Images borrowed from:
//   https://kenney.nl/assets/space-shooter-redux
#include <citro2d.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <3ds.h>

#define MAX_SPRITES 3
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SPEED 1

// Simple sprite struct
typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES;

//Buffer
C2D_TextBuf g_staticBuf, g_dynamicBuf;
C2D_Text g_staticText[5];

static void sceneInit(void)
{
	// Create two text buffers: one for static text, and another one for
	// dynamic text - the latter will be cleared at each frame.
	g_staticBuf = C2D_TextBufNew(4096); // support up to 4096 glyphs in the buffer
	g_dynamicBuf = C2D_TextBufNew(4096);

	// Parse the static text strings
	C2D_TextParse(&g_staticText[0], g_staticBuf, "Instructions");
	C2D_TextParse(&g_staticText[1], g_staticBuf, "Move up: ↑");
	C2D_TextParse(&g_staticText[2], g_staticBuf, "Move left: ←");
	C2D_TextParse(&g_staticText[3], g_staticBuf, "Move right: →");
	C2D_TextParse(&g_staticText[4], g_staticBuf, "Move down: ↓");
	C2D_TextParse(&g_staticText[5], g_staticBuf, "Use  to change size");

	// Optimize the static text strings
	C2D_TextOptimize(&g_staticText[0]);
	C2D_TextOptimize(&g_staticText[1]);
	C2D_TextOptimize(&g_staticText[2]);
	C2D_TextOptimize(&g_staticText[3]);
	C2D_TextOptimize(&g_staticText[4]);
	C2D_TextOptimize(&g_staticText[5]);
}

static void sceneRender(float size)
{
	// Clear the dynamic text buffer
	C2D_TextBufClear(g_dynamicBuf);

	// Draw static text strings
	C2D_DrawText(&g_staticText[0], C2D_WithColor | C2D_AlignCenter, 100.0f, 30.0f, 0.5f, size, size, C2D_Color32f(1.0f, 1.0f, 1.0f, 0.900f));
	C2D_DrawText(&g_staticText[1], C2D_WithColor | C2D_AlignJustified, 10.0f, 60.0f, 0.5f, size, size, C2D_Color32f(0.0f, 0.0f, 1.0f, 0.625f));
	C2D_DrawText(&g_staticText[2], C2D_WithColor | C2D_AlignJustified, 10.0f, 90.0f, 0.5f, size, size, C2D_Color32f(0.0f, 0.0f, 1.0f, 0.625f));
	C2D_DrawText(&g_staticText[3], C2D_WithColor | C2D_AlignJustified, 10.0f, 120.0f, 0.5f, size, size, C2D_Color32f(0.0f, 0.0f, 1.0f, 0.625f));
	C2D_DrawText(&g_staticText[4], C2D_WithColor | C2D_AlignJustified, 10.0f, 150.0f, 0.5f, size, size, C2D_Color32f(0.0f, 0.0f, 1.0f, 0.625f));
	C2D_DrawText(&g_staticText[5], C2D_WithColor | C2D_AlignRight, 300.0f, 100.0f, 0.5f, size, size, C2D_Color32f(1.0f, 0.0f, 1.0f, 0.625f));

	// Generate and draw dynamic text
	char buf[161];

	C2D_Text Sprites;
	snprintf(buf, sizeof(buf), "Sprites:     %zu/%u", numSprites, MAX_SPRITES);
	C2D_TextParse(&Sprites, g_dynamicBuf, buf);
	C2D_TextOptimize(&Sprites);
	C2D_DrawText(&Sprites, C2D_AlignCenter | C2D_WordWrap | C2D_WithColor, 200.0f, 170.0f, 0.5f, 0.5f, 0.5f, C2D_Color32f(1.0f, 0.0f, 0.0f, 1.0f));

	C2D_Text CPU;
	snprintf(buf, sizeof(buf), "CPU:     %6.2f%%", C3D_GetProcessingTime() * 6.0f);
	C2D_TextParse(&CPU, g_dynamicBuf, buf);
	C2D_TextOptimize(&CPU);
	C2D_DrawText(&CPU, C2D_AlignCenter, 200.0f, 205.0f, 0.5f, 0.5f, 0.5f);

	C2D_Text GPU;
	snprintf(buf, sizeof(buf), "GPU:     %6.2f%%", C3D_GetDrawingTime() * 6.0f);
	C2D_TextParse(&GPU, g_dynamicBuf, buf);
	C2D_TextOptimize(&GPU);
	C2D_DrawText(&GPU, C2D_AlignCenter, 200.0f, 215.0f, 0.5f, 0.5f, 0.5f);
}

static void sceneExit(void)
{
	// Delete the text buffers
	C2D_TextBufDelete(g_dynamicBuf);
	C2D_TextBufDelete(g_staticBuf);
}

static void initSprites()
{

	srand(time(NULL)); // Sets a seed for random numbers

	for (size_t i = 0; i < MAX_SPRITES; i++)
	{
		Sprite *sprite = &sprites[i];
		// Random image, position, rotation and SPEED
		C2D_SpriteFromSheet(&sprite->spr, spriteSheet, i);
		C2D_SpriteSetCenter(&sprite->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&sprite->spr, rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&sprite->spr, C3D_Angle(rand() / (float)RAND_MAX));
		sprite->dx = rand() * 4.0f / RAND_MAX - 2.0f;
		sprite->dy = rand() * 4.0f / RAND_MAX - 2.0f;
		if (i == 2)
		{
			C2D_SpriteSetRotationDegrees(&sprite->spr, 0);
		}
	}
}

static void moveSprites()
{

	for (size_t i = 0; i < numSprites - 1; i++)
	{
		Sprite *sprite = &sprites[i];
		C2D_SpriteMove(&sprite->spr, sprite->dx, sprite->dy);
		C2D_SpriteRotateDegrees(&sprite->spr, 1.0f);

		// Check for collision with the screen boundaries
		if ((sprite->spr.params.pos.x < sprite->spr.params.pos.w / 2.0f && sprite->dx < 0.0f) ||
			(sprite->spr.params.pos.x > (SCREEN_WIDTH - (sprite->spr.params.pos.w / 2.0f)) && sprite->dx > 0.0f))
			sprite->dx = -sprite->dx;

		if ((sprite->spr.params.pos.y < sprite->spr.params.pos.h / 2.0f && sprite->dy < 0.0f) ||
			(sprite->spr.params.pos.y > (SCREEN_HEIGHT - (sprite->spr.params.pos.h / 2.0f)) && sprite->dy > 0.0f))
			sprite->dy = -sprite->dy;
	}
}

static void movePlayerSprite()
{
	Sprite *sprite = &sprites[2];
	//LEFT BORDER
	if (sprite->spr.params.pos.x < SCREEN_WIDTH && sprite->spr.params.pos.x > 0)
	{
		C2D_SpriteMove(&sprite->spr, sprite->dx, 0);
	}
	//RIGHT BORDER
	else
	{
		if (sprite->spr.params.pos.x >= SCREEN_WIDTH)
			C2D_SpriteMove(&sprite->spr, -SPEED, 0);
		else
			C2D_SpriteMove(&sprite->spr, SPEED, 0);
	}

	if (sprite->spr.params.pos.y <= SCREEN_HEIGHT && sprite->spr.params.pos.y > 0)
	{
		C2D_SpriteMove(&sprite->spr, 0, sprite->dy);
	}
	else
	{
		if (sprite->spr.params.pos.y > SCREEN_HEIGHT)
		{
			C2D_SpriteMove(&sprite->spr, 0, -SPEED);
		}
		else
			C2D_SpriteMove(&sprite->spr, 0, SPEED);
	}
	sprite->dx = 0;
	sprite->dy = 0;
}

static void movePlayerController(u32 kHeld)
{
	Sprite *sprite = &sprites[2];
	//UP
	if (kHeld & KEY_UP)
	{
		sprite->dy += -SPEED;
	}
	// //DOWN
	if (kHeld & KEY_DOWN)
	{
		sprite->dy += SPEED;
	}
	//LEFT
	if (kHeld & KEY_LEFT)
	{
		sprite->dx += -SPEED;
	}
	//RIGHT
	if (kHeld & KEY_RIGHT)
	{
		sprite->dx += SPEED;
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

	// Create screens
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet)
		svcBreak(USERBREAK_PANIC);

	// Initialize sprites
	initSprites();

	// Initialize the scene
	sceneInit();

	float size = 0.5f;

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
			movePlayerController(kHeld);

		// Text Controller
		if (kHeld & KEY_L)
			size -= 1.0f / 128;
		else if (kHeld & KEY_R)
			size += 1.0f / 128;
		else if (kHeld & KEY_X)
			size = 0.5f;
		else if (kHeld & KEY_Y)
			size = 1.0f;

		if (size < 0.25f)
			size = 0.25f;
		else if (size > 2.0f)
			size = 2.0f;

		// Move sprites
		moveSprites();
		movePlayerSprite();

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		C2D_TargetClear(top, C2D_Color32(0x68, 0xB0, 0xD8, 0xFF));
		C2D_SceneBegin(top);
		sceneRender(size);

		C2D_TargetClear(bottom, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(bottom);

		for (size_t i = 0; i < numSprites; i++)
			C2D_DrawSprite(&sprites[i].spr);
		C3D_FrameEnd(0);
	}

	// Deinitialize the scene
	sceneExit();

	// Delete graphics
	C2D_SpriteSheetFree(spriteSheet);

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
