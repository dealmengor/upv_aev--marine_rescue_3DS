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
#define CASTAWAY_SPAWN 10

// LifeBoat Variables
#define BOAT_LIFES 3
#define BOAT_SPEED 2
#define BOAT_SEAT_COUNT 0
#define BOAT_FUEL_RECHARGE 25
#define BOAT_FUEL_CONSUMPTION 1
#define BOAT_TOP_SCREEN_WIDTH 392
#define BOAT_TOP_SCREEN_HEIGHT 220

// Sharpedos Variables
#define MAX_SHARPEDOS 10
#define MAX_SHARPEDOS_SPEED 1
#define MAX_MEGA_SHARPEDOS_SPEED 2
#define MEGA_SHARPEDO_SPAWN_LEVEL 5

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
    GAMEOVER_GAMESTATE, // 0
    START_GAMESTATE,    // 1
    LEVEL_UP_GAMESTATE, // 2
    NEW_GAMESTATE,      // 3
    WIN_GAMESTATE       // 4
} game_state;

typedef enum
{
    INITIAL_TIME_STATE, // 0
    TIME_CONTINUITY     // 1
} time_state;

/* Structures */

// Castaway sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity
    bool alive;
    bool picked_up;

} Castaway;

// CoastGuard sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity only on the horizontal x-axis
    float speed;
} CoastGuardShip;

// Lifeboat sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // velocity
    int speed;
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
    int speed;
    bool stalking;
} Sharpedo;

/** Function signatures **/

/* Initializer Functions */
void init_sprites();
void init_sea();
void init_castaways();
void init_sharpedo();
void init_lifeboat(int lifes);
void init_coastguardship();

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
void gameStatusController(int sentinel);