// Main Header File

/* Includes */
#include <citro2d.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <3ds.h>
#include "stb/stretchy_buffer.h"

/* Defines */

// Screens dimensions
#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240
#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240

// Game Configuratation Variables
#define START_POINTS 0
#define START_LEVEL 1
#define NEXT_LEVEL 100
#define RESCUE_POINTS 10
#define TIME_DIFFERENCE_QUANTITY 3
#define TIME_BUFFER_SIZE 80
#define SCOREBOARD_LIMIT 5
#define MENU_OPTIONS_QUANTITY 5
#define MENU_COORDINATES_DIMENSION 2

// Castaways Variables
#define MAX_CASTAWAYS 10
#define MAX_CASTAWAYS_SPEED 1
#define CASTAWAY_SPAWN 5

// LifeBoat Variables
#define BOAT_LIFES 5
#define BOAT_SPEED 2
#define BOAT_SEAT_COUNT 0
#define BOAT_FUEL_RECHARGE 30
#define BOAT_FUEL_CONSUMPTION 1
#define BOAT_TOP_SCREEN_WIDTH 380
#define BOAT_TOP_SCREEN_HEIGHT 220
#define BOAT_START_POS_X 0
#define BOAT_START_POS_Y 0

// Sharpedos Variables
#define MAX_SHARPEDOS 10
#define MAX_SHARPEDOS_SPEED 1.5f
#define MAX_MEGA_SHARPEDOS_SPEED 2
#define MEGA_SHARPEDO_SPAWN_LEVEL 4

// Dynamic Text
#define BUFFER_SIZE 160
#define STATIC_TEXT_COUNT 1
#define FONT_SIZE 0.5f

// Colors
#define WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f)
#define CYAN C2D_Color32(0x68, 0xB0, 0xD8, 0xFF)

// Icons

// Boat Selector | Coordinates | X, Y
const int m_boat_selector_coordinates[MENU_OPTIONS_QUANTITY][MENU_COORDINATES_DIMENSION] = {
    {191, 109}, // Start Game
    {212, 132}, // Top 5
    {192, 156}, // Instructions
    {210, 180}, // Credits
    {220, 205}  // Exit
};

/* Enumerated variables */
typedef enum
{
    EXIT_GAMESTATE,         // 0
    START_GAMESTATE,        // 1
    PAUSED_GAMESTATE,       // 2
    LEVEL_UP_GAMESTATE,     // 3
    GAMEOVER_GAMESTATE,     // 4
    WIN_GAMESTATE,          // 5
    MENU_GAMESTATE,         // 6
    TOP_LIST_GAMESTATE,     // 7
    INSTRUCTIONS_GAMESTATE, // 8
    CREDITS_GAMESTATE       // 9
} game_state;

typedef enum
{
    INITIAL_TIME_STATE,  // 0
    INTIAL_PAUSED_TIME,  // 1
    TIME_CONTINUITY,     // 2
    STOP_TIME_CONTINUITY // 3
} time_state;

typedef enum
{
    NORTH_LIFEBOAT1,     // 0
    NORTHEAST_LIFEBOAT2, // 1
    EAST_LIFEBOAT3,      // 2
    SOUTHEAST_LIFEBOAT4, // 3
    SOUTH_LIFEBOAT5,     // 4
    SOUTHWEST_LIFEBOAT6, // 5
    WEST_LIFEBOAT7,      // 6
    NORTHWEST_LIFEBOAT8  // 7

} lifeboat_sprites;

/* Structures */

// Castaway sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity
    float speed;
    bool visible;
} Castaway;

// CoastGuard sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity
    float speed;
} CoastGuardShip;

// Lifeboat sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity
    float speed;
    int lifes;
    int fuel;
    bool alive;
    int seatcount;
} Lifeboat;

// Sharpedo sprite struct
typedef struct
{
    int id;
    C2D_Sprite spr;
    float dx, dy; // velocity
    float speed;
    bool stalking;
} Sharpedo;

// Screen sprites struct
typedef struct
{
    C2D_Sprite spr;
} Screen;

typedef struct
{
    char name[64];
    int score;
} player_score;

// Icon sprite struct
typedef struct
{
    C2D_Sprite spr;
} Icon;

/** Function signatures **/

/* Initializer Functions */
void init_sprites();

// Characters
void init_castaways();
void init_sharpedo();
void init_lifeboat(int lifes, bool alive, int pos_x, int pos_y);
void init_coastguardship();

// Icons
void init_boat_selector();

/* Screens */

// TOP Screens
void init_game_title_screen();
void init_sea_screen();
void init_game_over_screen();
void init_game_over_screen2();
void init_top_list_screen();
void init_instructions_screen();
void init_credits_screen();

// Bottom Screens
void init_scoreboard_screen();
void init_pause_screen();
void init_menu_screen();

/*Top List System */
void score_dialog();
void save_score();
void score_checker();

/* Sprite Controller */
void controllerSprites_lifeboat(int sprite_id);

/* Motion Functions */
void moveSprites_castaways();
void moveSprites_sharpedos();
void moveSprite_coastguardship();
void moveSprite_Lifeboat();
void moveLifeboatController(u32 kHeld);
void moveSprite_boat_selector(u32 kHeld);

/* Bounce Controllers */
void bounceCastaway_Coastguardship(Castaway *castaway);
void bounceSharpedo_Coastguardship(Sharpedo *sharpedo);
void bounceCoastGuardShip_Lifeboat();

/* Lifeboat Controllers */
void lifeboatpickUp(Lifeboat *lboat, Castaway *castaway);
void lifeboatDeath(Lifeboat *lboat);

/* Spawn Controllers */
void spawnNewCastaway();
void spawnNewSharpedo();

/* Collision Functions */
//void collisionSharpedo_Sharpedo();
void collisionsharpedo_Castaway();
void collisionsharpedo_Lifeboat();
void collisionCastaway_Lifeboat();
void collisionCoastGuardShip_Lifeboat();
void collisionCastaway_Coastguardship();
void collisionSharpedo_Coastguardship();

/* Drawer Functions */

// Characters
void drawer_castaways();
void drawer_sharpedo();
void drawer_lifeboat();
void drawer_coastguardship();

// Icons
void drawer_boat_selector();

/* Screens */

// TOP Screens
void drawer_game_title_screen();
void drawer_sea_screen();
void drawer_game_over_screen();
void drawer_game_over_screen2();
void drawer_top_list_screen();
void drawer_top_list(float size);
void drawer_instructions_screen();
void drawer_credits_screen();

// Bottom Screens
void drawer_scoreboard_screen();
void drawer_dynamic_score(float size);
void drawer_pause_screen();
void drawer_menu_screen();

/* System Functions */
void sceneInit_bottom();
void scenesExit();
void cleaner();

/* Game Controllers */
void gameStatusController(int game_sentinel, int time_sentinel);
void gameTimeController(int time_sentinel, int game_sentinel);
void gameInputController(int game_sentinel, u32 kDown, u32 kHeld);
void gameInitController();
void gameMoveSpritesController();
void gameCollisionsController();
void gameDrawersTopScreenController(int game_sentinel);
void gameDrawersBottomScreenController(int game_sentinel);

/* Predefined scores if there is no savegame */
char *predef_score_names[] = {"Raknia", "Ninbor", "NekoNoName", "Kirameiko", "TheGodz"};
int predef_score_scores[] = {150, 100, 80, 60, 40};