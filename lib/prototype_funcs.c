#include "../include/includes_for_prototype2.h"
#define SC_HEIGHT	650
#define SC_WIDTH	1200
#define ACC     0.1
#define DECC    0.5
#define MAX_SPEED       10
#define SCALE   25
#define SPEED_X 3
#define SPEED_Y 4
#define OMEGA	0.12
game_t* game;
pthread_mutex_t lock_count_main=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_count_collider=PTHREAD_MUTEX_INITIALIZER;
void generate_broken_asteroids(asteroid* broken,float x_multiplier,float y_multiplier){
        asteroid* smaller1=(asteroid* )(malloc(sizeof(asteroid)));
        asteroid* smaller2=(asteroid* )(malloc(sizeof(asteroid)));
        asteroid* next=game->as_null->next;
        pthread_mutex_lock(&lock_count_main);
        pthread_mutex_lock(&lock_count_collider);
        game->as_null->next=smaller1;
        smaller1->next=smaller2;
        smaller2->next=next;
       	game->asteroids_on_screen+=2;

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


void draw_each_blast(){
        blast* ptr = game->bl_null;
        for(int i=0;i<game->blasts_on_screen;i++){
                ptr=ptr->next;
		if(ptr->x == 0 || ptr->x== SC_WIDTH ||ptr->y==0||ptr->y == SC_HEIGHT)
                        ptr->alive=false;
                if(ptr->alive){
                	ptr->x=translate_spaceship_x(ptr->trans,ptr->speed,SC_WIDTH,ptr->direction,ptr->x);
                	ptr->y=translate_spaceship_y(ptr->trans,ptr->speed,SC_HEIGHT,ptr->direction,ptr->y);
                	al_use_transform(ptr->trans);
                	al_draw_line(0,0,1,0,al_map_rgb(0,255,0),3.0f);
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
void handle_sp_collision(float sp_x,float sp_y,float theta){
        asteroid* ptr=game->as_null;
        for(int i=0;i<game->asteroids_on_screen;i++){
                ptr=ptr->next;
                if(!ptr->alive) continue;

                if(is_spaceship_hit(sp_x,sp_y,ptr,theta,SCALE)){

                        ptr->speed_x *=-1;
                        ptr->speed_y *= -1;

                        printf("YOU DIED\n");
                        while(is_spaceship_hit(sp_x,sp_y,ptr,theta,SCALE)){
				int x=getuid();
                                translate_asteroid_x(ptr->trans,SC_WIDTH,&(ptr->x),&(ptr->speed_x));
                                translate_asteroid_y(ptr->trans,SC_HEIGHT,&(ptr->y),&(ptr->speed_y));

                        }
                }
        }
}
void handle_collision(){
        asteroid* ptr;
        asteroid*ptr1=game->as_null;
        for(int i=0;i<game->asteroids_on_screen-1;i++){
                ptr1=ptr1->next;
                if(!ptr1->alive) continue;

                ptr=ptr1;
                for(int j=0;j<game->asteroids_on_screen-(i+1);j++){
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
                ptr->color = al_map_rgb(255,255,255);
                ptr->trans = (ALLEGRO_TRANSFORM*)(malloc(sizeof(ALLEGRO_TRANSFORM)));
                al_identity_transform((ptr->trans));
        }
        return as_null;
}
void draw_each_asteroid(){
        asteroid* ptr = game->as_null;
        for(int i=0;i<game->asteroids_on_screen;i++){
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
void clean_blasts(){
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
                if(!(curr->alive)){
                        if(i!=itr-1)
                                prev->next=nxt;
                        else prev->next=NULL;
                        free(curr->trans);
                        free(curr);
                        curr=NULL;
                        game->blasts_on_screen-=1;
                }
        }
}


