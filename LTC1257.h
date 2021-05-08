/*
 * LTC1257.h
 *
 * Created: 12/04/2021 21:08:56
 * Author: Sergiy Sydorenko
 * Email: mail@insanehydraulics.com
 * Description: Bit banging library for controlling the LTC1257 12 bit DAC with an 8 bit AVR micro controller.
 * The DAC needs three pins - DIN (Data IN), SCL (Serial Clock), and LOAD (Data Load). 
 * Data is latched into the shift register on the rising edge of SCL, MSB first.
 * DAC is updated when LOAD is pulled low, and latched on rising edge of LOAD.
 */ 


#ifndef LTC1257_H_
#define LTC1257_H_

//define ports and pins here
#define DIN B, 5 
#define SCL B, 4
#define LOAD D, 3

//initiate pins of the mcu
void initiate_dac(void);

//send data to the DAC, max number can only be 4096, because the LTC1257 has 12 bit resolution
void dac_set(uint16_t dac_setting);

#endif /* LTC1257_H_ */