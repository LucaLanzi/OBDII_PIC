#include <xc.h>
#include "PIC18F46K22-Config.h"
#include "LCD.h"

#define _XTAL_FREQ 64000000 //defines the internal frequency to 64Mhz (Configurable Pg. 30)


void welcomeSplash(void){
    
    //Splash Screen for when the OBDIIPIC turns on
    LCD_cursor_set(1,1);   
    LCD_write_string(">>> OBDIIPIC <<<");
    LCD_cursor_set(2,1);
    LCD_write_string("***>> V1.0 <<***");

}





void main(void) {
    //intitialize the PIC

    OSCCON = 0b01110000;   //16Mhz Internal RC Oscillator Frequency (Page 30. PIC Datasheet)
    OSCTUNEbits.PLLEN = 1; //Enable PLL; FOSC 64Mhz


    LCD_init(); //initialize the LCD
    LCD_clear();

    while(1){
    welcomeSplash();
    }
    
    return;
}
