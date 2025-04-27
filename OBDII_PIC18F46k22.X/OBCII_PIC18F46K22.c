#include <xc.h>
#include <stdio.h>
#include "PIC18F46K22-Config.h"
#include "LCD.h"

#define _XTAL_FREQ 16000000 //defines the internal frequency to 16Mhz (Configurable Pg. 30)

//UART Variable Config
#define BUFFER_SIZE 32 //Set the total byte size of the UART buffer
char buffer[BUFFER_SIZE]; //Define a character array to hold the incoming bytes from Rx
unsigned char buffer_count = 0; //a counter for the array index
char RX_char; //variable to store the byte read from the RCREG
volatile char message_complete = 0; // flag for completing UART message


//UART Function Prototypes
void UART1_Init(void); //initialize UART1
void UART1_SendString(char string[]); //Function to send string to ELM327
void UART1_SendChar(char c); //Function to break down the string and send one character at a time to the ELM327

//Read RPM Function Prototypes
void print_RPM(void);
void print_Vbatt(void);
void print_AI_Temp(void);
unsigned char hex_char_to_value(char c);

void main(void){

    OSCCON = 0b01110000; //16Mhz FOSC
    

    LCD_init(); //initialize the LCD
    LCD_clear(); //clear the LCD
    UART1_Init(); //initialize UART1

    __delay_ms(2000);
    UART1_SendString("ATE0\r"); //Turn off Echo from ELM327
    __delay_ms(500);
    LCD_clear();

    while(1){
            print_RPM(); //Print RPM Values
            print_Vbatt(); //Print Vbatt;
        

        //other program stuff here
    }
}

// ########  Beginning of UART CONFIGURATION AND SETUP BLOCK ######
void UART1_Init(void) {
    ANSELC = 0; //ensure all outputs are digital on PORTC
    
    //Configure I/O
    TRISCbits.TRISC6 = 0;  //Sets the Tx output
    TRISCbits.TRISC7 = 1;  //Set the Rx input


    //Configure EUSART1 Transmit Bits
    TXSTA1bits.TXEN = 1; //Enable the UART Transmit
    TXSTA1bits.TX9 = 0; //Enable 8-bit transmission mode
    TXSTA1bits.SYNC = 0; //Set communication to be asynchronous

    //For 16Mhz FOSC
    TXSTA1bits.BRGH = 1;    //Enables the high speed Baud Rate Generator
    BAUDCON1bits.BRG16 = 1; //Enables the 16 bit Baud Rate Generator

    //Actual rate is 9592 with these settings, very low error   
    SPBRGH1 = 0b00000001; //  Higher Byte of the Baud Rate Generator, set to 1 which is 256 in 16 bit; 256+160 = 416
    SPBRG1 = 0b10100000;  // Lower Byte of the Baud Rate Generator -> SPBRGH:SPBRGX, 16 bit register; set to 160

    //Default for STN1110 w ELM327 Protocol
    BAUDCON1bits.CKTXP = 0; //Idle State for Transmit is high
    BAUDCON1bits.DTRXP = 0; //Data Receive Polarity bit is not inverted (active high)

    //Configure EUSART1 Recieve Bits
    RCSTA1bits.SPEN = 1; //Enables EUSART, automatically sets the I/O configuration
    RCSTA1bits.CREN = 1; //Enable the Continuous Recieve
    RCSTA1bits.RX9 = 0; //Enable 8 bit reception
    
    //Interrupt Settings
    PIE1bits.RC1IE = 1;   // Disable UART receive interrupt
    INTCONbits.PEIE = 1;  // Disable peripheral interrupts
    INTCONbits.GIE = 1;   // Disable global interrupts
}

void __interrupt() UART_ISR(void) { //using an interrupt service routine for when a byte is recieved
    if (PIE1bits.RC1IE && PIR1bits.RC1IF) { // If UART1 is on and the recive interrupt is high, meaning it is full
        RX_char = RCREG1; // Read byte immediately to clear RCIF

        // Handle overrun error if necessary
        if (RCSTAbits.OERR) {
            RCSTAbits.CREN = 0;
            RCSTAbits.CREN = 1;
        }

        // Filter out the non displayable characters, and make sure the character being sent isnt a ">" so that we know its a full line we want
        if ((RX_char >= 32 && RX_char <= 126) && RX_char != '>') {
            buffer[buffer_count] = RX_char; //set the RX_char value to a value in the buffer array, increment each time
            buffer_count++;
        }

        if (RX_char == '>' || RX_char == '\n' || buffer_count >= BUFFER_SIZE-1) { //if the character coming in is ">", or "\n" or the buffer count is at its max, end the string
            buffer[buffer_count] = '\0'; // Null-terminate
            message_complete = 1;        // Set flag to display later
        }
    }
}

void UART1_SendString(char string[]) { //User inputs desired string in the array with a carriage return value '\r'
    for (unsigned int i = 0; string[i] != '\0'; i++){ //function iterates through the string, each time assigning the character in the array to the character to send in the Send_Char function
        UART1_SendChar(string[i]); 
    }
}

void UART1_SendChar(char c){ //Function that takes in a character
    while (!TXSTAbits.TRMT); //while the Transmist Shifter Register Status bit is empty, wait here, once its full, set the value of the char c to the value of the TXREG
        TXREG = c;
}

// ######## END OF UART CONFIGURATION AND SETUP BLOCK ######

// Request RPM Code

unsigned char hex_char_to_value(char c) { 
    if (c >= '0' && c <= '9') return c - '0'; //if the character is between 0 and 9, (ASCII codes 48-57) subtract 0 to get its actual value
    if (c >= 'A' && c <= 'F') return c - 'A' + 10; //if the character is between A and F (ASCII codes 65-70) subtract A (which is 10 in hex) and add 10
    if (c >= 'a' && c <= 'f') return c - 'a' + 10; //if the character is between a and f (ASCII codes 97-102) subtract a and add 10
    return 0; // Error case if nothing matches in these places, return an error value of zero
}

void print_RPM(void){

    UART1_SendString("010C\r"); // Request RPM OBDII PID
    while(!message_complete) {
        // Wait here until full reply received
    }

    unsigned int A = (hex_char_to_value(buffer[4]) << 4) | hex_char_to_value(buffer[5]); //go to the 4th in
    unsigned int B = (hex_char_to_value(buffer[6]) << 4) | hex_char_to_value(buffer[7]);
    unsigned int RPM = ((A * 256) + B) / 4;
    
    char rpm_string[16]; //create a string call it rpm_string
    sprintf(rpm_string, "%u", RPM); // Turn RPM number we grabbed from converted hex values into a string

    LCD_cursor_set(1,1);
    LCD_write_string("RPM");
    LCD_cursor_set(2,1);
    LCD_write_string(rpm_string);

    buffer_count = 0;     // Clear buffer for next message
    message_complete = 0; // Ready for next

    __delay_ms(500);
}

void print_Vbatt(void) {

    UART1_SendString("ATRV\r"); // Request Battery Voltage using AT command
    while(!message_complete) {
        // Wait here until full reply received
    }
    
    LCD_cursor_set(1,7);
    LCD_write_string("VBatt");
    LCD_cursor_set(2,7);
    LCD_write_string(buffer);
    LCD_cursor_set(2,12);
    LCD_write_string("     ");

    buffer_count = 0;     // Clear buffer for next message
    message_complete = 0; // Ready for next

    __delay_ms(500);
}

void print_AI_Temp(void){

}

