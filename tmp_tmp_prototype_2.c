#include "./include/includes_for_prototype2.h"
#define OMEGA	0.12
#define SC_WIDTH	1200
#define SC_HEIGHT	650
#define	ACC	0.1
#define DECC	0.5
#define MAX_SPEED	10
#define SCALE	25
#define SPEED_X	8 
#define SPEED_Y 4

/* flags */
#define GAME_STARTED	0b00000001
#define GAME_PAUSED	0b00000010
#define GAME_NEW	0b00000100
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_cond=PTHREAD_COND_INITIALIZER;
extern pthread_mutex_t lock_count_collider;
extern pthread_mutex_t lock_count_main;
float x,y,theta_in_radians,speed; 
extern game_t* game;
ALLEGRO_TRANSFORM* trans;
ALLEGRO_FONT* font;
	
/* typedef struct sp{
	float theta_in_radians;
	float speed;
	float x;
	float y;
}sp;
sp* hero={0,0,SC_WIDTH/2,SC_HEIGHT/2};
*/

void initialize_variables(){ /* ....or effectively start fresh game */
	x=(SC_WIDTH)/2;
	y=(SC_HEIGHT)/2;
	theta_in_radians=0;
	speed=0; 
	game->state=(GAME_PAUSED | GAME_NEW);
	game->score=0;
	game->blasts_on_screen=0;
	game->asteroids_on_screen=4;
	if(game->bl_null) free(game->bl_null);
	game->bl_null = (blast*)(malloc(sizeof(blast)));
	game->bl_null->alive=true;
	game->as_null=generate_asteroids();
	al_identity_transform(trans);
	al_translate_transform(trans,x,y);
}
/* below two functions are thread functions */
void* handle_blast(void* null){

while(1){
	if(!(game->state & GAME_PAUSED)){
	pthread_mutex_lock(&lock);
	blast* bl_ptr=game->bl_null;
	for(int i=0;i<game->blasts_on_screen;i++){
		asteroid* as_ptr=game->as_null;
		if(bl_ptr->next){
			bl_ptr=bl_ptr->next;	
			for(int y=0;(y<game->asteroids_on_screen);y++){
				as_ptr=as_ptr->next;
				if(!as_ptr->alive) continue;
				if(calc_distance(as_ptr->x,as_ptr->y,bl_ptr->x,bl_ptr->y)<23){
					if(as_ptr->scale ==1) break_asteroid(as_ptr,bl_ptr);
						bl_ptr->alive=false;
						as_ptr->alive=false;
						break;
					
				}
			}
		}
	}
	pthread_mutex_unlock(&lock);
	} else break;
}
}

		
void *collider(void *null){
		while(1){
			
			if(!(game->state & GAME_PAUSED)){
				pthread_mutex_lock(&lock);
				if(game->state & GAME_NEW){
					pthread_cond_signal(&read_cond);
					pthread_mutex_unlock(&lock);
				       	sleep(7);
				}else{
					pthread_cond_signal(&read_cond);
				       	pthread_mutex_unlock(&lock);
				}
				pthread_cond_signal(&read_cond);
				if(game->event.type==ALLEGRO_EVENT_TIMER){
					pthread_mutex_lock(&lock);
					pthread_mutex_lock(&lock_count_collider);
					handle_sp_collision(x,y,theta_in_radians);
					pthread_mutex_unlock(&lock_count_collider);
					pthread_mutex_unlock(&lock);
				}	
			}
			else break;
		}
	
}
/* Above two functions are thread functions */
void play_game(){
	pthread_t blast_handler;
	if(pthread_create(&blast_handler,NULL,handle_blast,NULL))
	{	printf("FATAL : Could not create blast handling thread : %s",strerror(errno));
		exit(1);
	}
	pthread_mutex_lock(&lock);
	pthread_cond_wait(&read_cond,&lock);
	game->state &= ~(GAME_NEW);
	pthread_mutex_unlock(&lock);

	
	ALLEGRO_COLOR acolor=al_map_rgb(0,255,0);
	bool left=false,right=false,up=false,down=false,space=false,shift=false,redraw=false,escape=false;
	while(1){
		al_wait_for_event(game->queue,&game->event);
		if(game->event.type==ALLEGRO_EVENT_TIMER){
			redraw=true; 
			pthread_mutex_lock(&lock);
			clean_blasts();
			pthread_mutex_unlock(&lock);
			pthread_mutex_lock(&lock_count_main);
			handle_collision();
			pthread_mutex_unlock(&lock_count_main);
		}
		
		if(game->event.type==ALLEGRO_EVENT_KEY_DOWN){
			switch(game->event.keyboard.keycode){
				case(ALLEGRO_KEY_D):
					right=true;
					break;
				case(ALLEGRO_KEY_A):
					left=true;
					break;
				case(ALLEGRO_KEY_W):
					up=true;
					break;
				case(ALLEGRO_KEY_S):	
					down=true;
					break;
				case(ALLEGRO_KEY_SPACE):
					space=true;
					break;
				case(ALLEGRO_KEY_LSHIFT):
					shift=true;
					break;

				case(ALLEGRO_KEY_ESCAPE):
					escape=true;
					break;
			}

		}
		
		
		if(game->event.type==ALLEGRO_EVENT_KEY_UP){
				switch(game->event.keyboard.keycode){
				case(ALLEGRO_KEY_D):
					right=false;
					break;
				case(ALLEGRO_KEY_A):
					left=false;
					break;
				case(ALLEGRO_KEY_W):
					up=false;
					break;
				case(ALLEGRO_KEY_S):	
					down=false;
					break;
				case(ALLEGRO_KEY_LSHIFT):
					shift=false;
					break;
	

				}	
			}
		
		if(!up ){
			speed=0;
		}
		
		if(up)
		{
			speed=4;
			if(shift){
				if(speed<MAX_SPEED)
					speed+=ACC;
			}
		}
		if(left)
		{
			if((theta_in_radians -= OMEGA)<0)
				theta_in_radians = 6.2832+theta_in_radians;

			rotate_object(trans,x,y,-OMEGA);
		}
		if(right)
		{
			if((theta_in_radians += OMEGA)>6.2832)
				theta_in_radians-=6.2832;
			rotate_object(trans,x,y,OMEGA);
		}
		if(space)
		{
			pthread_mutex_lock(&lock);
			generate_blast(x,y,theta_in_radians);
			pthread_mutex_unlock(&lock);
			game->blasts_on_screen++;
			space=false;
		}
		if(escape){
			game->state |= GAME_STARTED;
			game->state |= GAME_PAUSED;
			break;
		}
		
		if(game->event.type==ALLEGRO_EVENT_DISPLAY_CLOSE)
			exit(1);

		if(redraw && al_is_event_queue_empty(game->queue)){
			al_clear_to_color(al_map_rgb(0,0,0));
				pthread_mutex_lock(&lock);
				pthread_mutex_lock(&lock_count_main);
				draw_each_asteroid();
				pthread_mutex_unlock(&lock_count_main);
				pthread_mutex_unlock(&lock);
				if(speed){
					x= translate_spaceship_x(trans,speed,SC_WIDTH,theta_in_radians,x);
	                        	y=translate_spaceship_y(trans,speed,SC_HEIGHT,theta_in_radians,y);
				}
				
				al_use_transform(trans);
				al_draw_line(SCALE*(0.5),SCALE*(0.28867),0,SCALE*(-0.577),acolor,3.0f);
       		                al_draw_line(SCALE*(-0.333),0,SCALE*(-0.1),0,acolor,3.0f);
                	        al_draw_line(SCALE*(0.333),0,SCALE*(0.1),0,acolor,3.0f);
                        	al_draw_line(SCALE*(-0.5),SCALE*(0.28867),0,SCALE*(-0.577),acolor,3.0f);
				
				if(game->blasts_on_screen)
					draw_each_blast();
				al_flip_display();

			}
	}
/*	void *status;
	if(pthread_join(collider_thread,&status)==-1){
		printf("FATAL : Could not create collider thread : %s",strerror(errno));
		exit(1);
	}
	if(pthread_join(blast_handler,&status)==-1){
		printf("FATAL : Could not join blast handler thread : %s",strerror(errno));
		exit(1);
	}*/
}


