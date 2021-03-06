#include "marine_rescue.h"

/* Globals */

/* Socoreboard Variables */
int game_status = MENU_GAMESTATE;
int points = START_POINTS;
int level = START_LEVEL;

/* Time Variables */
time_t current_epoch_time, initial_second, current_initial_paused_time, last_paused_time, game_time, next_spawn, next_fuel_consumption;
int diff_t[TIME_DIFFERENCE_QUANTITY]; //Save time differences
struct tm ts;						  //Time Structure
char time_buf[TIME_BUFFER_SIZE];	  //Buffer Convert from epoch to human-readable date

/* Element Counters */
int castaway_count_active = 0;
int sharpedo_count_active = 0;
int castawaysaved = 0;

/* Structures & Data Structures Declaratation */
static Castaway castaways[MAX_CASTAWAYS];
static Sharpedo sharpedos[MAX_SHARPEDOS];
static Lifeboat lifeboat;
static CoastGuardShip coastguardship;

Lifeboat *lboat = &lifeboat;
CoastGuardShip *cgship = &coastguardship;

// Icons
static Icon boat_selector;
Icon *b_selector = &boat_selector;
size_t b_selector_coordinates_matrix_index;

// TOP Screens
static Screen game_title, sea, game_over, game_over2, win, top_list, instructions, credits;

// Bottom Screens
static Screen menu, scoreboard, pause;

/*Top List System */
int score_data;
bool checker = false;

/* Spritesheets Declaratation */
static C2D_SpriteSheet castaways_spriteSheet;
static C2D_SpriteSheet coastguard_spriteSheet;
static C2D_SpriteSheet sharpedo_spriteSheet;
static C2D_SpriteSheet screens_spriteSheet;
static C2D_SpriteSheet screens2_spriteSheet;

/* C2D_Text Declaration Variables */
C2D_TextBuf g_dynamicBuf; // Buffer Declaratation

/* Audio Variables */
int sound_status = STAND_BY_STATE;

// Wave buffer struct and type
ndspWaveBuf waveBuf[1];

/* Initializer Functions */
void init_sprites()
{
	// Characters
	castaways_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/castaways.t3x");
	if (!castaways_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	coastguard_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/coastguard.t3x");
	if (!coastguard_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	sharpedo_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sharpedos.t3x");
	if (!sharpedo_spriteSheet)
		svcBreak(USERBREAK_PANIC);

	// Screens
	screens_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/screens.t3x");
	if (!screens_spriteSheet)
		svcBreak(USERBREAK_PANIC);
	screens2_spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/screens2.t3x");
	if (!screens2_spriteSheet)
		svcBreak(USERBREAK_PANIC);
}

// Characters
void init_castaways()
{
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		// SPEED, Random image, position & rotation
		C2D_SpriteFromSheet(&castaway->spr, castaways_spriteSheet, rand() % 2);
		C2D_SpriteSetCenter(&castaway->spr, 0.5f, 0.5f);
		C2D_SpriteSetPos(&castaway->spr, rand() % TOP_SCREEN_WIDTH, rand() % TOP_SCREEN_HEIGHT);
		C2D_SpriteSetRotation(&castaway->spr, C3D_Angle(rand() / (float)RAND_MAX));
		castaway->speed = MAX_CASTAWAYS_SPEED;
		castaway->dx = castaway->speed;
		castaway->dy = castaway->speed;
		castaway->visible = false;
	}
}

void init_sharpedo()
{
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sprite = &sharpedos[i];
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
		if (i == 0)
		{
			sprite->stalking = false; // One Sharpedo for first level
			sharpedo_count_active += 1;
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

// Icons
void init_boat_selector()
{
	int pos_x, pos_y;
	pos_x = m_boat_selector_coordinates[b_selector_coordinates_matrix_index][0];
	pos_y = m_boat_selector_coordinates[b_selector_coordinates_matrix_index][1];

	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&b_selector->spr, screens_spriteSheet, 8);
	C2D_SpriteSetCenter(&b_selector->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&b_selector->spr, pos_x, pos_y);
}

/* Screens */

// TOP Screens
void init_game_title_screen()
{
	Screen *sprite = &game_title;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 0);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_sea_screen()
{
	Screen *sprite = &sea;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 5);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_game_over_screen()
{
	Screen *sprite = &game_over;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens2_spriteSheet, 0);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_game_over_screen2()
{
	Screen *sprite = &game_over2;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens2_spriteSheet, 1);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_win_game_screen()
{
	Screen *sprite = &win;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens2_spriteSheet, 2);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_top_list_screen()
{
	Screen *sprite = &top_list;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 1);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_instructions_screen()
{
	Screen *sprite = &instructions;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 2);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

void init_credits_screen()
{
	Screen *sprite = &credits;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 3);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, TOP_SCREEN_WIDTH / 2, TOP_SCREEN_HEIGHT);
}

