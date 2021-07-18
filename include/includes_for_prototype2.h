#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include "../include/animation_funcs.h"
#include "../include/menu_functions.h"
#include "./include_allegro.h"
#include "./collision.h"
typedef struct blast blast;
typedef struct blast{
        float x;
        float y;
        float speed;
        bool alive;
        float direction;
        ALLEGRO_TRANSFORM* trans;
        blast* next;
}blast;
typedef struct game_t
{
        int state;
        int blasts_on_screen;
        int asteroids_on_screen;
        int score;
        ALLEGRO_TIMER* timer;
        ALLEGRO_EVENT_QUEUE* queue;
        ALLEGRO_EVENT event;
        ALLEGRO_DISPLAY* disp;
        asteroid* as_null;
        blast* bl_null;
}game_t;

void draw_each_blast();
void generate_blast(float sp_pos_x,float sp_pos_y,float direction);
void handle_sp_collision(float sp_x,float sp_y,float theta);
void handle_collision();
asteroid* generate_asteroids();
void draw_each_asteroid();
void clean_blasts();
void generate_broken_asteroids(asteroid* broken,float x_multiplier,float y_multiplier);
void break_asteroid(asteroid *as,blast *bl);
