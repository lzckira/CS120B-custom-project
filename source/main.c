  
/*	Author: zlian030
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Custom Project
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/eeprom.h>	
#include "header/scheduler.h"
#include "source/io.c"
#include "header/io.h"
#include "header/timer.h"

unsigned char upPattern[] = {3,8,9,14,18,19,26,31,33,37};
unsigned char downPattern[] = {1,6,11,12,16,23,24,28,35,41};
unsigned char upLine[] = {16,16,16,16,16,16,16,16,16,16};
unsigned char downLine[] = {32,32,32,32,32,32,32,32,32,32};
unsigned char i = 0;
unsigned char countUp = 0;
unsigned char countDown = 0;
unsigned char firstUp = 1;
unsigned char firstDown = 1;
unsigned char numbers = 1;
unsigned char upFlag = 0;
unsigned char downFlag = 0;
unsigned char startMess[] = "Press start     High score";
unsigned char loseMess[] = "GAME OVER       You score is ";
unsigned char score = 0;
unsigned char winMess[] = "Goal!!!!!       You score is 100";
unsigned char startMess2[] = "Press start     High score 100";
unsigned char gameStatus = 0;
unsigned char charPos = 17;
unsigned char k = 0;


void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

char downBlades[8] = { 0x15, 0x0E, 0x15, 0x0E, 0x04, 0x15, 0x0E, 0x04 };
char upBlades[8] = { 0x04, 0x0E, 0x15, 0x04, 0x0E, 0x15, 0x0E, 0x15 };
char rabbit[8] = { 0x08, 0x04, 0x1E, 0x05, 0x0F, 0x1E, 0x1E, 0x0D };
enum mapSet{start}map_state;
	
int mapDisp(int state){
	if(gameStatus == 1 && downLine[9] != 16) {
		switch(state){
		case start:
			LCD_ClearScreen();
			if(numbers == upPattern[countUp]){
				countUp++;	
			}
			else if(numbers == downPattern[countDown]){
				countDown++;
			}
			numbers++;
			i = firstUp;
			upFlag = 0;
			if (firstUp <= 10) {
				if (upLine[firstUp - 1] == 1) {
					upFlag = 1;
				}
			}
			while(i <= countUp){
				if (upLine[i - 1] > 0) {
					LCD_Cursor(upLine[i - 1]);
					LCD_WriteData(0);
					upLine[i - 1]--;
				}
				i++;
			}
			i = firstDown;
			downFlag = 0;
			if (firstDown <= 10) {
				if (downLine[firstDown - 1] == 17) {
					downFlag = 1;
				}
			}
			while(i <= countDown){
				if (downLine[i - 1] > 16){
					LCD_Cursor(downLine[i - 1]);
					LCD_WriteData(1);
					downLine[i - 1]--;
				}
				i++;
			}
			LCD_Cursor(0);
			break;
		}
	}
	else {}
	return state;
}

enum cusChar{up,down};

int charDisp(int state){
	if(gameStatus == 1) {
		switch(state) {
			case up:
				if (ADC >= 950) {
					state = down;
					LCD_Cursor(17);
					LCD_WriteData(2);
					LCD_Cursor(0);
					charPos = 17;
					if (downFlag == 1) { gameStatus = 2; break; }
				}
				else {
					LCD_Cursor(1);
					LCD_WriteData(2);
					LCD_Cursor(0);
					if (upFlag == 1) { gameStatus = 2; break; }
				}
				break;	
			case down:
				if (ADC <= 50) {
					state = up;
					LCD_Cursor(1);
					LCD_WriteData(2);
					LCD_Cursor(0);
					charPos = 1;
					if (upFlag == 1) { gameStatus = 2; break; }
				}
				else {
					LCD_Cursor(17);
					LCD_WriteData(2);
					LCD_Cursor(0);
					if (downFlag == 1) { gameStatus = 2; break; }
				}
				break;		
		}
	}
	else {}
	return state;
}

enum theMenu{reset, waitStart, win, lose, playing};

int theMenu(int state){
	switch(state) {
		case reset:
			if (gameStatus == 5) {
				score = 0;
				countUp = 0;
				countDown = 0;
				firstUp = 1;
				firstDown = 1;
				numbers = 1;
				upFlag = 0;
				downFlag = 0;
				for (i = 0; i < 10; i++) {
					upLine[i] = 16;
					downLine[i] = 32;
				}
			}
			if (PINA == 0xFB) { 
				k++;
				if (k >= 15) {
					eeprom_write_byte((uint8_t*)1, 0);
				} 
			}
			i = 0;
			if (PINA == 0xFF) {
				state = playing;
				gameStatus = 1;
			}
		break;
		
		case waitStart:
			if (PINA == 0xFB) {
				state = playing;
				gameStatus = 1;
			}
		break;
		
		case win:
			if (gameStatus == 3) {
				LCD_ClearScreen();
				LCD_DisplayString(1, winMess);
				LCD_Cursor(0);
				score = 20;
				for (i = 0; i < 10; i++) {
					downLine[i] = 32;
				}
				eeprom_write_byte((uint8_t*)1, 20);
			}
			gameStatus = 0;
			if (PINA == 0xFB) {
				state = reset;
				gameStatus = 5;
			}
		break;
		
		case lose:
			if (gameStatus == 2) {
				score = 0;
				for (i = 0; i < 10; i++) {
					if (upLine[i] == 0) { score++; }
					if (downLine[i] == 16) { score++; }
				}
				LCD_ClearScreen();
				LCD_DisplayString(1, loseMess);
				LCD_Cursor(30);
				LCD_WriteData(score / 2 + '0');
				LCD_Cursor(31);
				LCD_WriteData((score * 5) % 10 + '0');
				LCD_Cursor(0);
				if (score > eeprom_read_byte((uint8_t*)1)) { 
					eeprom_write_byte((uint8_t*)1,score); 
				};
			}
			gameStatus = 0;
			if (PINA == 0xFB) {
				state = reset;
				gameStatus = 5;
			}
		break;

		case playing:
			k = 0;
			if (gameStatus == 2) { state = lose; }
			else if(downLine[9] == 16 && gameStatus == 1) {
			    state = win;
				gameStatus = 3;
			}
			if (upLine[firstUp - 1] == 0) {
				firstUp++;
			}
			if (downLine[firstDown - 1] == 16) {
				firstDown++;
			}
			if (PINA == 0xFB) { 
				state = reset; 
				gameStatus = 5; 
			}
			else { }
		break;
	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
	
	LCD_CustomChar(0, downBlades);
	LCD_CustomChar(1, upBlades);
	LCD_CustomChar(2, rabbit);
    static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
    //Task 1 (theMenu)
    task1.state = waitStart;//Task initial state
    task1.period = 300;//Task Period
    task1.elapsedTime = task1.period;//Task current elapsed time.
    task1.TickFct = &theMenu;//Function pointer for the tick
    //Task 2 (mapDisp)
    task2.state = start;//Task initial state
    task2.period = 500;//Task Period
    task2.elapsedTime = task2.period;//Task current elapsed time.
    task2.TickFct = &mapDisp;//Function pointer for the tick
    //Task 3 (charDisp)
    task3.state = down;//Task initial state
    task3.period = 100;//Task Period
    task3.elapsedTime = task3.period;//Task current elapsed time.
    task3.TickFct = &charDisp;//Function pointer for the tick
    unsigned long GCD = tasks[0]->period;
    unsigned short i;//Scheduler for-loop iterator
    
	for ( i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}
	
    TimerSet(GCD);
    TimerOn();
    LCD_init();
	score = eeprom_read_byte((uint8_t*)1);
	if (score != 20) {
		LCD_DisplayString(1, startMess);
		LCD_Cursor(29);
		LCD_WriteData(score / 2 + '0');
		LCD_Cursor(30);
		LCD_WriteData(score * 5 % 10 + '0');
	}
	else {
		LCD_DisplayString(1, startMess2);
	}
		LCD_Cursor(0);
    while (1) {
		ADC_init();
		for (i = 0; i < numTasks; i++) { //Scheduler code
			if (tasks[i]->elapsedTime == tasks[i]->period) { //Task is ready to tick
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); //set next state
				tasks[i]->elapsedTime = 0; //Reset the elapsed time for next tick;
			}
			tasks[i]->elapsedTime += GCD; 		
		}
		while(!TimerFlag) {};
		TimerFlag = 0;
    }
    return 1;
}
