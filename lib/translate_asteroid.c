#include "../include/animation_funcs.h"

void translate_asteroid_y(ALLEGRO_TRANSFORM* trans,float screen_height,float* curr_y,float* speed_y) {
	float tmp_y =(*curr_y)+(*speed_y);
	if(tmp_y<0){
		*speed_y*=-1;
		tmp_y*=-1;
	}
	if(tmp_y > screen_height){
		*speed_y*=-1;
		tmp_y = screen_height-(tmp_y-screen_height);
	}
	al_translate_transform(trans,0,tmp_y);
	*curr_y=tmp_y;
}
void translate_asteroid_x(ALLEGRO_TRANSFORM* trans,float screen_width,float* curr_x,float* speed_x){
	float tmp_x = (*curr_x) + (*speed_x);
	if(tmp_x < 0){
		*speed_x *= -1;
		tmp_x*=-1;
	}
	if(tmp_x > screen_width){
		*speed_x*= -1;
		tmp_x = screen_width-(tmp_x-screen_width);
	}
	al_translate_transform(trans,tmp_x,0);
	*curr_x = tmp_x;
}



