#include "./include/includes_for_prototype2.h"
#define NR_ASTEROIDS 8
pthread_mutex_t blast_lock_handle_blast= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blast_lock_collider= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ast_lock=PTHREAD_MUTEX_INITIALIZER;
float x,y,theta_in_radians,speed; 
extern game_t* game;
ALLEGRO_TRANSFORM* trans;
ALLEGRO_FONT* font;
extern ALLEGRO_FONT* local_font;
extern ALLEGRO_TRANSFORM local_trans;

void initialize_variables(){  
/* ....or effectively start fresh game */
	x=(SC_WIDTH)/2;
	y=(SC_HEIGHT)/2;
	theta_in_radians=0;
	speed=0; 
	game->state=(GAME_PAUSED | PROT_SP);
	game->score=0;
	game->blasts_on_screen=0;
	game->asteroids_on_screen=NR_ASTEROIDS;
	if(game->bl_null) free(game->bl_null);
	game->bl_null = (blast*)(malloc(sizeof(blast)));
	game->bl_null->alive=true;
	game->as_null=generate_asteroids();
	al_identity_transform(trans);
	al_translate_transform(trans,x,y);
}

void* handle_blast(void* null){
/*it's one of the two thread functions used by the game */
	while(1){
	if(!(game->state & RESPAWNING)){
		pthread_mutex_lock(&blast_lock_handle_blast);
		blast* bl_ptr=game->bl_null;
		for(int i=0;i<game->blasts_on_screen;i++){
			asteroid* as_ptr=game->as_null;
			if(bl_ptr->next && bl_ptr->next->alive){
				bl_ptr=bl_ptr->next;	
				pthread_mutex_lock(&ast_lock);
				int itr=game->asteroids_on_screen;
				for(int y=0;(y<itr);y++){
					as_ptr=as_ptr->next;
					if(!as_ptr->alive) continue;
					if(calc_distance(as_ptr->x,as_ptr->y,bl_ptr->x,bl_ptr->y)<23){
						if(as_ptr->scale == 1) break_asteroid(as_ptr,bl_ptr);
							bl_ptr->alive=false;
							as_ptr->alive=false;
							break;
					}
				}
				pthread_mutex_unlock(&ast_lock);
			}
	}	
	pthread_mutex_unlock(&blast_lock_handle_blast);
	} 
}

}

void *collider(void *null){
/* one of the two thread functions used by game */
	static int sleep_count=0;
	static int i=0;
		while(1){ 
			if(!((game->state & RESPAWNING))){	
				if((game->state & PROT_SP) || (game->state & NEW_GAME_STARTED) || (game->state & SP_RESPAWN)){
					if(game->state & NEW_GAME_STARTED){
						game->state &= ~(NEW_GAME_STARTED);
						game->state |= PROT_SP;
						sleep_count=0;
						i=0;
					}
					if(game->state & SP_RESPAWN){
						game->state &= ~(SP_RESPAWN);
						game->state |= PROT_SP;
						sleep_count=0;
						i=0;
					}
					for(;i<3;i++){
				       		sleep(1);
						sleep_count++;
					}
					game->state &= ~PROT_SP;
					sleep_count=0;
					i=0;
				}

				if(game->event.type==ALLEGRO_EVENT_TIMER){
					pthread_mutex_lock(&ast_lock);
					handle_sp_collision(&x,&y,&theta_in_radians);
					pthread_mutex_unlock(&ast_lock);
				}	
			}
			
		}
}

