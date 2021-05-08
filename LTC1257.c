/*
 * LTC1257.c
 *
 * Created: 12/04/2021 21:27:48
 * Author: Sergiy Sydorenko
 * Email: mail@insanehydraulics.com
 *
 */ 

#include <stdint.h>
#include "IO_Macros.h"
#include "LTC1257.h"


void initiate_dac(void){
	PinMode(DIN, Output);
	PinMode(SCL, Output);
	PinMode(LOAD, Output);
	DigitalWrite(DIN, Low);
	DigitalWrite(SCL, Low);
	DigitalWrite(LOAD, High);	
}

void dac_set(uint16_t dac_setting){
	//clear the 4 MSBs of the dac_setting, just in case
	dac_setting &= 4095;
	int8_t i;
	for(i = 11; i >= 0; i--){
		DigitalWrite(SCL, Low);
		//Load the MSB into the data pin
		if(dac_setting & (1<<(i))){
			DigitalWrite(DIN, High);
		} else {
			DigitalWrite(DIN, Low);
		}
		DigitalWrite(SCL, High);
	}
	DigitalWrite(SCL, Low);
	DigitalWrite(LOAD, Low);
	DigitalWrite(LOAD, High);
}
