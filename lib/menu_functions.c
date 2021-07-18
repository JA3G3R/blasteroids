#include "../include/menu_functions.h"
//its kinda embarassing but u have to assign separate 'first' and 'set' variables for each option
// also initialize [first]='true' and [set]='false'
bool select_option(ALLEGRO_TRANSFORM* trans,bool* first,bool* set,int mouse_x,int mouse_y,int min_x,int max_x,int min_y,int max_y){
	if(mouse_x > min_x && mouse_x < max_x && mouse_y > min_y  && mouse_y < max_y){
		if(*first){
			al_translate_transform(trans,0,-10);
			*first = false;
	                *set=true;
		}
	 }


	 if(mouse_x < min_x || mouse_x > max_x || mouse_y < min_y || mouse_y > max_y){
	 	if(!(*first)){
			al_translate_transform(trans,0,10);
			
		}
		if(*set){
			*first =true;
			*set=false;
		}
	
	}
	 return !(*first);

}

