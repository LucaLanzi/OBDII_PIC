#include <xc.h>
#include "PIC18F46K22-Config.h"
#include "LCD.h"

#define _XTAL_FREQ 64000000 //defines the internal frequency to 16Mhz (Configurable Pg. 30)

char message[] = " Hello from PIC18F46K22!   ";  // Add spaces for smooth wrap
#define LCD_WIDTH 16
int startIndex = 0;


void welcomeSplash(void){
    //Splash Screen for when the OBDIIPIC turns on
    
    LCD_cursor_set(1,1);   
    LCD_write_string("*** OBDIIPIC ***");
    LCD_cursor_set(2,1);
    LCD_write_string("ECE3301L Project");

    
    
}


void main(void) {
    //intitialize the PIC
    OSCCON = 0b01110000; //16Mhz FOSC
    OSCTUNEbits.PLLEN = 1; //Enable PLL; FOSC 64Mhz
    LCD_init(); //initialize the LCD
    LCD_clear();
    
    return;
}
