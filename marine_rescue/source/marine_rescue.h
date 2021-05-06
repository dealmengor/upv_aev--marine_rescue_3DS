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

// Game Variables
#define MAX_SHARKS 2
#define MAX_CASTAWAY 2
#define BOAT_LIFES 3

/* Structures */

// Castaway sprite struct
typedef struct
{
    C2D_Sprite spr;
    float dx, dy; // System Reference
    bool alive;

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