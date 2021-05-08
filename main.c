/*
 * Parker Flow meter Board.c
 *
 * Created: 25/04/2021 
 * Author : Sergiy Sydorenko
 * Email: mail@insanehydraulics.com
 * more details about this project at:
 * https://www.insanehydraulics.com/backengthis/flowmeterrecovery1.html
 * This code was tested on ATtiny4313, with AVR/GNU C compiler and -O3 (optimize most) optimization
 *
 */ 

//The frequency of the Parker's board oscillator
#define F_CPU 11059000UL

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "IO_Macros.h"
#include "LTC1257.h"

//port that works with INT0
#define PULSE_INPUT D, 2

//11059000 / 65535 = 168.75 Hz, period of approx. 0.006 sec. 
#define REFRESH_AT_OVERFLOWS 60 // 0.35 sec
#define REFRESH_AT_MAX_OVERFLOWS 140 // 0.8 sec

//To simulate decimal places in integer Hz calculations
#define PRECISION_FACTOR 100

//Variables for the counters
volatile uint16_t pulses = 0;
volatile uint16_t timer0_overflows = 0;
volatile uint16_t timer0_overflows_snapshot = 0;
volatile uint8_t timer0_snapshot = 0;
volatile uint8_t timer1_overflows = 0;

/*
//Constants for interpolation
//Do not forget to update this number if you update the interpolation table!
static uint8_t HOW_MANY_POINTS = 9;
//Hertz times PRECISION_FACTOR, in ascending order
static uint32_t HERTZ_PTS[] = {0, 299, 300, 920, 3390, 7400, 11900, 79333, 84357};
//Respective flow values - i.e. the integer value sent to the 12 bit LTC1257 DAC (2.048V full scale)
static uint16_t FLOW_PTS[]  = {0,   0,  20,  64,  192,  384,  576,   3846,  4095};
*/

//Experimental constant values to match the Parker's line
static uint8_t HOW_MANY_POINTS = 12;
//Hertz times PRECISION_FACTOR, in ascending order
static uint32_t HERTZ_PTS[] = {0, 299, 300, 1018, 2303, 3684, 4994, 6410, 10400, 11780, 78533, 83507};
//Respective flow values - i.e. the integer value sent to the 12 bit LTC1257 DAC (2.048V full scale)
static uint16_t FLOW_PTS[]  = {0,   0,  20,   64,  128,  192,  256,  320,   512,   576,  3846,  4095};


void enableTimer0Interrupt(void){
	BitSet(TIMSK, TOIE0);
}

void enableTimer1Interrupt(void){
	BitSet(TIMSK, TOIE1);
}

inline void resetTimer0(void){
	TCNT0 = 0;
	timer0_overflows = 0;
}

inline void resetTimer1(void){
	TCNT1 = 0;
	timer1_overflows = 0;
}

inline void startTimer0(void){
	BitSet(TCCR0B, CS00);
}

inline void stopTimer0(void){
	BitClear(TCCR0B, CS00);
}

inline void startTimer1(void){
	BitSet(TCCR1B, CS10);
}

inline void stopTimer1(void){
	BitClear(TCCR1B, CS10);
}

void setupPulseInput(void){
	PinMode(PULSE_INPUT, Input);
	DigitalWrite(PULSE_INPUT, Low);
	//MCU Control register, Interrupt Sense Control (The rising edge of INT0 generates an interrupt request)
	BitSet(MCUCR, ISC01);
	BitSet(MCUCR, ISC00);
}

void enablePulseInterrupt(void){
	//General Interrupt Mask Register
	BitSet(GIMSK, INT0);
}

void disablePulseInterrupt(void){
	BitClear(GIMSK, INT0);
}

uint32_t calculateFrequency(){
	if (pulses <= 1) return 0;
	return (F_CPU * PRECISION_FACTOR) / (((timer0_overflows_snapshot * 256UL) + timer0_snapshot) / (pulses - 1));
}

int16_t interpolateFlow(uint32_t frequency){
	uint8_t segment;
	//check input bounds and saturate if out of bounds
	if (frequency >= HERTZ_PTS[HOW_MANY_POINTS - 1]){
		return FLOW_PTS[HOW_MANY_POINTS - 1];
		} else if (frequency <= HERTZ_PTS[0]){
		return FLOW_PTS[0];
	}
	//find segment that holds frequency
	for (segment = 0; segment < (HOW_MANY_POINTS - 1); segment++){
		if ((HERTZ_PTS[segment] <= frequency) && (HERTZ_PTS[segment + 1] >= frequency)){
			//found the correct segment
			uint32_t t;
			if (frequency <= HERTZ_PTS[segment]){
				return FLOW_PTS[segment];
			}
			if (frequency >= HERTZ_PTS[segment + 1]){
				return FLOW_PTS[segment + 1];
			}
			t = (frequency - HERTZ_PTS[segment]);
			return FLOW_PTS[segment] + (t * (FLOW_PTS[segment + 1] - FLOW_PTS[segment])) / (HERTZ_PTS[segment + 1] - HERTZ_PTS[segment]);
		}
	}
	//something went wrong, return error value or do something else, I'll return 0
	return 0;
}

ISR(INT0_vect){
	timer0_snapshot = TCNT0;
	timer0_overflows_snapshot = timer0_overflows;
	pulses++;
	if (pulses == 1){
		startTimer0();
	}
}

ISR(TIMER0_OVF_vect){
	timer0_overflows++;
}

ISR(TIMER1_OVF_vect){
	timer1_overflows++;
}

int main(void)
{
	uint32_t last_frequency = 0;
	int16_t overflows = 0;
    initiate_dac();
	setupPulseInput();
	enableTimer0Interrupt();
	enableTimer1Interrupt();
	enablePulseInterrupt();
	sei();
	resetTimer0();
	resetTimer1();
	startTimer1();
	
    while (1) 
    {
		//above 50 Hz refresh period becomes 0.35 sec, at 10 Hz it is about 0.7 sec) 
		overflows = REFRESH_AT_MAX_OVERFLOWS - last_frequency / 63;
		if (overflows < REFRESH_AT_OVERFLOWS) {
			overflows = REFRESH_AT_OVERFLOWS;
		}  
		if (timer1_overflows > overflows) {
			cli();
			stopTimer0();
			stopTimer1();
			last_frequency = calculateFrequency();
			dac_set(interpolateFlow(last_frequency));
			pulses = 0;
			sei();
			resetTimer0();
			resetTimer1();
			startTimer1();
		}
    }
}