void menu(){
	bool play_first=true,play_set=false;
	bool exit_first=true,exit_set=false;
	bool new_game_first=true,new_game_set=false;
	bool is_exit_selected,is_play_selected,is_new_game_selected;
	bool redraw=true;
	
	ALLEGRO_TRANSFORM menu_exit;		
	ALLEGRO_TRANSFORM menu_play;		
	ALLEGRO_TRANSFORM menu_new_game;		
	al_identity_transform(&menu_play);
	al_identity_transform(&menu_exit);
	al_identity_transform(&menu_new_game);
	al_translate_transform(&menu_play,50,SC_HEIGHT-(5*(SC_HEIGHT)/7));
	al_translate_transform(&menu_new_game,50,SC_HEIGHT-(5*(SC_HEIGHT)/7)+40);
	al_translate_transform(&menu_exit,50,SC_HEIGHT-(5*(SC_HEIGHT)/7)+80);
	
	while(1){	
		al_wait_for_event(game->queue,&game->event);
		if(game->event.type==ALLEGRO_EVENT_TIMER){
			redraw=true;
		}
		if(game->event.type==ALLEGRO_EVENT_DISPLAY_CLOSE){
			exit(1);
		}
		if(game->event.type==ALLEGRO_EVENT_MOUSE_AXES){
			is_play_selected=select_option(&menu_play,&play_first,&play_set,game->event.mouse.x,game->event.mouse.y,40,150,SC_HEIGHT-(5*(SC_HEIGHT)/7)-10,SC_HEIGHT-(5*(SC_HEIGHT)/7)+10);
			if(game->state & GAME_STARTED){	is_exit_selected=select_option(&menu_exit,&exit_first,&exit_set,game->event.mouse.x,game->event.mouse.y,40,100,SC_HEIGHT-(5*(SC_HEIGHT)/7)+70,SC_HEIGHT-(5*(SC_HEIGHT)/7)+90);
			is_new_game_selected=select_option(&menu_new_game,&new_game_first,&new_game_set,game->event.mouse.x,game->event.mouse.y,40,150,SC_HEIGHT-(5*(SC_HEIGHT)/7)+30,SC_HEIGHT-(5*(SC_HEIGHT)/7)+50);}
			else {
			is_exit_selected=select_option(&menu_exit,&exit_first,&exit_set,game->event.mouse.x,game->event.mouse.y,40,150,SC_HEIGHT-(5*(SC_HEIGHT)/7)+30,SC_HEIGHT-(5*(SC_HEIGHT)/7)+50);

			}
		}
		if(game->event.type==ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
			if(is_play_selected){
				game->state |= GAME_STARTED;
				game->state &= ~GAME_PAUSED;
				break;
			}

			if(is_exit_selected)
				//save_and_exit();
				exit(1);

			if(is_new_game_selected){
				initialize_variables();
				game->state &= ~GAME_PAUSED;
				game->state &= GAME_NEW;
				break;
			}
						
		}
		if(redraw && al_is_event_queue_empty(game->queue)){

			al_clear_to_color(al_map_rgb(0,0,0));

			al_use_transform(&menu_exit);
			al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"EXIT");
			if(game->state & GAME_STARTED){
				al_use_transform(&menu_new_game);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"NEW GAME");
				al_use_transform(&menu_play);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"RESUME GAME");
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"EXIT");
			}
			else {
				al_use_transform(&menu_play);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"PLAY GAME");
				al_identity_transform(&menu_exit);
				al_translate_transform(&menu_exit,50,SC_HEIGHT-(5*(SC_HEIGHT)/7)+40);
				al_use_transform(&menu_exit);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"EXIT");
			}
			al_flip_display();
			
		}/*end if*/
	}/* end while */
}

