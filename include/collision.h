#include "asteroid.h"
#include <stdbool.h>
#include <math.h>
typedef struct vertex{
        float x;
        float y;
}vertex;
bool is_collision_detected(asteroid* as1,struct asteroid* as2);
void on_collision(asteroid *as1,struct asteroid *as2);
bool is_spaceship_hit(float as_x,float as_y,asteroid*sp,float theta_in_radians,float scale);
float calc_distance(float x1,float y1,float x2,float y2);