// Bottom Screens
void init_scoreboard_screen()
{
	Screen *sprite = &scoreboard;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 6);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, BOTTOM_SCREEN_WIDTH / 2, BOTTOM_SCREEN_HEIGHT);
}

void init_pause_screen()
{
	Screen *sprite = &pause;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 7);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, BOTTOM_SCREEN_WIDTH / 2, BOTTOM_SCREEN_HEIGHT);
}

void init_menu_screen()
{
	Screen *sprite = &menu;
	// Position, rotation and SPEED
	C2D_SpriteFromSheet(&sprite->spr, screens_spriteSheet, 4);
	C2D_SpriteSetCenter(&sprite->spr, 0.5f, 1.0f);
	C2D_SpriteSetPos(&sprite->spr, BOTTOM_SCREEN_WIDTH / 2, BOTTOM_SCREEN_HEIGHT);
}

/*Top List System */
void score_dialog()
{
	static char hint_buf[64], input_buf[64];
	SwkbdState swkbd;
	SwkbdButton button_pressed = SWKBD_BUTTON_NONE;
	sprintf(hint_buf, "Enter your nickname to save your score");
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK | SWKBD_FILTER_AT, 0, 0);
	swkbdSetHintText(&swkbd, hint_buf);

	/* The system OS stops us here until the user is done inputting text */
	button_pressed = swkbdInputText(&swkbd, input_buf, sizeof(input_buf));
	/* We resume execution here */
	if (button_pressed == SWKBD_BUTTON_CONFIRM)
	{
		save_score(input_buf, score_data);
	}
	else
	{
		score_data = 0;
	}
}

void save_score(char *name, int score_data)
{

	FILE *fr = fopen("Marine_Rescue_scoreboard.txt", "r");
	player_score *records = NULL;

	if (fr)
	{
		/* Read all records */
		do
		{
			player_score new_record;
			//Validate if new nickname and record is similar to the end of the file
			if (fscanf(fr, "%s\n", new_record.name) == EOF)
				break;
			if (fscanf(fr, "@%d\n", &new_record.score) == EOF)
				break;
			sb_push(records, new_record);
		} while (1);
		fclose(fr);
	}

	/* copying the last player score to the Scoreboard list */
	player_score last_player_score;
	strcpy(last_player_score.name, name);
	last_player_score.score = score_data;
	sb_push(records, last_player_score);

	/* Reorder scores */
	for (int i = 0; i < sb_count(records); ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			if (records[j].score < records[i].score)
			{
				player_score tmp = records[j];
				records[j] = records[i];
				records[i] = tmp;
			}
		}
	}

	/* Rewrite ordered scores */
	FILE *fw = fopen("Marine_Rescue_scoreboard.txt", "w");
	for (int i = 0; i < sb_count(records); ++i)
	{
		fprintf(fw, "%s\n", records[i].name);
		fprintf(fw, "@%d\n", records[i].score);
	}
	fclose(fw);
	sb_free(records);
}

