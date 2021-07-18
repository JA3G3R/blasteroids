#include "../include/collision.h"

float local_x1,local_x2,local_y1,local_y2;
float spx1,spx2,spy1,spy2;
float centre_distance;
void get_variables(asteroid* as1,asteroid *as2){
	local_x1=as1->x;
	local_x2=as2->x;
	local_y1=as1->y;
	local_y2=as2->y;
	spx1=as1->speed_x;
	spx2=as2->speed_x;
	spy1=as1->speed_y;
	spy2=as2->speed_y;
	float tmpx=local_x2-local_x1;
	float tmpy=local_y2-local_y1;
	centre_distance=sqrt((tmpx*tmpx)+(tmpy*tmpy));
}
bool is_collision_detected(asteroid* as1,asteroid* as2){
	get_variables(as1,as2);
	return centre_distance < (as1->scale*25 + as2->scale*25);
}
	
void on_collision(asteroid* as1,asteroid* as2){
	get_variables(as1,as2);
	
	//impact vector
	float imp_x=(local_x2-local_x1)/centre_distance;
	float imp_y=(local_y2-local_y1)/centre_distance;//slope of normal= (y2-y1)/(x2-x1)
	float cos_theta=imp_x/sqrt(imp_x*imp_x+imp_y*imp_y);
	float sin_theta=1-cos_theta*cos_theta;
	//tangent vector or normal to impact vector
	float tan_x=(imp_y);
	float tan_y= -(imp_x);//slope of tangent = -1/slope of normal
	
	//these are the dot pdts of speed vector and impact vector,to find component of speed along impact axis
	float imp_comp1=spx1*imp_x + spy1*imp_y; 
	float imp_comp2=spx2*imp_x +spy2*imp_y;
	float tan_comp1=spx1*tan_x+spy1*tan_y;
	float tan_comp2=spx2*tan_x+spy2*tan_y;
	
	//to execute elastic collision,the velocities along impact vector are exchanged
	//i.e the imp_comp[n] which contains the impact component
	as1->speed_x=imp_comp2*imp_x+tan_comp1*tan_x;
	as2->speed_x=imp_comp1*imp_x+tan_comp2*tan_x;
	as1->speed_y=imp_comp2*imp_y+tan_comp1*tan_y;
	as2->speed_y=imp_comp1*imp_y+tan_comp2*tan_y;
	while(is_collision_detected(as1,as2)){
		as1->x+=as1->speed_x;
		as1->y+=as1->speed_y;
		as2->x+=as2->speed_x;
		as2->y+=as2->speed_y;
	
	}
}
	

