#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "./include/include_allegro.h"
#include "./include/menu_functions.h"
#include "./include/animation_funcs.h"
#include "./include/collision.h"
#include <math.h>
#define OMEGA	0.12
#define SC_WIDTH	1200
#define SC_HEIGHT	650
#define	ACC	0.1
#define DECC	0.5
#define MAX_SPEED	10
#define SCALE	25
#define SPEED_X	8 
#define SPEED_Y 4
int nr_asteroids=3;
pthread_mutex_t lock_count_collider=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_count_main=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_break = PTHREAD_MUTEX_INITIALIZER;
ALLEGRO_TIMER* timer;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_EVENT event;
float x=(SC_WIDTH)/2,y=(SC_HEIGHT)/2,theta_in_radians=0,speed=0; 
int blasts_on_screen=0;
asteroid* as_null;
bool game_started=false,game_paused=false;

/* typedef struct sp{
	float theta_in_radians;
	float speed;
	float x;
	float y;
}sp;
sp* hero={0,0,SC_WIDTH/2,SC_HEIGHT/2};
*/

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
blast* bl_null;

void generate_broken_asteroids(asteroid* broken,float x_multiplier,float y_multiplier);
void break_asteroid(asteroid *as,blast *bl);
void clean_blasts(blast* bl_null,int* blasts_on_screen){
	blast* prev=bl_null;
	blast* curr=bl_null;
	blast* nxt=bl_null->next;
	int itr= *blasts_on_screen;
	for(int i=0;i<itr;i++){
		curr=nxt;
		if(prev->next!=nxt)
			prev=prev->next;
		if(i!=itr-1)
			nxt=nxt->next;
		if(!(curr->alive)){
			if(i!=itr-1)
				prev->next=nxt;
			else prev->next=NULL;
			free(curr->trans);
			free(curr);
			curr=NULL;
			*blasts_on_screen-=1;
		}
	}

}

void draw_each_blast(blast* bl_null,int* blasts_on_screen){
	blast* ptr = bl_null;
	for(int i=0;i<*blasts_on_screen;i++){
		ptr=ptr->next;
		if(ptr->x == 0 || ptr->x == SC_WIDTH ||ptr->y==0||ptr->y == SC_HEIGHT)
			ptr->alive=false;
		if(ptr->alive){
		ptr->x=translate_spaceship_x(ptr->trans,ptr->speed,SC_WIDTH,ptr->direction,ptr->x);
		ptr->y=translate_spaceship_y(ptr->trans,ptr->speed,SC_HEIGHT,ptr->direction,ptr->y);
		al_use_transform(ptr->trans);
		al_draw_line(0,0,1,0,al_map_rgb(0,255,0),3.0f);
		}
	}

}

