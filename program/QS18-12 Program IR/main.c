#include <reg52.H>
#include <stdlib.h>
#include <intrins.h>

typedef	unsigned char uint8_t;
typedef	unsigned int uint16_t;

/********************************/
/*		Real Time Clock	Pin		*/
/********************************/
sbit	ds1302_sck	=	P2^0;
sbit	ds1302_sdin	=	P2^1;
sbit	ds1302_cs	=	P2^2;
sbit	hx1838_data	=	P0^0;
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
uint8_t code time_max_val[3] = {0x24,0x60,0x60};
uint8_t code date_max_val[2] = {0x32,0x13};

uint8_t	code remote_data[10]  = {0x16,0x0C,0x18,0x5e,0x08,0x1C,0x5A,0x42,0x52,0x4A};
uint8_t	code shift_num[10]  = {8,5,1,0,2,6,7,3,9,4};
uint8_t	code shift_week[7]  = {5,3,7,6,2,0,1};

uint8_t data clock_data[7]  = {0x30,0x36,0x01,0x20,0x06,0x01,0x14};

static uint8_t data timer_count;
static uint8_t data nixie_roll = 200;
uint8_t IRCOM[4] = {0};
bit	update_flag = 0;
bit	roll_nixie_flag = 0;

/********************************/
/*			All Functions		*/
/********************************/
void ds1302_ini(void);
void ds1302_setup(uint8_t *clock_ptr);
uint8_t ds1302_read(uint8_t cmd);
void ds1302_write(uint8_t cmd, uint8_t data_in);
void ds1302_wr(uint8_t cmd);
void update_nixie_clock(void);
void IR_transmit(void);
void nixie_setup(void);
void roll_nixie_fun(void);

void delay(uint8_t x){
	
	uint8_t i;
	while(x--){for(i = 0; i<13; i++);}
}

void timer0_isr() interrupt 1{
	
	TR0 = 0; TF0 = 0;

	timer_count++;
	
	if(timer_count > 19){
		timer_count = 0;
		nixie_roll++;
		
		if(nixie_roll > 239){
			nixie_roll = 0;
			roll_nixie_flag = 1;
		}
		
		else if(nixie_roll < 240){
			update_flag = 1;
		}
	}
	
	if(timer_count == 10){cd4017_dot = 1;}
	else if(!timer_count){cd4017_dot = 0;}
	
	TL0 = 0xB0;
	TH0 = 0x3C;
	TR0 = 1;
}

void main(void){
	
	hx1838_data = 1;
	ds1302_cs = 0;ds1302_sck = 0;
	cd4017_rst = 0;cd4017_rst = 1;cd4017_rst = 0;
	
	ds1302_ini();
	if(ds1302_read(0xC1) != 0xAA){
		ds1302_setup(clock_data);
	}
	
	TMOD = 0x01;
	ET0 = 1;
	TL0 = 0xB0;
	TH0 = 0x3C;
	TR0 = 1;
	EA = 1;

	while(1){
		
		if(update_flag){
			update_nixie_clock();
			update_flag = 0;
		}
		
		if(roll_nixie_flag){
			roll_nixie_fun();
			update_nixie_clock();
			roll_nixie_flag = 0;
		}
		
		hx1838_data = 1;
		
		if(!hx1838_data){
			EA = 0;
			IR_transmit();
			
			if(IRCOM[2] == 0x46){
				IRCOM[2] = 0x00;
				nixie_setup();
			}
			
			EA = 1;
		}
	}
}

