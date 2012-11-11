#ifndef __MATRIX_H__
#define __MATRIX_H__
/*
	Matrix Class
	coded by kioku
*/

#include "Vector.h"
#include <math.h>

//3x3 Matrix
class CMatrix3{
	public:
		CVector m[3];//line vector
	
		CMatrix3()
		{
			long i;
			for(i=0;i<3;i++){
				m[i].x=0;
				m[i].y=0;
				m[i].z=0;
			}
		}

		CMatrix3(CVector RowVec[3])
		{
			long i;
			for(i=0;i<3;i++) m[i]=RowVec[i];
		}
		
		CMatrix3 operator+(CMatrix3 obj){
			long i;
			CMatrix3 mat;
			for(i=0;i<3;i++) mat.m[i] = m[i] + obj.m[i];
			return mat;
		}

		CMatrix3 operator-(CMatrix3 obj)
		{
			long i;
			CMatrix3 mat;
			for(i=0;i<3;i++) mat.m[i] = m[i] - obj.m[i];
			return mat;
		}

		CMatrix3 operator*(CMatrix3 obj)
		{
			long i;
			CMatrix3 mat;
			CVector	Column[3];
			Column[0].x=obj.m[0].x; Column[0].y=obj.m[1].x; Column[0].z=obj.m[2].x; 
			Column[1].x=obj.m[0].y; Column[1].y=obj.m[1].y; Column[1].z=obj.m[2].y; 
			Column[2].x=obj.m[0].z; Column[2].y=obj.m[1].z; Column[2].z=obj.m[2].z;
			
			for(i=0;i<3;i++){
				mat.m[i].x = m[i] * Column[0];
				mat.m[i].y = m[i] * Column[1];
				mat.m[i].z = m[i] * Column[2];
			}
			return mat;
		}

		
		CMatrix3 operator*(float d){
			long i;
			CMatrix3 mat;
			for(i=0;i<3;i++) mat.m[i] = m[i] * d;
			return mat;
		}

		CMatrix3 operator/(float d){
			long i;
			CMatrix3 mat;
			for(i=0;i<3;i++) mat.m[i] = m[i] / d;
			return mat;
		}
	
		//2operator
		CMatrix3 operator+=(CMatrix3 obj){
			long i;
			for(i=0;i<3;i++) m[i] += obj.m[i];
			return CMatrix3(m);
		}

		CMatrix3 operator-=(CMatrix3 obj)
		{
			long i;
			for(i=0;i<3;i++) m[i] -= obj.m[i];
			return CMatrix3(m);
		}
		
		CMatrix3 operator*=(CMatrix3 obj)
		{
			long i;
			CVector	Column[3];
			CMatrix3 mat;
			Column[0].x=obj.m[0].x; Column[0].y=obj.m[1].x; Column[0].z=obj.m[2].x; 
			Column[1].x=obj.m[0].y; Column[1].y=obj.m[1].y; Column[1].z=obj.m[2].y; 
			Column[2].x=obj.m[0].z; Column[2].y=obj.m[1].z; Column[2].z=obj.m[2].z;
			
			for(i=0;i<3;i++){
				mat.m[i].x = m[i] * Column[0];
				mat.m[i].y = m[i] * Column[1];
				mat.m[i].z = m[i] * Column[2];
			}

			for(i=0;i<3;i++){
				m[i] = mat.m[i];
				m[i] = mat.m[i];
				m[i] = mat.m[i];
			}
			return mat;
		}

		CMatrix3 operator/=(float d){
			long i;
			for(i=0;i<3;i++) m[i] /= d;
			return CMatrix3(m);
		}


		//Vector product
		CVector operator*(CVector obj){
			CVector vec;
			vec.x = m[0] * obj;
			vec.y = m[1] * obj;
			vec.z = m[2] * obj;
			return vec;
		}
		//Vector product
		CVector operator*=(CVector obj){
			CVector vec;
			vec.x = m[0] * obj;
			vec.y = m[1] * obj;
			vec.z = m[2] * obj;
			obj = vec;
			return obj;
		}

		//Inverse
		CMatrix3 GetInverse()
		{
			long i,j,k;
			float a[3][3], c[3][3];

			//input
			for(i=0; i<3; i++)
				a[0][i]=m[i].x; a[1][i]=m[i].y; a[2][i]=m[i].z; 
			for(i=0; i<3; i++){
				for(j=0; j<3; j++){
					if(a[j][i]==0.0f) a[j][i]=0.00001f;
				}
			}

			// LU
			for(i=0; i<3; i++){
				for(j=i+1; j<3; j++){
					a[j][i] /= a[i][i];
					for( k = i+1; k<3; k++ ){
						a[j][k] -= a[i][k] * a[j][i];
					}
				}
			}
		
			// Culc Inverse Matrix
			for(k=0; k<3; k++){
				// Init
				for(i=0; i<3; i++){
					if( i==k )	c[i][k]=1;
					else		c[i][k]=0;
				}
				// Culc
				for(i=0; i<3; i++){
					for(j=i+1; j<3; j++){
						c[j][k] -= c[i][k] * a[j][i];// U
					}
				}
				for(i=3-1; i>=0; i--){
					for(j=i+1; j<3; j++){
						c[i][k] -= a[i][j] * c[j][k];// L
					}
					c[i][k] /= a[i][i];
				}
			}

			//output
			CMatrix3 mat;
			for(i=0; i<3; i++)
				mat.m[i].x=c[0][i]; mat.m[i].y=c[1][i]; mat.m[i].z=c[2][i]; 

			return mat;
		}


		//diff
		bool operator==(CMatrix3 obj){
			if((m[0] == obj.m[0])&&(m[1] == obj.m[1])&&(m[2] == obj.m[2]))  return true;
			else															return false;
		}
		bool operator!=(CMatrix3 obj){
			if((m[0] == obj.m[0])||(m[1] == obj.m[1])||(m[2] == obj.m[2]))  return false;
			else															return true;
		}


		//option

		//3x3 Rotate Matrix Util
		void SetRotateX (float rad)
		{
			float a_sin, a_cos;//accelaration
			a_sin = sinf(rad);
			a_cos = cosf(rad);
			m[0].x = 1; m[0].y=		0; m[0].z=    0; //|1		0		0	|
			m[1].x = 0; m[1].y= a_cos; m[1].z=-a_sin; //|0	cosƒÆ	 -sinƒÆ	|
			m[2].x = 0; m[2].y= a_sin; m[2].z=a_cos; //|0	sinƒÆ	 cosƒÆ	|
		}
		void SetRotateY (float rad)
		{
			float a_sin, a_cos;//accelaration
			a_sin = sinf(rad);
			a_cos = cosf(rad);
			m[0].x = a_cos; m[0].y=0; m[0].z= a_sin; //|cosƒÆ	0	sinƒÆ	|
			m[1].x = 0;		m[1].y=1; m[1].z=  	  0; //|0		1		 0	|
			m[2].x =-a_sin; m[2].y=0; m[2].z= a_cos; //|-sinƒÆ	0	cosƒÆ	|
		}
		void SetRotateZ (float rad)
		{
			float a_sin, a_cos;//accelaration
			a_sin = sinf(rad);
			a_cos = cosf(rad);	
			m[0].x= a_cos; m[0].y=-a_sin; m[0].z=0;//|cosƒÆ		 -sinƒÆ		0|
			m[1].x= a_sin; m[1].y=a_cos; m[0].z=0; //|sinƒÆ	 cosƒÆ		0|
			m[2].x=		0; m[2].y=	0;	 m[2].z=1; //|	0			0		1|
		}
};


#endif //__MATRIX_H__