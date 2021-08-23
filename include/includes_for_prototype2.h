#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include "../include/animation_funcs.h"
#include "../include/menu_functions.h"
#include "./include_allegro.h"
#include "./collision.h"
/* defines */
#define OMEGA   0.10
#define SC_WIDTH        1200
#define SC_HEIGHT       650
#define ACC     0.1
#define DECC    0.5
#define MAX_SPEED       10
#define SCALE   25
#define SPEED_X 8
#define SPEED_Y 4

/* flags */
#define GAME_STARTED     0b00000001 //this indicated that a game has been started by the player
#define GAME_PAUSED      0b00000010 //this indicated that the player is in menu
#define PROT_SP          0b00000100 //this is used to protect the sp from asteroids
#define NEW_GAME_STARTED 0b00001000 //this indicates that a fresh game has been started by the player
#define SP_RESPAWN	 0b00010000 //this indicates that the spaceship has respawned after impact from an asteroid
#define RESPAWNING	 0b00100000 //this indicates that the player is in respawning screen
#define SP_DEAD		 0b01000000 //this indicates that all the lives are used by the spaceship and its dead
typedef struct blast blast;
typedef struct blast
{
        bool alive;
        float x;
        float y;
        float speed;
        float direction;
        ALLEGRO_TRANSFORM* trans;
        blast* next;
}blast;

typedef struct game_t
{
        int state;
        int blasts_on_screen;
        int asteroids_on_screen;
	int asteroids_alive;
        int score;
	int lives_left;
        ALLEGRO_TIMER* timer;
        ALLEGRO_EVENT_QUEUE* queue;
        ALLEGRO_EVENT event;
        ALLEGRO_DISPLAY* disp;
        asteroid* as_null;
        blast* bl_null;
}game_t;

void draw_each_blast();
void generate_blast(float sp_pos_x,float sp_pos_y,float direction);
void handle_sp_collision(float *sp_x,float *sp_y,float *theta);
void handle_collision();
asteroid* generate_asteroids();
void draw_each_asteroid();
void clean_blasts(bool );
void clean_asteroids(bool );
void* generate_new_asteroids(void* null);
void generate_broken_asteroids(asteroid* broken,float x_multiplier,float y_multiplier);
void break_asteroid(asteroid *as,blast *bl);
void respawn_wait();
void you_died();
