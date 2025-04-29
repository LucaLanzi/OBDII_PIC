# ECE 3301 Final Project: OBDII Scanner using the PIC18F46K22 ##
## By Luca Lanzillotta and Zihong Zheng ##

### Assignment Requirements: ###
* Final Project Must Use:
    * Serial Communication: UART/I2C/SPI
        * Setup UART Communication for the PIC with the correct values, and ensure that the pic can send and recieve commands
    * CCP Module
        * Use compare module to count to 30s, if the device hasnt been plugged in to the OBDII port till then, trigger an interrupt and request the user to plug in the device
    * Timer
        * Delays

*Abstract:*
* [*Link to original Cornell Project*](https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/s2012/ppv5/index.html)
* Create an OBD II scanner that is compatible with any ISO OBC II standard to read basic vehicle telemetry and diagnostics. The design will utilize a Sparkfun OBD II to UART intpreter board and will utilize half duplex capabilities to send requests from the PIC18F46K22 and receive data to display on an LCD. Additionally the board will include buttons to navigate a menu to send and receive specific requests such as RPM, Oil Pressure, Cylinder Diagnostics, and will have a dedicated mode to display live data on the display. 

### Components: ###
* Microcontroller: 
    * PIC18F46K22 and curiosity board (might not use if we design a PCB)
* OBC II to UART: 
    * Sparkfun OBD-II to UART board
* Display: 
    * Generic Microcontroller compatible LCD
* Buttons: 
    * 4/5 buttons in a d-pad style arrangement to emulate a controller d-pad
* Casing: 
    * 3D printed encasing to hold all the components together
* PCB: 
    * Potential to design and manufacture a little PCB that holds the pic together with its necessary connectors to communicate to the board.

# Design Notes: #
## System Block Diagram: ##

![System Block Diagram](Images/OBDII_UARTBlockDiagram.png)

## System State Diagram: ## 

![System State Diagram](Images/SystemStateDiagram.drawio%20(1).png)

## Startup Sequence: ##

### Welcome Screen ###
* *note:* To be displayed on a 2x16 LCD Display using the PIC18F46K22    

* Write to the LCD upon startup the following string in the example below, then use TMR0 set with the proper clock frequency and calculation to create a 1 second delay
  Be sure to include the LCD.h file as well as the LCD.c file to ensure proper LCD use.

        Example:
        
        **Welcome to OBDII-PIC**
        https://github.com/LucaLanzi/OBDII_PIC
        // 1s delay

        Code Chunk:

* Detects which ISO standard is being used
    * Wait for serial 'OK' once board has detected OBDII standard
    Use an if/else statement that is constantly being checked with a flag of some kind, so we always wait for the IC to initiate until the 'ok' command is sent, we can poll for this.
    Once this poll flag has been cleared (while loop with a flag command) we can notify the user that the device has been configured.
    * Notify user "OBDII-PIC Configured"
    * SAE ISO version: xxxxxxxx
    Here we can utilized the 'AT DP' command to Display the Protocol being used, then store it, and display it in the next splash screen sequence, after which we can meet it with another 1 second delay caused by the TMR0 of the PIC.

            Example:

            OBDII-PIC Configured
            SAE ISO Ver: xxxxxxx
            // 1s delay


        Code Chunk:
    

## Standard Operation Mode: ##

* Goes to a menu where you can do the following:
* Using a D-PAD style button layout, you can move a cursor and select:
The Cursor should be flashing in the position it is in. Controls are Left, Right, Enter and Back, where each acronym is an example of the 4 modes of operation
        
        Example:

        Menu
            L.R.  S.E.C  C.E.C  S.I

### Live readings: ###
* DIsplay RPM, Air Intake temp, Coolant Temp, Battery Voltage
        
        Example: 

            Live Reading Mode:
                RPM     A.I. Temp  C. Temp   Batt. V    
                1000    xxF        xx.F    12.4V  

    Code Chunk:

### Scan Error Codes: ###
* Display OBDII PID Error Codes (User can use up and down keys to navigate through them) 
        
        Example:
            
            Error Code Mode:
                Error Code(s): xxxxxxx

    Code Chunk:
           

### Clear Error Codes: ###
* Cursor Should blink over the selection preference

        Example:

                Clear Error Code(s)? Y/N    
    
    Code Chunk:

### Get System info: (ISO Standard and Firmware version) ###
        
        Example:

            System Info Mode:
                ISO Ver: xxxxxx, Firm. V: xxxxx

    Code Chunk:

* This is entirely possible over the serial interface by sending an AT command, utilizing the ELM327 command set
Refer to pg 10. of the [ELM327 Datasheet](https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf)
* Here it states, by sending the following:
    "AT DP" over serial the IC intepreter will send the command 'Describe the current protocol' to the STN1110 and will forward that as an OBDII PID request to the vehicle
    what will be returned is a string of some type with the ISO standard desription.
    By using this we can, at the request of the user, when they navigate to the proper setting and hit enter, send the AT command over serial and display the result on the LCD


## Shut Down Operation: ##
* Just unplug the damn thing

### LCD User Interaction Configuration: ###
* The user must be able to navigate left and right on the 2x16 LCD utilizing a cursor and to be able to select a mode of operation, and exit a current mode
* The LCD.h and LCD.c header and source files respectively provide us with the functionality of a blinking cursor, allowing the user to see where their cursor position is
* By creating a command function that keeps track of where this cursor is, we can increment and decrement the position of the cursor respectively to where the user would like it to be, and by utilizing simple integer variables, we can keep track of where within the sub menus we are.

To do this we need:
* Define the Menu options (we can use an array)
* We draw the Menu, and with each user input we can determine where we are flashing the cursor along this array
* When the user hits the 'enter' button, we set which value of the menu we want to excecute, triggering something like a switch case to display the menu we would like to enter
* If we are in a sub menu, we can use the 'back' button to trigger us out of the switch case and go back to the main screen.

    Code Chunk: 



### OBDII-UART ###

* UART and BAUDCON Setup:
Setup for UART Transmit/Recieve: [Page 272](https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F2X-4XK22-Data-Sheet-40001412H.pdf)

BAUD Rate is 9600 bps for OBDII-UART Sparkfun Board
Setup for BAUD Rate:
16-bit asynchronous, using the high speed Baud rate generator, and the 16 bit baud register
for this, SYNC = 0 (for asynchronous), BRG16 = 1 (using the 16 bit high speed baud rate generator), BRGH = 1 for setting the value for the 16 bit register.

To set BAUD rate:
16Mhz FOSC
Use 16-bit/Asynchronous for Sync = 0, BRG16 = 1, BRGH = 1:
Desired Baud Rate: FOSC/4[(n+1)]
where n = 416
BAUD RATE = 16,000,000/(4(416+1)) = 9592.32

Code Chunk for UART Setup:

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

* UART Tx/RX Parser:
Because we are sending and receiving data over UART, we need to be able to recieve one byte at a time, store it somewhere, and then move along and store the next byte. We also need to know when this starts and stops. For this we need to do the following

* Define when we are receiving data: 
    Data is recieved in the RCREG1, when a byte is sent and recieved, the RC1IF interrupt flag in the PIR1 register is set to 1. RC1IF = 1 means a byte was recieved.
    Once this is true, we can read the value in RCREG1, and store it in a buffer array. Once we have read it and stored it, the RC1IF flag is cleared, and we can then be ready to recieve the next byte.
    
    We need to check for a condition (when RC1IF is 1) then assign that byte to a variable, then put that variable in a position in an array, and then repeat but doing so in the next part of the array

    Code Chunk:

    

### Reference Documents and Useful Links: ###

# OBDII-UART Documentation: #
* [Original Project Documentation - OBDII Data Logger Cornell University Project](https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/s2012/ppv5/index.html)
* [Sparkfun OBD-II to UART board](https://www.sparkfun.com/sparkfun-obd-ii-uart.html)
* [Sparkfun OBD-II to UART board schematic](https://cdn.sparkfun.com/assets/2/4/d/c/d/520ab4c5757b7f5e0acc8c0e.pdf)
* [Sparkfun OBD-II to UART Implementation Guide](https://learn.sparkfun.com/tutorials/obd-ii-uart-hookup-guide/all)
* [OBD Protocol Information](https://en.wikipedia.org/wiki/On-board_diagnostics#Standard_interfaces)
* [OBD PID Information](https://en.wikipedia.org/wiki/OBD-II_PIDs)
* [Display Hookup Guide](https://learn.sparkfun.com/tutorials/basic-character-lcd-hookup-guide)
* [Github Syntax and Formatting](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax)

# PIC18F46K22 UART Documentation: #
* [PIC18F46K22 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F2X-4XK22-Data-Sheet-40001412H.pdf)


### To Do Notes ###
* Setup UART to talk to the STN1110

* Display the UART out from the STN1110 on the LCD