void play_game(){
	pthread_t blast_handler;
	pthread_t collider_thread;
	pthread_t create_asteroids;
	int blink_timer=0;
	if(pthread_create(&collider_thread,NULL,collider,NULL)){
		printf("FATAL : Could not create collider thread : %s",strerror(errno));
		exit(1);
	}

	if(pthread_create(&blast_handler,NULL,handle_blast,NULL))
	{	printf("FATAL : Could not create blast handling thread : %s",strerror(errno));
		exit(1);
	}
	
	if(pthread_create(&create_asteroids,NULL,generate_new_asteroids,NULL))
	{	printf("FATAL : Could not create asteroid generating thread: %s",strerror(errno));
		exit(1);
}
		
	ALLEGRO_COLOR acolor=al_map_rgb(0,255,0);
	bool left=false,right=false,up=false,down=false,space=false,shift=false,redraw=false,escape=false;
	while(1){
		if(!(game->state & RESPAWNING))
			al_wait_for_event(game->queue,&game->event);
		if(game->event.type==ALLEGRO_EVENT_TIMER){

			redraw=true; // makes sure that only the timer event triggers the display to advance
			
			pthread_mutex_lock(&blast_lock_handle_blast); // prevents stepping on the toe of handle blast thread

			if(game->blasts_on_screen)
			clean_blasts(); //cleans dead blasts if any

			pthread_mutex_unlock(&blast_lock_handle_blast);
			
			handle_collision(); // handles collision b/w asteroids

			pthread_mutex_lock(&ast_lock); 
			
			clean_asteroids(); //cleans dead asteroids

			pthread_mutex_unlock(&ast_lock);

			blink_timer++;
			
			//generate_new_asteroids();
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
			pthread_mutex_lock(&blast_lock_handle_blast);
			generate_blast(x,y,theta_in_radians);
			pthread_mutex_unlock(&blast_lock_handle_blast);
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

		if(redraw && al_is_event_queue_empty(game->queue)) {

				al_clear_to_color(al_map_rgb(0,0,0));
				draw_each_asteroid();
				
				if(!(game->state & RESPAWNING)) // we do not want the spaceship to be still visible on the screen after as-sp collision

			       	{
					if(speed){
						x= translate_spaceship_x(trans,speed,SC_WIDTH,theta_in_radians,x);
	                        		y=translate_spaceship_y(trans,speed,SC_HEIGHT,theta_in_radians,y);
					}	
					al_use_transform(trans);

					if((game->state & PROT_SP) && (blink_timer==5)) {
				       		al_draw_circle(0,0,20,al_map_rgb(255,255,255),3.0f);
						blink_timer=0;
					}
					else if(blink_timer > 5)
						blink_timer%=5;

					al_draw_line(SCALE*(0.5),SCALE*(0.28867),0,SCALE*(-0.577),acolor,3.0f);
       		                	al_draw_line(SCALE*(-0.333),0,SCALE*(-0.1),0,acolor,3.0f);

                	        	al_draw_line(SCALE*(0.333),0,SCALE*(0.1),0,acolor,3.0f);
                        		al_draw_line(SCALE*(-0.5),SCALE*(0.28867),0,SCALE*(-0.577),acolor,3.0f);
				

				if(game->blasts_on_screen)
					draw_each_blast();
				}
				else{
				       	respawn_wait();
					left=false,right=false,up=false,down=false,space=false,shift=false,escape=false;
				}

				al_flip_display();
				redraw=false;

		}
	}
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
			
			if(game->state & GAME_STARTED){
				
				is_exit_selected=select_option(&menu_exit,&exit_first,&exit_set,game->event.mouse.x,game->event.mouse.y,40,100,SC_HEIGHT-(5*(SC_HEIGHT)/7)+70,SC_HEIGHT-(5*(SC_HEIGHT)/7)+90);
				
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
				game->state |= GAME_STARTED;
				game->state &= ~GAME_PAUSED;
				game->state |= NEW_GAME_STARTED;
				game->state |= PROT_SP;
				break;
			}
						
		}
		if(redraw && al_is_event_queue_empty(game->queue)){

			al_clear_to_color(al_map_rgb(0,0,0));
			
			if(game->state & GAME_STARTED){
				al_use_transform(&menu_new_game);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"NEW GAME");
				
				al_use_transform(&menu_play);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"RESUME GAME");
				
				al_use_transform(&menu_exit);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"EXIT");
			}else {
				al_use_transform(&menu_play);
				al_draw_text(font,al_map_rgb(255,255,255),0,0,0,"PLAY GAME");

				al_use_transform(&menu_new_game);
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
