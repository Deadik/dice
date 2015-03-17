#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/sleep.h>
#define TRUE 1 
#define FALSE 0 
#define SLEEP_TIMEOUT 500
//Биты состояния
#define STATEDICE 0x01
#define STATEBUTTON 0x02

unsigned char digs[6]={
  0x01,//1
  0x02,//2
  0x03,//3
  0x06,//4
  0x07,//5
  0x0E,//6
};

short dig=0;
short dig2=0;
unsigned int timer = 0;
unsigned int sleepTimer = SLEEP_TIMEOUT;

//Байт для сохранения состояний
char STATE = (0<<STATEDICE)|(0<<STATEBUTTON);

//Бросок кубика
void shuffleDice(void);
//Инициализация портов
void init_io(void);
//Инициализация прерываний
void init_int(void);
//Смена режима
void setMode(void);

void setMode(){
	if((STATE & (1 << STATEDICE))){
		STATE&=~(1<<STATEDICE);
		PORTB=0x0F;
	}else{
		STATE|=(1<<STATEDICE);
		PORTB=0xFF;
	}
}

void init_int(){
	//Устанавливаем прерывани по кнопке
	GIMSK=(1<<INT0);
	MCUCR=(0<<ISC00)|(1<<ISC01);

	//Устанавливаем таймер
	OCR0A  = 0xFF;
	TCCR0A = (1<<WGM01)|(0<<WGM00)|(0<<CS02)|(0<<CS01)|(1<<CS00);
	TIFR |= 0x01;
	TCCR0B = (0<<CS02)|(1<<CS01)|(1<<CS00);// 1/64
	TIMSK = (0<<TOIE0)|(1<<OCIE0A);

	//Разрешаем прерывания
	sei();
}

void init_io(){
	DDRB = 0xFF;
	PORTB=0x0F;
	DDRD = (0 << PD2);
}

void shuffleDice(){
	for (int i = 0; i < 10; ++i)
	{
		dig = rand()%6;
		PORTB =(digs[dig]);
		_delay_ms(50);
	}
	if((STATE & (1 << STATEDICE))){
		for (int i = 0; i < 10; ++i)
		{
			dig2 = rand()%6;
			PORTB =(digs[dig])|(digs[dig2]<<4);
			_delay_ms(50);
		}
	}
}

SIGNAL(INT0_vect) {
	sleepTimer=SLEEP_TIMEOUT;
	cli();
	STATE|=(1<<STATEBUTTON);
	sei();
}

SIGNAL(TIMER0_COMPA_vect) {
	sleepTimer--;

	if((STATE & (1 << STATEBUTTON))){
		timer++;
	}

	if(timer>20&&timer<160){
		if((PIND & (1 << PD2)) !=0){
			STATE&=~(1<<STATEBUTTON);
			timer=0;
			shuffleDice();
		}
	}else if(timer>250){
		if((PIND & (1 << PD2)) == 0){
			setMode();
		}
		STATE&=~(1<<STATEBUTTON);
		timer=0;
	}
}

int main(void)
{
	init_io();
	init_int();
	while(1){}
} 