void score_checker()
{
	FILE *fp = fopen("Marine_Rescue_scoreboard.txt", "r");
	int i;

	for (i = 0; i < 5; i++)
	{
		player_score record;
		fscanf(fp, "%s\n", record.name);
		fscanf(fp, "@%d\n", &record.score);

		// Read the list and compare the player's points against the top 5 members
		if (score_data > record.score)
		{
			checker = true;
			break;
		}
		//Read only the 5 players of the file
		if (score_data < record.score && i == 4)
		{
			checker = false;
		}
	}
	fclose(fp);
}

/* Sprite Controller */
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

void moveSprite_boat_selector(u32 kHeld)
{
	// Adjusts the marker of the matrix coordinate index
	if ((b_selector_coordinates_matrix_index < MENU_OPTIONS_QUANTITY - 1) && (kHeld & KEY_DOWN))
	{
		b_selector_coordinates_matrix_index += 1;
	}
	else if ((b_selector_coordinates_matrix_index > 0) && (kHeld & KEY_UP))
	{
		b_selector_coordinates_matrix_index -= 1;
	}
	else
	{
		b_selector_coordinates_matrix_index = 0;
	}

	// Get the inverse translation
	int new_pos_x, new_pos_y;
	new_pos_x = ((b_selector->spr.params.pos.x - m_boat_selector_coordinates[b_selector_coordinates_matrix_index][0]) * -1);
	new_pos_y = ((b_selector->spr.params.pos.y - m_boat_selector_coordinates[b_selector_coordinates_matrix_index][1]) * -1);

	C2D_SpriteMove(&b_selector->spr, new_pos_x, new_pos_y);
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

void bounceCoastGuardShip_Lifeboat(int compass_sentinel)
{
	// UPPER SHIP
	if (compass_sentinel == NORTH)
		C2D_SpriteMove(&lboat->spr, 0, -lboat->speed);

	// LEFT SHIP
	if (compass_sentinel == WEST)
		C2D_SpriteMove(&lboat->spr, -lboat->speed, 0);

	// RIGTH SHIP
	if (compass_sentinel == EAST)
		C2D_SpriteMove(&lboat->spr, lboat->speed, 0);
}

/* Lifeboat Actions */
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
			gameStatusController(GAMEOVER_GAMESTATE, STOP_TIME_CONTINUITY);
		}
	}
}

void lifeboatBoarding()
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

			//Check for win
			if (points == WIN_POINTS)
			{
				gameStatusController(WIN_GAMESTATE, STOP_TIME_CONTINUITY); //WIN GAME!
			}
			else if (points % NEXT_LEVEL == 0)
			{
				gameStatusController(LEVEL_UP_GAMESTATE, TIME_CONTINUITY); //LEVEL UP!
			}
		}
		lboat->seatcount = 0;
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
			castaway_count_active += 1;
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
				sharpedo_count_active += 1;
				break;
			}
		}
		gameStatusController(START_GAMESTATE, TIME_CONTINUITY);
	}
}

