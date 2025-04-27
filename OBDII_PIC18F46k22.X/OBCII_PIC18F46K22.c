#include <xc.h>
#include "PIC18F46K22-Config.h"
#include "LCD.h"

#define _XTAL_FREQ 16000000 //defines the internal frequency to 16Mhz (Configurable Pg. 30)

//UART Variable Config
#define BUFFER_SIZE 32 //Set the total byte size of the UART buffer
char buffer[BUFFER_SIZE]; //Define a character array to hold the incoming bytes from Rx
unsigned char buffer_count = 0; //a counter for the array index
char RX_char; //variable to store the byte read from the RCREG


//Function Prototypes
void UART1_Init(void); //initialize UART1
char UART1_Read(void); //Read the UART RX, return the RCREG1 byte
void UART1_Save_Buffer(void); //Save RCREG to the UART1 buffer array
void UART1_SendString(char string[]); //Function to send string to ELM327
void UART1_SendChar(char c); //Function to break down the string and send one character at a time to the ELM327

void main(void){

    OSCCON = 0b01110000; //16Mhz FOSC
    

    LCD_init(); //initialize the LCD
    LCD_clear(); //clear the LCD
    UART1_Init(); //initialize UART1

    __delay_ms(2000);
    UART1_SendString("ATZ\r"); //resets the ELM327
    __delay_ms(500);
    UART1_SendString("ATE0\r"); //turns off echo
    __delay_ms(500);
    UART1_SendString("ATI\r"); //

    while(1){
        UART1_Save_Buffer(); //save the incoming result and print it
    }


}


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
    PIE1bits.RC1IE = 0;   // Disable UART receive interrupt
    INTCONbits.PEIE = 0;  // Disable peripheral interrupts
    INTCONbits.GIE = 0;   // Disable global interrupts
}

char UART1_Read(void) {

    // Wait here until the flag goes, high, this means that a byte was sent over to the RX pin
    while (!PIR1bits.RCIF);

    // Handle overrun error by resetting receiver
    if (RCSTAbits.OERR) { //This checks to see if a third character was received before we attempted to read it, therefore it clears it and resets
        RCSTAbits.CREN = 0; //turns off the continuous receive, clears the values in it
        RCSTAbits.CREN = 1; //turns it back on
    }
    return RCREG1;  // Set the RX_char variable to the byte recieved at the RCREG
}

void UART1_Save_Buffer(void){
    while (PIR1bits.RC1IF) { //RCREG1 is returned but not read, so the flag is set to '1', meaning we need to access its value

        RX_char = UART1_Read(); //Save the value in the RCREG1 to the RX_char variable

        if (RX_char >= 32 && RX_char <= 126) { //LCD has a set group of printable ASCII characters, here we check if the ones we recieve are printable or not
            buffer[buffer_count++] = RX_char; //Add that byte character received from the RX_Char variable to the buffer array
        }
        

        // If the incoming character is an end of line from the ELM327 ">" or our buffer is full, add a "\0" to the last buffer space to make it a C-string
        if (RX_char == '>' || RX_char == '\n' || buffer_count >= BUFFER_SIZE-1) {
            buffer[buffer_count] = '\0'; // '\0' creates a character array into a c-strings
            buffer_count = 0; //reset the buffer count

            LCD_clear();
            LCD_cursor_set(1,1);
            LCD_write_string(buffer); //print the string from the buffer array to the LCD
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