void nixie_setup(){
	
	uint8_t time_temp[6] = {0};
	uint8_t date_temp[6] = {0};
	uint8_t week_date = 0x00;
	
	uint8_t i, update_pos = 0;
	
	uint8_t *ptr_temp;
	uint8_t *ptr = time_temp;
	
	cd4017_dot = 0;
	
	cd4017_rst = 0;	cd4017_rst = 1;cd4017_rst = 0;					//	set all digit to zero
	for(i = 0; i < 8; i++){P1 = 0x00;P1 = 0x7E;P1 = 0x00;}
	
	while(1){
		
redo_label:
		
		if(!hx1838_data){IR_transmit();}
		
		if(IRCOM[2] == 0x09){	//	EQ = Enter
			
			IRCOM[2] = 0x00;
			
			if(update_pos == 0){
				for(i = 0; i < 3; i++){
					if((time_temp[2*i]<<4|time_temp[2*i+1]) >= time_max_val[i]){
						ptr = time_temp;
						goto redo_label;
					}
				}
				/*update_pos=1;
				ptr = &date_temp[0];
				cd4017_dot = 1;
				goto renew_val;
			}
			
			if(update_pos == 1){
				for(i = 0; i < 2; i++){
					if(	(date_temp[2*i]<<4|date_temp[2*i+1]) >= date_max_val[i] || 
						(date_temp[2*i]<<4|date_temp[2*i+1]) < 0x01){
						ptr = date_temp;
						goto redo_label;
					}
				}
				update_pos++;
			}
			
			if(update_pos >= 2){*/
				ds1302_write(0x8E,0x00);
				ds1302_write(0x90,0x50);
				ds1302_write(0x80,(time_temp[4]<<4|time_temp[5]));
				ds1302_write(0x82,(time_temp[2]<<4|time_temp[3]));
				ds1302_write(0x84,(time_temp[0]<<4|time_temp[1]));
				//ds1302_write(0x86,(date_temp[0]<<4|date_temp[1]));
				//ds1302_write(0x88,(date_temp[2]<<4|date_temp[3]));
				ds1302_write(0x8A,week_date);
				//ds1302_write(0x8C,(date_temp[4]<<4|date_temp[5]));
				break;
			}
		}
		
		else if(IRCOM[2] == 0x15){
			week_date++;
			if(week_date > 0x07){week_date = 0x01;}
			goto renew_val;
		}
		
		else if(IRCOM[2] == 0x07){
			if(week_date == 0x01){week_date = 0x08;}
			week_date--;
			goto renew_val;
		}
		
		for(i = 0; i < 10; i++){
			if(IRCOM[2] == remote_data[i]){
				*ptr = i;
				ptr++;
				goto renew_val;
			}
		}
		
		goto redo_label;
				
renew_val:
		
		ptr_temp = date_temp;
		if(update_pos == 0){ptr_temp = time_temp;}
		
		cd4017_rst = 0;	cd4017_rst = 1;cd4017_rst = 0;
		
		for(i = 0; i < 10; i++){
			if(shift_num[*ptr_temp] > i){cd4017_h0 = 0;cd4017_h0 = 1;cd4017_h0 = 0;}
				ptr_temp++;
			if(shift_num[*ptr_temp] > i){cd4017_h1 = 0;cd4017_h1 = 1;cd4017_h1 = 0;}
				ptr_temp++;
			if(shift_num[*ptr_temp] > i){cd4017_m0 = 0;cd4017_m0 = 1;cd4017_m0 = 0;}
				ptr_temp++;
			if(shift_num[*ptr_temp] > i){cd4017_m1 = 0;cd4017_m1 = 1;cd4017_m1 = 0;}
				ptr_temp++;
			if(shift_num[*ptr_temp] > i){cd4017_s0 = 0;cd4017_s0 = 1;cd4017_s0 = 0;}
				ptr_temp++;
			if(shift_num[*ptr_temp] > i){cd4017_s1 = 0;cd4017_s1 = 1;cd4017_s1 = 0;}
			
			if(shift_week[(week_date-1)] > i){cd4017_d0 = 0;cd4017_d0 = 1;cd4017_d0 = 0;}
			
			ptr_temp = date_temp;
			if(update_pos == 0){ptr_temp = time_temp;}
		}
		
		IRCOM[2] = 0x00;
	}
}

