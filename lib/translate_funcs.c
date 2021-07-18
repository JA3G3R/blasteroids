#include "../include/include_allegro.h"
#include "../include/animation_funcs.h"
#include <math.h>
// use to move an object on the screen along the direction of its speed, assign it to the current y coordinate of the object
float translate_spaceship_x(ALLEGRO_TRANSFORM* trans,int speed,float screen_width,float omega,float curr_x){
        float tmp_x = curr_x-(speed*cos(1.57+omega));
         if(tmp_x > screen_width)
                 tmp_x=screen_width;
         if( tmp_x < 0)
                 tmp_x=0;
         al_translate_transform(trans,tmp_x-curr_x,0);
        return tmp_x;
 }
// use to move an object on the screen along the direction of its speed, assign it to the current x coordinate of the object
 float translate_spaceship_y(ALLEGRO_TRANSFORM* trans,int speed,float screen_height,float omega,float curr_y){
         float tmp_y=curr_y-(speed*sin(1.57+omega));
         if(tmp_y>screen_height)
                   tmp_y=screen_height;
         if(tmp_y<0)
                 tmp_y=0;
 
         al_translate_transform(trans,0,tmp_y-curr_y);
         return tmp_y;
 }
void rotate_object(ALLEGRO_TRANSFORM* trans,float curr_x,float curr_y,float omega){
        al_translate_transform(trans,-curr_x,-curr_y);
        al_rotate_transform(trans,omega);
        al_translate_transform(trans,curr_x,curr_y);
}


