#include <reg52.h>
#include <stdlib.h>
#include <stdio.h>
#include <intrins.h>

typedef	unsigned char uint8_t;
typedef	unsigned int uint16_t;

/********************************/
/*		Real Time Clock	Pin		*/
/********************************/

/********************************/
/*		All Shift Data	Pin		*/
/********************************/
sbit	cd4017_rst	=	P1^7;
sbit	cd4017_h0	=	P1^1;
sbit	cd4017_h1	=	P1^2;
sbit	cd4017_m0	=	P1^3;
sbit	cd4017_m1	=	P1^4;
sbit	cd4017_s0	=	P1^5;
sbit	cd4017_s1	=	P1^6;
sbit	cd4017_d0	=	P3^2;
sbit	cd4017_dot	=	P3^3;
/********************************/
/*		Global Variable			*/
/********************************/

void main(void){
	
	uint8_t i, j, k;
	
	P1 = 0x00;
	cd4017_rst = 1;cd4017_rst = 0;

	while(1){
		
		cd4017_h0 = 0;cd4017_h0 = 1;cd4017_h0 = 0;
		cd4017_h1 = 0;cd4017_h1 = 1;cd4017_h1 = 0;
		cd4017_m0 = 0;cd4017_m0 = 1;cd4017_m0 = 0;
		cd4017_m1 = 0;cd4017_m1 = 1;cd4017_m1 = 0;
		cd4017_s0 = 0;cd4017_s0 = 1;cd4017_s0 = 0;
		cd4017_s1 = 0;cd4017_s1 = 1;cd4017_s1 = 0;
		cd4017_d0 = 0;cd4017_d0 = 1;cd4017_d0 = 0;

		for(i = 0;i < 15;i++){
			for(j = 0;j < 135;j++){
				for(k = 0;k < 250;k++);
			}
		}
	}
}
