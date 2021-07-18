#include "../include/collision.h"
#include <math.h>
vertex vertices[3];


float calc_distance(float x1,float y1,float x2,float y2){
	return sqrtf((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}
float distance_from_line(vertex* v1,vertex* v2,asteroid* as){
	float tmp= (as->x)*(v2->y-v1->y) - (as->y)*(v2->x-v1->x) -(v1->x)*(v2->y) + (v1->y)*(v2->x);
	float ret_val= tmp/(sqrtf((v2->y-v1->y)*(v2->y-v1->y)+(v2->x-v1->x)*(v2->x-v1->x)));
	if(ret_val < 0)
		return -(ret_val);
	return ret_val;
}
bool is_spaceship_hit(float sp_x,float sp_y,asteroid*as,float theta_in_radians,float scale){
	//[check while debugging!!!]
	//initializes the vertices array
	//by using rotation transformation formula
	
	float xarray[3]={0,scale*0.5,-(scale*0.5)};
	float yarray[3]={-(scale*0.577),scale*0.28867,scale*0.28867};
	for(int i = 0;i<3;i++){

		vertices[i].x=sp_x+(xarray[i])*cos(theta_in_radians) - (yarray[i])*sin(theta_in_radians);
		vertices[i].y=sp_y+(xarray[i])*sin(theta_in_radians) + (yarray[i])*cos(theta_in_radians);
	}

	
	//first checks if the centre of circle makes an acute angle with one of the 3 sides by applying pythagorean formula 
	//then further checks if the distance between the same side and centre of circle is less than or equal to radius
	//of the circle and if that is the case returns true i.e the spaceship is hit!!!	
	for(int i=0;i<3;i++){
		int tmp=i+1;
		for(int j=0;j<3-(i+1);j++){
			//calculating and storing each distance in some variables for convenience
			float side1= calc_distance(vertices[i].x,vertices[i].y,vertices[tmp].x,vertices[tmp].y);
			float side2=calc_distance(vertices[i].x,vertices[i].y,as->x,as->y);
			float side3=calc_distance(vertices[tmp].x,vertices[tmp].y,as->x,as->y);

			//checking the conditions
			if((side2*side2 <= side1*side1 + side3*side3) && (side3*side3 <= side1*side1 + side2*side2)){
				if(distance_from_line(&vertices[i],&vertices[tmp],as) <= as->scale*25)
					return true;
			}
			tmp++;
		}

	}
	//check if the asteroid hit any of the vertex
	for(int i=0;i<3;i++){
		if(calc_distance(vertices[i].x,vertices[i].y,as->x,as->y) <= as->scale*25)
			return true;
	}
	return false;
}	