/* Collision Functions */
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

		// UPPER
		if (abs(cgship->spr.params.pos.x - lboat->spr.params.pos.x) <= 50.0f &&
			abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) <= 30.0f)
		{
			// Lifeboat Boarding
			lifeboatBoarding();
			// Bounce
			bounceCoastGuardShip_Lifeboat(NORTH);
		}

		// Left
		else if (abs(cgship->spr.params.pos.x - lboat->spr.params.pos.x) == 60.0f &&
				 abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) < 13.0f)
		{
			// Lifeboat Boarding
			lifeboatBoarding();
			// Bounce
			bounceCoastGuardShip_Lifeboat(WEST);
		}

		// Right
		else if (cgship->spr.params.pos.x - lboat->spr.params.pos.x == -60.0f &&
				 abs(cgship->spr.params.pos.y - lboat->spr.params.pos.y) < 13.0f)
		{
			// Lifeboat Boarding
			lifeboatBoarding();
			// Bounce
			bounceCoastGuardShip_Lifeboat(EAST);
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

// Characters
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
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
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

// Icons
void drawer_boat_selector()
{
	C2D_DrawSprite(&b_selector->spr);
}

/* Screens */

// TOP Screens
void drawer_game_title_screen()
{
	C2D_DrawSprite(&game_title.spr);
}

void drawer_sea_screen()
{
	C2D_DrawSprite(&sea.spr);
}

void drawer_game_over_screen()
{
	C2D_DrawSprite(&game_over.spr);
}

void drawer_game_over_screen2()
{
	C2D_DrawSprite(&game_over2.spr);
}

void drawer_win_game_screen()
{
	C2D_DrawSprite(&win.spr);
}

void drawer_top_list_screen()
{
	C2D_DrawSprite(&top_list.spr);
}

void drawer_top_list(float size)
{
	// Clear the dynamic text buffer
	C2D_TextBufClear(g_dynamicBuf);

	float initial_x = 60.0f;
	float initial_y = 80.0f;
	C2D_Text s_text;
	FILE *fr = fopen("Marine_Rescue_scoreboard.txt", "r");
	player_score *records = NULL;

	if (fr)
	{
		/* Read all records */
		do
		{
			player_score new_record;
			if (fscanf(fr, "%s\n", new_record.name) == EOF)
				break;
			if (fscanf(fr, "@%d\n", &new_record.score) == EOF)
				break;
			sb_push(records, new_record);
		} while (1);
		fclose(fr);
	}
	if (sb_count(records) < SCOREBOARD_LIMIT)
	{
		int j = 0;
		while (sb_count(records) < SCOREBOARD_LIMIT)
		{
			player_score predef_score;
			strcpy(predef_score.name, predef_score_names[j]);
			predef_score.score = predef_score_scores[j];
			sb_push(records, predef_score);
			++j;
		}
		/* Reorder scores */
		for (int i = 0; i < sb_count(records); ++i)
		{
			for (int j = 0; j < i; ++j)
			{
				if (records[j].score < records[i].score)
				{
					player_score tmp = records[j];
					records[j] = records[i];
					records[i] = tmp;
				}
			}
		}
		/* Write predefined scores to file */
		/* Rewrite ordered scores */
		FILE *fw = fopen("Marine_Rescue_scoreboard.txt", "w");
		for (int i = 0; i < sb_count(records); ++i)
		{
			fprintf(fw, "%s\n", records[i].name);
			fprintf(fw, "@%d\n", records[i].score);
		}
		fclose(fw);
	}
	for (int i = 0; i < 5 && i < sb_count(records); i++)
	{
		char buf[BUFFER_SIZE];
		snprintf(buf, sizeof(buf), "%s ??????- %d", records[i].name, records[i].score);
		C2D_TextParse(&s_text, g_dynamicBuf, buf);
		C2D_TextOptimize(&s_text);
		C2D_DrawText(&s_text, C2D_AtBaseline | C2D_WithColor, initial_x, initial_y, 0.0f, 0.7f, 0.7f, WHITE);
		initial_y += 20.0f;
	}
	sb_free(records);
}

void drawer_instructions_screen()
{
	C2D_DrawSprite(&instructions.spr);
}

void drawer_credits_screen()
{
	C2D_DrawSprite(&credits.spr);
}

// Bottom Screens
void drawer_scoreboard_screen()
{
	C2D_DrawSprite(&scoreboard.spr);
}

void drawer_dynamic_score(float size)
{
	// Clear the dynamic text buffer
	C2D_TextBufClear(g_dynamicBuf);

	// Generate and draw dynamic text
	char buf[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[BUFFER_SIZE], buf4[BUFFER_SIZE], buf5[BUFFER_SIZE], buf6[BUFFER_SIZE];
	C2D_Text dynText_lifes, dynText_points, dynText_levels, dynText_passengers, dynText_fuel, dynText_time;

	snprintf(buf, sizeof(buf), " %d", lboat->lifes);
	snprintf(buf2, sizeof(buf2), " %d", points);
	snprintf(buf3, sizeof(buf3), " %d", level);
	snprintf(buf5, sizeof(buf5), " %d", lboat->fuel);
	snprintf(buf4, sizeof(buf4), "%d/3", lboat->seatcount);
	snprintf(buf6, sizeof(buf6), " %s", time_buf);

	C2D_TextParse(&dynText_lifes, g_dynamicBuf, buf);
	C2D_TextParse(&dynText_points, g_dynamicBuf, buf2);
	C2D_TextParse(&dynText_levels, g_dynamicBuf, buf3);
	C2D_TextParse(&dynText_fuel, g_dynamicBuf, buf5);
	C2D_TextParse(&dynText_passengers, g_dynamicBuf, buf4);
	C2D_TextParse(&dynText_time, g_dynamicBuf, buf6);

	C2D_TextOptimize(&dynText_lifes);
	C2D_TextOptimize(&dynText_points);
	C2D_TextOptimize(&dynText_levels);
	C2D_TextOptimize(&dynText_fuel);
	C2D_TextOptimize(&dynText_passengers);
	C2D_TextOptimize(&dynText_time);

	C2D_DrawText(&dynText_levels, C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, 90.0f, 116.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_points, C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, 91.0f, 136.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_lifes, C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, 268.0f, 115.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_fuel, C2D_AtBaseline | C2D_WithColor | C2D_AlignCenter, 268.0f, 130.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_passengers, C2D_AtBaseline | C2D_WithColor, 261.0f, 145.0f, 0.5f, size, size, WHITE);
	C2D_DrawText(&dynText_time, C2D_AtBaseline | C2D_WithColor, 129.0f, 215.0f, 0.5f, size, size, WHITE);
}

void drawer_pause_screen()
{
	C2D_DrawSprite(&pause.spr);
}

void drawer_menu_screen()
{
	C2D_DrawSprite(&menu.spr);
}

/* System Functions */
void sceneInit()
{
	// Create one buffers: one for dynamic text, it'll be cleared at each frame.
	g_dynamicBuf = C2D_TextBufNew(4096);
}

void scenesExit()
{
	// Delete the text buffers
	C2D_TextBufDelete(g_dynamicBuf);

	// Delete graphics
	C2D_SpriteSheetFree(castaways_spriteSheet);
	C2D_SpriteSheetFree(sharpedo_spriteSheet);
	C2D_SpriteSheetFree(coastguard_spriteSheet);
	C2D_SpriteSheetFree(screens_spriteSheet);
	C2D_SpriteSheetFree(screens2_spriteSheet);
}

void cleaner()
{
	/* Reset Socoreboard Variables */
	points = START_POINTS;
	level = START_LEVEL;

	/* Element Counters */
	castawaysaved = 0;

	/*Top List System */
	score_data = 0;
	checker = false;

	// Time Variables cleaning
	current_epoch_time = 0;
	initial_second = 0;
	current_initial_paused_time = 0;
	last_paused_time = 0;
	game_time = 0;
	next_spawn = 0;
	next_fuel_consumption = 0;

	// Arrays & buffers cleaning
	memset(diff_t, 0, sizeof(diff_t));
	C2D_TextBufClear(g_dynamicBuf);

	// Lifeboat_selector cleaning
	void init_boat_selector();

	// Lifeboat cleaning
	init_lifeboat(BOAT_LIFES, false, BOAT_START_POS_X, BOAT_START_POS_Y);

	// CoastGuard Ship cleaning
	init_coastguardship();

	// Sharpedos cleaning
	for (size_t i = 0; i < MAX_SHARPEDOS; i++)
	{
		Sharpedo *sprite = &sharpedos[i];
		if (sprite->stalking == false && sharpedo_count_active > 1)
		{
			sprite->stalking = true; // One Sharpedo for first level
			sharpedo_count_active -= 1;
		}
		else
		{
			break;
		}
	}

	// Castaways cleaning
	for (size_t i = 0; i < MAX_CASTAWAYS; i++)
	{
		Castaway *castaway = &castaways[i];
		if (castaway->visible == true)
		{
			castaway->visible = false;
			castaway_count_active -= 1;
			break;
		}
	}
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
		lboat->seatcount = BOAT_SEAT_COUNT;
		score_data = points;
		score_checker(); // Compare current score against the Top List
		break;

	case WIN_GAMESTATE:
		game_status = WIN_GAMESTATE;
		score_data = points;
		score_checker(); // Compare current score against the Top List
		break;

	case MENU_GAMESTATE:
		game_status = MENU_GAMESTATE;
		cleaner();
		break;

	case TOP_LIST_GAMESTATE:
		game_status = TOP_LIST_GAMESTATE;
		break;

	case INSTRUCTIONS_GAMESTATE:
		game_status = INSTRUCTIONS_GAMESTATE;
		break;

	case CREDITS_GAMESTATE:
		game_status = CREDITS_GAMESTATE;
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
	// General GAMESTATE Controls
	if ((kDown & KEY_L) && (kDown & KEY_R))
		gameStatusController(EXIT_GAMESTATE, STOP_TIME_CONTINUITY); // Break in order to return to hbmenu

	// Start GAMESTATE & Level Up GAMESTATE Controls
	if (game_sentinel == START_GAMESTATE || game_sentinel == LEVEL_UP_GAMESTATE)
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

	/* Game Over & Win GAMESTATE Controls */
	// Save the player's score
	if (game_sentinel == GAMEOVER_GAMESTATE || game_sentinel == WIN_GAMESTATE)
	{
		if (checker == true)
		{
			if (kDown & KEY_A)
			{
				score_dialog();
			}
		}
		if (kDown & KEY_B)
		{
			gameStatusController(MENU_GAMESTATE, INITIAL_TIME_STATE);
		}
	}

	// Menu GAMESTATE Controls
	if (game_sentinel == MENU_GAMESTATE || game_sentinel == TOP_LIST_GAMESTATE || game_sentinel == INSTRUCTIONS_GAMESTATE || game_sentinel == CREDITS_GAMESTATE)
	{
		if ((kDown & KEY_START))
			gameStatusController(START_GAMESTATE, INITIAL_TIME_STATE);

		if (kDown & KEY_UP)
			moveSprite_boat_selector(kHeld);

		if (kDown & KEY_DOWN)
			moveSprite_boat_selector(kHeld);

		if (kDown & KEY_A)
		{
			switch (b_selector_coordinates_matrix_index)
			{
			case 0:
				gameStatusController(START_GAMESTATE, INITIAL_TIME_STATE);
				break;
			case 1:
				gameStatusController(TOP_LIST_GAMESTATE, STOP_TIME_CONTINUITY);
				break;
			case 2:
				gameStatusController(INSTRUCTIONS_GAMESTATE, STOP_TIME_CONTINUITY);
				break;
			case 3:
				gameStatusController(CREDITS_GAMESTATE, STOP_TIME_CONTINUITY);
				break;
			case 4:
				gameStatusController(EXIT_GAMESTATE, STOP_TIME_CONTINUITY);
				break;
			}
		}
	}
}

void gameInitController()
{
	// Characters
	init_castaways();
	init_sharpedo();
	init_coastguardship();
	init_lifeboat(BOAT_LIFES, false, BOAT_START_POS_X, BOAT_START_POS_Y);

	// Icons
	init_boat_selector();

	/* Screens */

	// TOP Screens
	init_game_title_screen();
	init_sea_screen();
	init_game_over_screen();
	init_game_over_screen2();
	init_win_game_screen();
	init_top_list_screen();
	init_instructions_screen();
	init_credits_screen();

	// Bottom Screens
	init_menu_screen();
	init_scoreboard_screen();
	init_pause_screen();
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
		// Important to draw screens first and then the characters
		drawer_sea_screen();
		drawer_castaways();
		drawer_sharpedos();
		drawer_lifeboat();
		drawer_coastguardship();
	}
	else if (game_sentinel == MENU_GAMESTATE)
	{
		drawer_game_title_screen();
	}
	else if (game_sentinel == TOP_LIST_GAMESTATE)
	{
		drawer_top_list_screen();
		drawer_top_list(FONT_SIZE);
	}
	else if (game_sentinel == INSTRUCTIONS_GAMESTATE)
	{
		drawer_instructions_screen();
	}
	else if (game_sentinel == CREDITS_GAMESTATE)
	{
		drawer_credits_screen();
	}
	else if (game_sentinel == GAMEOVER_GAMESTATE)
	{
		if (checker == true)
		{
			drawer_game_over_screen();
		}
		else if (checker == false)
		{
			drawer_game_over_screen2();
		}
	}
	else if (game_sentinel == WIN_GAMESTATE)
	{
		drawer_win_game_screen();
	}
}

void gameDrawersBottomScreenController(int game_sentinel)
{
	if (game_sentinel == START_GAMESTATE || game_sentinel == WIN_GAMESTATE || game_sentinel == LEVEL_UP_GAMESTATE || game_sentinel == GAMEOVER_GAMESTATE)
	{
		drawer_scoreboard_screen();
		drawer_dynamic_score(FONT_SIZE);
	}
	else if (game_sentinel == PAUSED_GAMESTATE)
	{
		drawer_pause_screen();
	}
	else if (game_sentinel == MENU_GAMESTATE || game_sentinel == TOP_LIST_GAMESTATE || game_sentinel == INSTRUCTIONS_GAMESTATE || game_sentinel == CREDITS_GAMESTATE)
	{
		drawer_menu_screen();
		drawer_boat_selector();
	}
}

void gameSoundController(int sound_sentinel)
{

	if (sound_sentinel == PLAY_STATE)
	{
		// Pointer for Linear Allocation
		u32 *audioBuffer = (u32 *)linearAlloc(SAMPLESPERBUF * BYTESPERSAMPLE * 2);

		// Initializes NDSP.
		ndspInit();

		// Output mode to set.
		ndspSetOutputMode(NDSP_OUTPUT_MONO);

		// Sets the interpolation type of a channel.
		ndspChnSetInterp(0, NDSP_INTERP_LINEAR);

		// Sets the sample rate of a channel. ID of the channel (0..23).
		ndspChnSetRate(0, SAMPLERATE);

		/*
		Sets the format of a channel. ID of the channel (0..23). 
		PCM8. Pulse-code modulation(PCM), bit depth is the number of bits of information in each sample 
		and it directly corresponds to the resolution of each sample. 8 bits per sample
		*/
		ndspChnSetFormat(0, NDSP_FORMAT_PCM8);

		/*
		Sets the mix parameters (volumes) of a channel.
		0: Front left volume.
		1: Front right volume.
		2: Back left volume:
		3: Back right volume:
		4..7: Same as 0..3, but for auxiliary output 0.
		8..11: Same as 0..3, but for auxiliary output 1.
		*/

		// Mix Channels
		float mix[12];
		memset(mix, 0, sizeof(mix)); // Fill with "0" all mix elements and then use the 0 and 1 channel
		mix[0] = 1.0;
		mix[1] = 1.0;
		ndspChnSetMix(0, mix);

		// Fill with "0" all waveBuf elements and then then use the 0 and 1 channel
		memset(waveBuf, 0, sizeof(waveBuf));
		waveBuf[0].data_vaddr = memcpy(&audioBuffer[0], BUFFER_TO_STREAM, sizeof(BUFFER_TO_STREAM)); // Data virtual address
		waveBuf[0].nsamples = SAMPLESPERBUF;														 // Total number of samples
		waveBuf[0].looping = true;																	 //Looping Music True
		ndspChnWaveBufAdd(0, &waveBuf[0]);
	}
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

	// Initialize the scene
	sceneInit();

	// Initialize audio Buffer
	gameSoundController(PLAY_STATE);

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