void IR_transmit(){
	
	uint8_t j, k, N = 0;
	IRCOM[2] = 0x00;
	
	delay(15);
	
	if(hx1838_data){return;}
	while (!hx1838_data){delay(1);}

	for (j=0;j<4;j++){
		for (k=0;k<8;k++){
			while(hx1838_data){delay(1);}
			while(!hx1838_data){delay(1);}
			while(hx1838_data){delay(1); N++; if(N>=30){IRCOM[2] = 0x00;return;}}

			IRCOM[j]=IRCOM[j] >> 1;
			if (N>=8){IRCOM[j] = IRCOM[j] | 0x80;}
			N=0;
		}
	}

	j = ~IRCOM[3];

	if(IRCOM[2]!= j){IRCOM[2] = 0x00;return;}
}

void roll_nixie_fun(){
	
	uint8_t i;
	
	cd4017_rst = 0;cd4017_rst = 1;cd4017_rst = 0;
	
	for(i = 0; i < 100; i++){
		delay(250);delay(250);delay(250);
		P1 = 0x00;
		P1 = 0x7E;
		P1 = 0x00;
	}
}

void update_nixie_clock(){
	
	uint8_t i;
	
	for(i = 0; i < 7; i++)
		clock_data[i] = ds1302_read(0x81+(i*2));
		
	cd4017_rst = 0;	cd4017_rst = 1;cd4017_rst = 0;
		
	for(i = 0; i < 10; i++){
		if(shift_num[(clock_data[2]/16)] > i){cd4017_h0 = 0;cd4017_h0 = 1;cd4017_h0 = 0;}
		if(shift_num[(clock_data[2]%16)] > i){cd4017_h1 = 0;cd4017_h1 = 1;cd4017_h1 = 0;}
		if(shift_num[(clock_data[1]/16)] > i){cd4017_m0 = 0;cd4017_m0 = 1;cd4017_m0 = 0;}
		if(shift_num[(clock_data[1]%16)] > i){cd4017_m1 = 0;cd4017_m1 = 1;cd4017_m1 = 0;}
		if(shift_num[(clock_data[0]/16)] > i){cd4017_s0 = 0;cd4017_s0 = 1;cd4017_s0 = 0;}
		if(shift_num[(clock_data[0]%16)] > i){cd4017_s1 = 0;cd4017_s1 = 1;cd4017_s1 = 0;}
		if(shift_week[(clock_data[5]-1)] > i){cd4017_d0 = 0;cd4017_d0 = 1;cd4017_d0 = 0;}
	}
}

void ds1302_ini(){
	
	ds1302_write(0x8E,0x00);
	ds1302_write(0x90,0x50);
}

void ds1302_setup(uint8_t *clock_ptr){
	
	uint8_t i;
	
	ds1302_write(0x8E,0x00);
	ds1302_write(0x90,0x50);
	for(i = 0; i < 8; i++){
		ds1302_write(0x80+(i*2),clock_ptr[i]);
	}
	ds1302_write(0xC0,0xAA);
}

void ds1302_write(uint8_t cmd, uint8_t data_in){
	
	ds1302_cs = 1;
	ds1302_wr(cmd);
	ds1302_wr(data_in);
	ds1302_cs = 0;
}

void ds1302_wr(uint8_t cmd){
	
	uint8_t i,buff;
	buff = cmd;
	
	for(i = 0; i < 8; i++){
		ds1302_sdin = 0;
		if(buff&0x01){ds1302_sdin = 1;};
		ds1302_sck = 1;_nop_();
		ds1302_sck = 0;_nop_();
		buff>>=1;
	}
}

uint8_t ds1302_read(uint8_t cmd){
	
	uint8_t i, buff;

	ds1302_cs = 1;
	ds1302_wr(cmd);
	
	for(i = 0; i < 8; i++){
		buff>>=1;
		if(ds1302_sdin){buff |= 0x80;};
		ds1302_sck = 1;_nop_();
		ds1302_sck = 0;_nop_();
	}
	
	ds1302_cs = 0;
	return(buff);
}
