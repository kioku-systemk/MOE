//Vector class  coded by KIOKU
#pragma once
#ifndef __KVECTOR__
#define __KVECTOR__

#include "kmath.h"

class CVector{
	public:
		float x;
		float y;
		float z;
		CVector(){x=0; y=0; z=0; }
		CVector(float jx, float jy, float jz){
			x = jx;
			y = jy;
			z = jz;
		}
		CVector operator+(CVector obj){
			CVector vec;
			vec.x = x + obj.x;
			vec.y = y + obj.y;
			vec.z = z + obj.z;
			return vec;
		}
		CVector operator-(CVector obj){
			CVector vec;
			vec.x = x - obj.x;
			vec.y = y - obj.y;
			vec.z = z - obj.z;
			return vec;
		}
		CVector operator*(float d){
			CVector vec;
			vec.x = x * d;
			vec.y = y * d;
			vec.z = z * d;
			return vec;
		}
		CVector operator/(float d){
			CVector vec;
			vec.x = (x / d);
			vec.y = (y / d);
			vec.z = (z / d);
			return vec;
		}
		
		CVector operator+=(CVector obj){
			x += obj.x;
			y += obj.y;
			z += obj.z;
			return CVector(x,y,z);
		}
		CVector operator-=(CVector obj){
			x -= obj.x;
			y -= obj.y;
			z -= obj.z;
			return CVector(x,y,z);
		}
		CVector operator*=(float d){
			x *= d;
			y *= d;
			z *= d;
			return CVector(x,y,z);
		}
		CVector operator/=(float d){
			x /= d;
			y /= d;
			z /= d;
			return CVector(x,y,z);
		}

		float operator*(CVector obj){//dot product
			return x*obj.x + y*obj.y + z*obj.z;
		}
		CVector normalize(){
			float a = absolute();
			x/=a; y/=a; z/=a;
			return CVector(x,y,z);
		}
		float absolute(){
			return (float)sqr(x*x + y*y + z*z);
		}
		CVector outer(CVector obj){//outer product
			CVector n;
			n.x = y*obj.z - z*obj.y;
			n.y = z*obj.x - x*obj.z;
			n.z = x*obj.y - y*obj.x;
			return n;
		}

		bool operator==(CVector obj){
			if((x == obj.x)&&(y == obj.y)&&(z == obj.z)) return true;
			else return false;
		}
		bool operator!=(CVector obj){
			if((x == obj.x)||(y == obj.y)||(z == obj.z)) return false;
			else return true;
		}

		CVector Min(CVector vec1, CVector vec2){
			if(vec1.x>vec2.x) vec1.x=vec2.x;
			if(vec1.y>vec2.y) vec1.y=vec2.y;
			if(vec1.z>vec2.z) vec1.z=vec2.z;
			return vec1;
		}
		CVector Max(CVector vec1, CVector vec2){
			if(vec1.x<vec2.x) vec1.x=vec2.x;
			if(vec1.y<vec2.y) vec1.y=vec2.y;
			if(vec1.z<vec2.z) vec1.z=vec2.z;
			return vec1;
		}
		CVector Min(CVector vec1, CVector vec2, CVector vec3){
			if(vec1.x>vec2.x) vec1.x=vec2.x;
			if(vec1.y>vec2.y) vec1.y=vec2.y;
			if(vec1.z>vec2.z) vec1.z=vec2.z;
			if(vec1.x>vec3.x) vec1.x=vec3.x;
			if(vec1.y>vec3.y) vec1.y=vec3.y;
			if(vec1.z>vec3.z) vec1.z=vec3.z;
			return vec1;
		}
		CVector Max(CVector vec1, CVector vec2, CVector vec3){
			if(vec1.x<vec2.x) vec1.x=vec2.x;
			if(vec1.y<vec2.y) vec1.y=vec2.y;
			if(vec1.z<vec2.z) vec1.z=vec2.z;
			if(vec1.x<vec3.x) vec1.x=vec3.x;
			if(vec1.y<vec3.y) vec1.y=vec3.y;
			if(vec1.z<vec3.z) vec1.z=vec3.z;
			return vec1;
		}

};
#endif