
#include "../include/animation_funcs.h"

void translate_asteroid_y(ALLEGRO_TRANSFORM* trans,float screen_height,float* curr_y,float* speed_y) {
	float tmp_y =(*curr_y)+(*speed_y);
	if(tmp_y+23<0){
		tmp_y+=screen_height+23;
	}
	if(tmp_y-23 > screen_height){
		tmp_y = -tmp_y+screen_height;
	}
	al_translate_transform(trans,0,tmp_y);
	*curr_y=tmp_y;
}
void translate_asteroid_x(ALLEGRO_TRANSFORM* trans,float screen_width,float* curr_x,float* speed_x){
	float tmp_x = (*curr_x) + (*speed_x);
	if(tmp_x+23 < 0){
		tmp_x+=screen_width+23;
	}
	if(tmp_x - 23> screen_width){
		tmp_x = -tmp_x+screen_width;
	}
	al_translate_transform(trans,tmp_x,0);
	*curr_x = tmp_x;
}



