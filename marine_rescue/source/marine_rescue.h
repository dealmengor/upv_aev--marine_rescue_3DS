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

//Screens dimensions
#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240
#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240

// Game Configuratation Variables
#define MAX_SHARKS 2
#define MAX_CASTAWAY 6
#define NEXT_LEVEL 100

// Boat Variables
#define BOAT_LIFES 3
#define BOAT_SPEED 3
#define BOAT_SEAT_COUNT 0

//Dynamic Text
#define BUFFER_SIZE 160
#define STATIC_TEXT_COUNT 1
#define FONT_SIZE 0.5f

//Colors
#define WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f)
#define CYAN C2D_Color32(0x68, 0xB0, 0xD8, 0xFF)

/* Structures */

// Castaway sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // System Reference
    bool alive;
    bool picked_up;

} Castaway;

// CoastGuard sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // System Reference
} CoastGuard;

// Lifeboat sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // System Reference
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
    float dx, dy; // System Reference
} Sea;

// Shark sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // System Reference
} Shark;