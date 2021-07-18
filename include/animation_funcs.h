#include "./include_allegro.h"
float translate_spaceship_x(ALLEGRO_TRANSFORM* trans,int speed,float screen_width,float omega,float curr_x);
float translate_spaceship_y(ALLEGRO_TRANSFORM* trans,int speed,float screen_height,float omega,float curr_y);
void translate_asteroid_x(ALLEGRO_TRANSFORM* trans,float screen_width,float* curr_x,float* speed_x);
void translate_asteroid_y(ALLEGRO_TRANSFORM*trans,float screen_height,float* curr_y,float* speed_y);
void rotate_object(ALLEGRO_TRANSFORM* trans,float curr_x,float curr_y,float omega);

