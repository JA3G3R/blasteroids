#include "../include/includes_for_prototype2.h"
game_t* game;
extern ALLEGRO_TRANSFORM *trans;
extern pthread_mutex_t ast_lock_collider;
ALLEGRO_TRANSFORM local_trans;
ALLEGRO_FONT* local_font; 
//ALLEGRO_TRANSFORM death_trans; 

void generate_broken_asteroids(asteroid* broken,float x_multiplier,float y_multiplier){
        asteroid* smaller1=(asteroid* )(malloc(sizeof(asteroid)));
        asteroid* smaller2=(asteroid* )(malloc(sizeof(asteroid)));
        
        float speed=sqrt(broken->speed_x*broken->speed_x + broken->speed_y*broken->speed_y);

        //smaller1 
        {
                smaller1->alive=true;
                smaller1->scale=0.5;
                smaller1->omega=broken->omega;
                smaller1->current_rotation=broken->current_rotation+0.5236;
                smaller1->speed_x=speed*x_multiplier;
                smaller1->speed_y=speed*y_multiplier;
                smaller1->color=al_map_rgb(0,0,255);
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
                smaller2->color=al_map_rgb(0,0,255);
                smaller2->trans=(ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
                al_identity_transform(smaller2->trans);
                smaller2->x=broken->x+smaller2->speed_x*smaller2->scale*25;
                smaller2->y=broken->y+smaller2->speed_y*smaller2->scale*25;
		        }

        asteroid* next=game->as_null->next;
	smaller1->next=smaller2;
        smaller2->next=next;
        game->as_null->next=smaller1;
       	game->asteroids_on_screen+=2;
}
	
void break_asteroid(asteroid *as,blast *bl){
        float as_speed=sqrt((as->speed_x*as->speed_x)+(as->speed_y*as->speed_y));
        float bl_speed_x=-(bl->speed*cos(1.57+bl->direction));
        float bl_speed_y=-(bl->speed*sin(1.57+bl->direction));
        float y_multiplier=-bl_speed_x/sqrt((bl_speed_x*bl_speed_x)+(bl_speed_y*bl_speed_y));
        float x_multiplier=bl_speed_y/sqrt((bl_speed_x*bl_speed_x)+(bl_speed_y*bl_speed_y));
        generate_broken_asteroids(as,x_multiplier,y_multiplier);
}

void draw_each_blast(){
        blast* ptr = game->bl_null;
        for(int i=0;i<game->blasts_on_screen;i++){
                ptr=ptr->next;
		if(ptr->x == 0 || ptr->x== SC_WIDTH ||ptr->y==0||ptr->y == SC_HEIGHT)
                        ptr->alive=false;
                if(ptr->alive){
			if(!(game->state & RESPAWNING))//we want that when respawning sp everything on the screen freezes(the following block of code updates the postion of blasts)
			{
                	ptr->x=translate_spaceship_x(ptr->trans,ptr->speed,SC_WIDTH,ptr->direction,ptr->x);
                	ptr->y=translate_spaceship_y(ptr->trans,ptr->speed,SC_HEIGHT,ptr->direction,ptr->y);
			}
                	al_use_transform(ptr->trans);
                	al_draw_line(0,0,2,0,al_map_rgb(255,0,255),7.0f);
                }
        }

}

void generate_blast(float sp_pos_x,float sp_pos_y,float direction){
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
        ptr->next=game->bl_null->next;
        game->bl_null->next=ptr;
}

void handle_sp_collision(float *sp_x,float *sp_y,float *theta){
	asteroid* ptr=game->as_null;
	int itr=game->asteroids_on_screen;
        for(int i=0;i<itr;i++){

                ptr=ptr->next;
                if(!ptr->alive) continue;

                if(is_spaceship_hit(*sp_x,*sp_y,ptr,*theta,SCALE)){

                        ptr->speed_x *=-1;
                        ptr->speed_y *= -1;

                        game->state |= SP_RESPAWN;
			game->state |= PROT_SP;
			game->lives_left--;
			*sp_x=SC_WIDTH/2;
			*sp_y=SC_HEIGHT/2;
			*theta=0;
			al_identity_transform(trans);
			al_translate_transform(trans,*sp_x,*sp_y);
			game->state |= RESPAWNING;
                        while(is_spaceship_hit(*sp_x,*sp_y,ptr,*theta,SCALE)){
                                translate_asteroid_x(ptr->trans,SC_WIDTH,&(ptr->x),&(ptr->speed_x));
                                translate_asteroid_y(ptr->trans,SC_HEIGHT,&(ptr->y),&(ptr->speed_y));
                        }
                }
        }
}

void handle_collision(){
        asteroid* ptr;
        asteroid*ptr1=game->as_null;
	int iter=game->asteroids_on_screen-1;
        for(int i=0;i<iter;i++){

                ptr1=ptr1->next;
                if(!ptr1->alive) continue;

                ptr=ptr1;
		int itr = game->asteroids_on_screen-(i+1);
                for(int j=0;j<itr;j++){
                        ptr=ptr->next;
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
        for(int i=0;i < game->asteroids_on_screen;i++){
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
		ptr->scale=1;
                ptr->alive=true;
                ptr->current_rotation=(rand()%20)*0.01+0.05;
                ptr->omega=ptr->current_rotation+0.05;
                ptr->color = al_map_rgb(0,0,255);
                ptr->trans = (ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
                al_identity_transform((ptr->trans));
        }
        return as_null;
}

void draw_each_asteroid(){
       asteroid* ptr = game->as_null;
       int itr = game->asteroids_on_screen;
        for(int i=0;i<itr;i++){
                ptr=ptr->next;
                if(ptr->alive){
			if(!(game->state & RESPAWNING)){
				// the following block of code updates the position of asteroids
				// and to implement respawn_wait we need that everything on the screen
				// freezes( i.e the asteroid positions don't get updated
                        	al_identity_transform(ptr->trans);
				al_scale_transform(ptr->trans,ptr->scale,ptr->scale);
                        	translate_asteroid_x(ptr->trans,SC_WIDTH,&(ptr->x),&(ptr->speed_x));
                        	translate_asteroid_y(ptr->trans,SC_HEIGHT,&(ptr->y),&(ptr->speed_y));
                        	rotate_object(ptr->trans,ptr->x,ptr->y,ptr->current_rotation);
                        	ptr->current_rotation+=ptr->omega;
                        	if(ptr->current_rotation>3.14159*2)
                        	ptr->current_rotation-=(3.14159*2);
			}
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

void clean_blasts(bool free_all){
        blast* prev=game->bl_null;
        blast* curr=game->bl_null;
        blast* nxt=game->bl_null->next;
        int itr= game->blasts_on_screen;
        for(int i=0;i<itr;i++){
                curr=nxt;
                if(prev->next!=nxt)
                        prev=prev->next;
                if(i!=itr-1)
                        nxt=nxt->next;
                if(!(curr->alive) || free_all){
                        game->blasts_on_screen-=1;
                        if(i!=itr-1)
                                prev->next=nxt;
                        else prev->next=NULL;
                        free(curr->trans);
                        free(curr);
                        curr=NULL;
                }
        }
}

void clean_asteroids(bool free_all){
        asteroid* prev=game->as_null;
        asteroid* curr=game->as_null;
	asteroid* nxt=game->as_null->next;
        int itr= game->asteroids_on_screen;
        for(int i=0;i<itr;i++){
                curr=nxt;
                if(prev->next!=nxt)
                        prev=prev->next;
                if(i!=itr-1)
                        nxt=nxt->next;
                if(!(curr->alive) || free_all){
                        game->asteroids_on_screen-=1;
                        if(i!=itr-1)
                                prev->next=nxt;
                        else prev->next=NULL;
                        free(curr->trans);
                        free(curr);
                        curr=NULL;
                }
        }
}

void* generate_new_asteroids(void *null){
	while(1){
	void *retval;
	if(game->state & GAME_PAUSED) pthread_exit(retval);
	int itr=game->asteroids_on_screen;
	if(itr <= 2){
	srand(time(0));
	asteroid* ptr;
        for(int i=0;i < 5; i++){
		if(game->state & GAME_PAUSED) pthread_exit(retval);
                ptr= (asteroid* )(malloc(sizeof(asteroid)));
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
		X=rand()%4;
		switch(X){
			case 0:
				ptr->x = (float)(rand()%SC_WIDTH);
				ptr->y = -23;
				break;
			case 1:
				ptr->x =  (float)(rand()%SC_WIDTH);
				ptr->y = SC_HEIGHT+23;
				break;
			case 2:
				ptr->x = -23;
				ptr->y= (float)(rand()%SC_HEIGHT);
				break;
			case 3:
				ptr->x = SC_WIDTH + 23;
				ptr->y = (float)(rand()%SC_HEIGHT);
				break;
		}
		ptr->scale=1;
                ptr->alive=true;
                ptr->current_rotation=(rand()%20)*0.01+0.05;
                ptr->omega=ptr->current_rotation+0.05;
                ptr->color = al_map_rgb(0,0,255);
                ptr->trans = (ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
                al_identity_transform((ptr->trans));
		ptr->next=game->as_null->next;
		pthread_mutex_lock(&ast_lock_collider);
		game->as_null->next=ptr;
		pthread_mutex_unlock(&ast_lock_collider);
		game->asteroids_on_screen++;

        }
	}
}
}

void respawn_wait(){
	int wait_timer=0;
	int time_remaining=3;
	char string[20];
	local_font= al_create_builtin_font();
	if(!local_font){
		printf("local_font variable can't be initialized\n");
		exit(1);
	}
	ALLEGRO_EVENT_QUEUE* local_queue=al_create_event_queue();
	ALLEGRO_TIMER* timer_loc=al_create_timer(1.0/30.0);
	ALLEGRO_EVENT local_event;
	al_register_event_source(local_queue,al_get_timer_event_source(timer_loc));

	al_identity_transform(&local_trans);
	al_scale_transform(&local_trans,2,2);
	al_translate_transform(&local_trans,SC_WIDTH/2,SC_HEIGHT/2);
	al_use_transform(&local_trans);
//	al_flush_event_queue(game->queue);
	sprintf(string,"Respawning in ... %d",time_remaining);
	al_start_timer(timer_loc);
	while(1){	
		
		al_wait_for_event(local_queue,&(local_event));
		
		if(wait_timer==30){
			time_remaining--;
			if(time_remaining== 0){	
				al_destroy_font(local_font);
				al_destroy_event_queue(local_queue);
				al_destroy_timer(timer_loc);
				break;
			}
			wait_timer=0;
			sprintf(string,"Respawning in ... %d",time_remaining);
		}
		if(game->event.type==ALLEGRO_EVENT_TIMER){
			wait_timer++;
			al_clear_to_color(al_map_rgb(0,0,0));
			al_draw_text(local_font,al_map_rgb(255,255,255),0,0,ALLEGRO_ALIGN_CENTRE,string);
			al_flip_display();
			
		}
		
	}
	game->state &= ~(RESPAWNING);
	al_flush_event_queue(game->queue);
}

/* void you_died(){
	int wait_timer=0;
	int time_remaining=3;
	char string[22];
	local_font= al_create_builtin_font();
	if(!local_font){
		printf("local_font variable can't be initialized\n");
		exit(1);
	}
	ALLEGRO_EVENT_QUEUE* local_queue=al_create_event_queue();
	ALLEGRO_TIMER* timer_loc=al_create_timer(1.0/30.0);
	ALLEGRO_EVENT local_event;
	al_register_event_source(local_queue,al_get_timer_event_source(timer_loc));
	ALLEGRO_FONT* death_font=al_create_builtin_font();

	al_identity_transform(&death_trans);
	al_scale_transform(&death_trans,2,2);
	al_use_transform(&death_trans);
	sprintf(string,"Back to menu in ... %d",time_remaining);
	al_start_timer(timer_loc);
	while(1){	
		
		al_wait_for_event(local_queue,&(local_event));
		
		if(wait_timer==30){
			time_remaining--;
			if(time_remaining== 0){	
				al_destroy_font(death_font);
				al_destroy_event_queue(local_queue);
				al_destroy_timer(timer_loc);
				break;
			}

			sprintf(string,"Back to menu in ... %d",time_remaining);
		}
		if(game->event.type==ALLEGRO_EVENT_TIMER){
			wait_timer++;
			al_clear_to_color(al_map_rgb(0,0,0));
			al_draw_text(local_font,al_map_rgb(255,255,255),SC_WIDTH/2,SC_HEIGHT/2,ALLEGRO_ALIGN_CENTRE,string);
			al_draw_text(local_font,al_map_rgb(255,255,255),SC_WIDTH/2,SC_HEIGHT/2+15,ALLEGRO_ALIGN_CENTRE,"YOU DIED !!!");
			al_flip_display();
			
		}
		
	}
	al_flush_event_queue(game->queue);
} */
