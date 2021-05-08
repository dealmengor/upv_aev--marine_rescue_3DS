// Main Header File

/* Includes */
#include <citro2d.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <3ds.h>

/* Defines */

// Screens dimensions
#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240
#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240

// Game Configuratation Variables
#define START_POINTS 0
#define START_SPEEDOMETER 0
#define START_LEVEL 1
#define NEXT_LEVEL 100
#define RESCUE_POINTS 10
#define TIME_DIFFERENCE_QUANTITY 2
#define TIME_BUFFER_SIZE 80

// Castaways Variables
#define MAX_CASTAWAYS 10
#define MAX_CASTAWAYS_SPEED 1
#define CASTAWAY_SPAWN 5

// LifeBoat Variables
#define BOAT_LIFES 3
#define BOAT_SPEED 2
#define BOAT_SEAT_COUNT 0
#define BOAT_FUEL_RECHARGE 15
#define BOAT_FUEL_CONSUMPTION 1
#define BOAT_TOP_SCREEN_WIDTH 380
#define BOAT_TOP_SCREEN_HEIGHT 220

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

/* Enumerated variables */
typedef enum
{
    EXIT_GAMESTATE,     // 0
    START_GAMESTATE,    // 1
    NEW_GAMESTATE,      // 2
    PAUSED_GAMESTATE,   // 3
    LEVEL_UP_GAMESTATE, // 4
    GAMEOVER_GAMESTATE, // 5
    WIN_GAMESTATE,      // 6
    MENU_GAMESTATE      // 7
} game_state;

typedef enum
{
    INITIAL_TIME_STATE, // 0
    TIME_CONTINUITY     // 1
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

// Sea sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity
} Sea;

// Sharpedo sprite struct
typedef struct
{
    int id;
    C2D_Sprite spr;
    float dx, dy; // velocity
    float speed;
    bool stalking;
} Sharpedo;

/** Function signatures **/

/* Initializer Functions */
void init_sprites();
void init_sea();
void init_castaways();
void init_sharpedo();
void init_lifeboat(int lifes, bool alive, int pos_x, int pos_y);
void init_coastguardship();

/* Sprites Controller */
void controllerSprites_lifeboat(int sprite_id);

/* Motion Functions */
void moveSprites_castaways();
void moveSprites_sharpedos();
void moveSprite_coastguardship();
void moveLifeboat_sprite();
void moveLifeboatController(u32 kHeld);

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
void drawer_sea();
void drawer_castaways();
void drawer_sharpedo();
void drawer_lifeboat();
void drawer_coastguardship();
void drawer_scoreboard(float size);

/* System Functions */
void sceneInit_bottom();
void scenesExit();

/* Game Controllers */
void gameStatusController(int sentinel);
void gameTimeController(int time_sentinel);
void gameInputController(int game_sentinel, u32 kDown, u32 kHeld);
void gameInitController();
void gameMoveSpritesController();
void gameCollisionsController();
void gameDrawersTopScreenController(int game_sentinel);
void gameDrawersBottomScreenController(int game_sentinel);