int main(){
	al_init();
	al_init_primitives_addon();
	al_install_keyboard();
	al_init_font_addon();
	al_install_mouse();
	
	trans=(ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
	game=(game_t*)(malloc(sizeof(game_t)));
	initialize_variables();

	game->disp = al_create_display(SC_WIDTH,SC_HEIGHT);
	game->timer = al_create_timer(1.0/30.0);
	game->queue= al_create_event_queue();
	font = al_create_builtin_font();
	

	al_register_event_source(game->queue,al_get_timer_event_source(game->timer));
	al_register_event_source(game->queue,al_get_keyboard_event_source());
	al_register_event_source(game->queue,al_get_display_event_source(game->disp));
	al_register_event_source(game->queue,al_get_mouse_event_source());
	
	al_start_timer(game->timer);
	if(pthread_create(&collider_thread,NULL,collider,NULL)){
		printf("FATAL : Could not create collider thread : %s",strerror(errno));
		exit(1);
	}
		while(1){
		if(game->state & GAME_PAUSED)
			menu();
		else play_game();
	
	}


free(game->bl_null);
al_shutdown_font_addon();
al_shutdown_primitives_addon();
al_uninstall_mouse();
al_uninstall_keyboard();
al_destroy_timer(game->timer);
al_destroy_display(game->disp);
al_destroy_event_queue(game->queue);
al_destroy_font(font);
}

