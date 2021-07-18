#include "include_allegro.h"
typedef struct asteroid asteroid;
typedef struct asteroid{
		 bool alive;
		 float scale;
                 float x;
                 float y;
                 float speed_x;
                 float speed_y;
                 ALLEGRO_TRANSFORM* trans;
                 asteroid* next;
                 float omega;
                 float current_rotation;
                 ALLEGRO_COLOR color;
         }asteroid;