void generate_blast(blast* bl_null,float sp_pos_x,float sp_pos_y,float direction){
	blast* ptr=(blast*)(malloc(sizeof(blast)));
	ptr->x=sp_pos_x;
	ptr->y=sp_pos_y;
	ptr->speed = MAX_SPEED*5;
	ptr->alive=true;
	ptr->direction=direction;
	ptr->trans = (ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
	al_identity_transform(ptr->trans);
	al_translate_transform(ptr->trans,ptr->x,ptr->y);
	rotate_object(ptr->trans,ptr->x,ptr->y,ptr->direction);
	ptr->next=bl_null->next;
	bl_null->next=ptr;
}

void handle_sp_collision(asteroid* as_null,float sp_x,float sp_y,float theta){
        asteroid* ptr=as_null;
        for(int i=0;i<nr_asteroids;i++){
		pthread_mutex_lock(&lock_break);
                ptr=ptr->next;
		pthread_mutex_unlock(&lock_break);
		if(!ptr->alive) continue;

                if(is_spaceship_hit(sp_x,sp_y,ptr,theta,SCALE)){
			
                        ptr->speed_x *=-1;
                        ptr->speed_y *= -1;

                        printf("YOU DIED\n");
                        while(is_spaceship_hit(sp_x,sp_y,ptr,theta,SCALE)){
                                translate_asteroid_x(ptr->trans,SC_WIDTH,&(ptr->x),&(ptr->speed_x));
                                translate_asteroid_y(ptr->trans,SC_HEIGHT,&(ptr->y),&(ptr->speed_y));

                        }
                }
        }

}

void handle_collision(asteroid* as_null){
        asteroid* ptr;
        asteroid*ptr1=as_null;
        for(int i=0;i<nr_asteroids-1;i++){
		pthread_mutex_lock(&lock_break);
                ptr1=ptr1->next;
		pthread_mutex_unlock(&lock_break);
		if(!ptr1->alive) continue;

                ptr=ptr1;
                for(int j=0;j<nr_asteroids-(i+1);j++){
			pthread_mutex_lock(&lock_break);
                        ptr=ptr->next;
			pthread_mutex_unlock(&lock_break);
			if(!ptr->alive) continue;
                        if(is_collision_detected(ptr,ptr1))
                                on_collision(ptr1,ptr);
                }
        }

}
asteroid* generate_asteroids(){
        srand(time(0));
        asteroid* as_null = (asteroid* )malloc(sizeof(asteroid));
        asteroid* ptr = as_null;
	ptr->alive=true;
        for(int i=0;i < nr_asteroids;i++){
                ptr->next = (asteroid* )(malloc(sizeof(asteroid)));
                ptr=ptr->next;
                int X=rand()%2;
                int Y=rand()%2;
                int is_negative_X = 1;
                int is_negative_Y = 1;
                for(int i=0;i<=X;i++){
                         is_negative_X*=-1;
                 }
                for(int i=0;i<=Y;i++){
                        is_negative_Y*=-1;
                }
                ptr->speed_x = ((float)(rand())/(float)(RAND_MAX))*SPEED_X*is_negative_X;
                ptr->speed_y=((float)(rand())/(float)(RAND_MAX))*SPEED_Y*is_negative_Y;
                ptr->x = (float)(rand()%SC_WIDTH);
                ptr->y= (float)(rand()%SC_HEIGHT);
		ptr->alive=true;
		ptr->scale=1.0;
                ptr->current_rotation=(rand()%20)*0.01+0.05;
                ptr->omega=ptr->current_rotation+0.05;
                ptr->color = al_map_rgb(255,255,255);
                ptr->trans = (ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
                al_identity_transform((ptr->trans));
        }
        return as_null;
}

void* handle_blast(void* null){

	while(1){
	pthread_mutex_lock(&lock);
	blast* bl_ptr=bl_null;
	for(int i=0;i<blasts_on_screen;i++){
		asteroid* as_ptr=as_null;
		if(bl_ptr->next){
			bl_ptr=bl_ptr->next;	
			if(bl_ptr->alive){
				for(int y=0;(y<nr_asteroids);y++){
					as_ptr=as_ptr->next;
					if(!as_ptr->alive) continue;
					if(calc_distance(as_ptr->x,as_ptr->y,bl_ptr->x,bl_ptr->y)<23){
						break_asteroid(as_ptr,bl_ptr);	
						bl_ptr->alive=false;
						break;
					}
				}
			}
		}
	}
	pthread_mutex_unlock(&lock);
	}
}

void draw_each_asteroid(asteroid* ast_null){
        asteroid* ptr = ast_null;
        for(int i=0;i<nr_asteroids;i++){
        	ptr=ptr->next;
		if(ptr->alive){
        		al_identity_transform(ptr->trans);
			al_scale_transform(ptr->trans,ptr->scale,ptr->scale);
        		translate_asteroid_x(ptr->trans,SC_WIDTH,&(ptr->x),&(ptr->speed_x));
        		translate_asteroid_y(ptr->trans,SC_HEIGHT,&(ptr->y),&(ptr->speed_y));
        		rotate_object(ptr->trans,ptr->x,ptr->y,ptr->current_rotation);
        		ptr->current_rotation+=ptr->omega;
        		if(ptr->current_rotation>3.14159*2)
        	        ptr->current_rotation-=(3.14159*2);
        		al_use_transform(ptr->trans);
        		al_draw_line(-20, 20, -25, 5, ptr->color, 2.0f);
        		al_draw_line(-25, 5, -25, -10, ptr->color, 2.0f);
        		al_draw_line(-25, -10, -5, -10, ptr->color, 2.0f);
        		al_draw_line(-5, -10, -10, -20, ptr->color, 2.0f);
        		al_draw_line(-10, -20, 5, -20, ptr->color, 2.0f);
        		al_draw_line(5, -20, 20, -10, ptr->color, 2.0f);
       			al_draw_line(20, -10, 20, -5, ptr->color, 2.0f);
        		al_draw_line(20, -5, 0, 0, ptr->color, 2.0f);
        		al_draw_line(0, 0, 20, 10, ptr->color, 2.0f);
        		al_draw_line(20, 10, 10, 20, ptr->color, 2.0f);
        		al_draw_line(10, 20, 0, 15, ptr->color, 2.0f);
        		al_draw_line(0, 15, -20, 20, ptr->color, 2.0f);
        	}
	}
}
void generate_broken_asteroids(asteroid* broken,float x_multiplier,float y_multiplier){
	asteroid* smaller1=(asteroid* )(malloc(sizeof(asteroid)));
	asteroid* smaller2=(asteroid* )(malloc(sizeof(asteroid)));
	asteroid* next=as_null->next;
	pthread_mutex_lock(&lock_count_main);
	pthread_mutex_lock(&lock_count_collider);
	as_null->next=smaller1;
	smaller1->next=smaller2;
	smaller2->next=next;
	nr_asteroids+=2;
	
	float speed=sqrt(broken->speed_x*broken->speed_x + broken->speed_y*broken->speed_y);

	//smaller1 
	{
		smaller1->alive=true;
		smaller1->scale=0.5;
		smaller1->omega=broken->omega;
		smaller1->current_rotation=broken->current_rotation+0.5236;	
		smaller1->speed_x=speed*x_multiplier;
		smaller1->speed_y=speed*y_multiplier;
		smaller1->color=al_map_rgb(255,255,255);
		smaller1->trans=(ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
		al_identity_transform(smaller1->trans);
		smaller1->x=broken->x+smaller1->speed_x*smaller1->scale*25;
		smaller1->y=broken->y+smaller1->speed_y*smaller1->scale*25;


	}
	//smaller2
	{
		smaller2->alive=true;
		smaller2->scale=0.5;
		smaller2->omega=-broken->omega;
		smaller2->current_rotation=broken->current_rotation-0.5236;
		smaller2->speed_x=-speed*x_multiplier;
		smaller2->speed_y=-speed*y_multiplier;
		smaller2->color=al_map_rgb(255,255,255);
		smaller2->trans=(ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
		al_identity_transform(smaller2->trans);
		smaller2->x=broken->x+smaller2->speed_x*smaller2->scale*25;
		smaller2->y=broken->y+smaller2->speed_y*smaller2->scale*25;
	}	
	broken->alive=false;
	pthread_mutex_unlock(&lock_count_collider);
	pthread_mutex_unlock(&lock_count_main);
}
			
	
void break_asteroid(asteroid *as,blast *bl){
	float as_speed=sqrt((as->speed_x*as->speed_x)+(as->speed_y*as->speed_y));	
	float bl_speed_x=-(bl->speed*cos(1.57+bl->direction));	
	float bl_speed_y=-(bl->speed*sin(1.57+bl->direction));	
	float y_multiplier=-bl_speed_x/sqrt((bl_speed_x*bl_speed_x)+(bl_speed_y*bl_speed_y));	
	float x_multiplier=bl_speed_y/sqrt((bl_speed_x*bl_speed_x)+(bl_speed_y*bl_speed_y));	
	generate_broken_asteroids(as,x_multiplier,y_multiplier);
}	

void *collider(void *null){
	while(1){
		if(game_started && !game_paused){
			sleep(7);
			break;
		}
	}
		while(1){
			if(!game_paused){
				if(event.type==ALLEGRO_EVENT_TIMER){
					pthread_mutex_lock(&lock_count_collider);
					handle_sp_collision(as_null,x,y,theta_in_radians);
					pthread_mutex_unlock(&lock_count_collider);
				}
			}
		}
	
}

int main(){
	al_init();
	al_init_primitives_addon();
	al_install_keyboard();
	al_init_font_addon();
	al_install_mouse();
	
	pthread_t collider_thread;
	pthread_t blast_handler;
	if(pthread_create(&collider_thread,NULL,collider,NULL)){
		printf("FATAL : Could not create collider thread : %s",strerror(errno));
		return 1;
	}
	if(pthread_create(&blast_handler,NULL,handle_blast,NULL))
	{	printf("FATAL : Could not create blast handling thread : %s",strerror(errno));
		return 1;
	}

	ALLEGRO_DISPLAY* disp = al_create_display(SC_WIDTH,SC_HEIGHT);
	timer = al_create_timer(1.0/30.0);
	queue= al_create_event_queue();
	ALLEGRO_FONT* font = al_create_builtin_font();

	ALLEGRO_TRANSFORM* trans = (ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));

	ALLEGRO_TRANSFORM menu_exit;
	ALLEGRO_TRANSFORM menu_play;
	
	as_null=generate_asteroids();


	al_identity_transform(&menu_play);
	al_identity_transform(&menu_exit);
	al_identity_transform(trans);
	al_translate_transform(trans,x,y);

	
	ALLEGRO_COLOR acolor = al_map_rgb(0,255,0);

	al_register_event_source(queue,al_get_timer_event_source(timer));
	al_register_event_source(queue,al_get_keyboard_event_source());
	al_register_event_source(queue,al_get_display_event_source(disp));
	al_register_event_source(queue,al_get_mouse_event_source());
	
	bl_null = (blast*)(malloc(sizeof(blast)));
	bl_null->alive=true;
	bool left=false,right=false,up=false,down=false,space=false,shift=false;
	bool redraw=true;
	bool in_menu_window=true;
	bool play_first=true,play_set=false;
	bool exit_first=true,exit_set=false;
	bool is_exit_selected,is_play_selected;
	al_start_timer(timer);
	while(1){
		al_wait_for_event(queue,&event);
		if(event.type==ALLEGRO_EVENT_TIMER){
			redraw=true; 
			pthread_mutex_lock(&lock);
			clean_blasts(bl_null,&blasts_on_screen);
			pthread_mutex_unlock(&lock);
			if(game_started && !game_paused){
				pthread_mutex_lock(&lock_count_main);
				handle_collision(as_null);
				pthread_mutex_unlock(&lock_count_main);
			}
				
		}
		if(game_started && !game_paused){
			if(event.type==ALLEGRO_EVENT_KEY_DOWN){
			switch(event.keyboard.keycode){
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
					game_paused=true;
					
					in_menu_window=true;
					break;
			}

			}
		}
		
		if(event.type==ALLEGRO_EVENT_KEY_UP){
			if(game_started && !game_paused){
				switch(event.keyboard.keycode){
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
					true;
				

				}	
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
			generate_blast(bl_null,x,y,theta_in_radians);
			pthread_mutex_unlock(&lock);
			blasts_on_screen++;
			space=false;
		}
		if(event.type==ALLEGRO_EVENT_MOUSE_AXES){
			is_play_selected=select_option(&menu_play,&play_first,&play_set,event.mouse.x,event.mouse.y,40,150,SC_HEIGHT-(5*(SC_HEIGHT)/7)-10,SC_HEIGHT-(5*(SC_HEIGHT)/7)+10);
			
			is_exit_selected=select_option(&menu_exit,&exit_first,&exit_set,event.mouse.x,event.mouse.y,40,100,SC_HEIGHT-(5*(SC_HEIGHT)/7)+30,SC_HEIGHT-(5*(SC_HEIGHT)/7)+50);
		}
	
		if(event.type==ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
			if(is_play_selected){
				if(game_started && game_paused)
					game_paused=false;
				if(!game_started)
					game_started=true;
				in_menu_window=false;
			}
			if(is_exit_selected)
				break;


		}
		if(event.type==ALLEGRO_EVENT_DISPLAY_CLOSE)
			break;

		if(redraw && al_is_event_queue_empty(queue)){
			al_clear_to_color(al_map_rgb(0,0,0));
			if(game_started && !game_paused){
				pthread_mutex_lock(&lock_count_main);
				draw_each_asteroid(as_null);
				pthread_mutex_unlock(&lock_count_main);
				if(speed){
					x= translate_spaceship_x(trans,speed,SC_WIDTH,theta_in_radians,x);
	                        	y=translate_spaceship_y(trans,speed,SC_HEIGHT,theta_in_radians,y);
				}
				
				al_use_transform(trans);
				al_draw_line(SCALE*(0.5),SCALE*(0.28867),0,SCALE*(-0.577),acolor,3.0f);
       		                al_draw_line(SCALE*(-0.333),0,SCALE*(-0.1),0,acolor,3.0f);
                	        al_draw_line(SCALE*(0.333),0,SCALE*(0.1),0,acolor,3.0f);
                        	al_draw_line(SCALE*(-0.5),SCALE*(0.28867),0,SCALE*(-0.577),acolor,3.0f);
				
				if(blasts_on_screen)
					draw_each_blast(bl_null,&blasts_on_screen);


			}
			if(in_menu_window){
				
				al_use_transform(&menu_exit);
				al_draw_text(font,al_map_rgb(255,255,255),50,SC_HEIGHT-(5*(SC_HEIGHT)/7)+40,0,"EXIT");
				al_use_transform(&menu_play);
				if(game_paused)
					al_draw_text(font,al_map_rgb(255,255,255),50,SC_HEIGHT-(5*(SC_HEIGHT)/7),0,"RESUME GAME");
				else al_draw_text(font,al_map_rgb(255,255,255),50,SC_HEIGHT-(5*(SC_HEIGHT)/7),0,"PLAY GAME");
			}
				al_flip_display();
				redraw = false;
		}
		
	}

free(bl_null);
al_shutdown_font_addon();
al_shutdown_primitives_addon();
al_uninstall_mouse();
al_uninstall_keyboard();
al_destroy_timer(timer);
al_destroy_display(disp);
al_destroy_event_queue(queue);
al_destroy_font(font);
}
