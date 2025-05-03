#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "PIC18F46K22-Config.h"
#include "LCD.h"

//Preset Definitions
#define _XTAL_FREQ 16000000 //defines the internal frequency to 16Mhz (Configurable Pg. 30)
#define short_delay_preset 50
#define long_delay_preset 2000
#define splash_delay 4 // the number of seconds you want to delay the port checker
#define MAX_DTC_COUNT 10

//I/O Setup Function Prototypes
void input_init(void);
void ADC_init(void);

// ## EUSART1 Function Prototypes and Variables
    #define BUFFER_SIZE 256 //Set the total byte size of the UART buffer (Huge so we can store many DTC's in the case of having many of them)
    char buffer[BUFFER_SIZE]; //Define a character array to hold the incoming bytes from Rx (Array has 32 elements, but index range is from 0-31)
    unsigned char buffer_count = 0; //a counter for the array index
    char RX_char; //variable to store the byte read from the RCREG
    volatile char message_complete = 0; // flag for completing UART message
    void UART1_Init(void); //initialize UART1
    void UART1_SendString(char string[]); //Function to send string to ELM327
    void UART1_SendChar(char c); //Function to break down the string and send one character at a time to the ELM327

// ## Welcome Splash and Disconnect Flag Function Prototypes
    void welcome_splash(void);
    void ccp1_init(void); //Use CCP1 special event interrupt to trigger after 3s of being on and not plugged in
    void tmr1_init(void);

// ## Main Menu Function Prototypes
    int menu_sel = -1; // -1 means we are in menu, not in any mode
    unsigned int readADC(); //For Pot input readings
    unsigned int result; //ADC result
    void display_mm(void);
    void main_menu(void);
    void parsing_notif(void); 
    void clear_parsing_notif(void);

// ## Live Reading Mode Function Prototypes and Variables ##
    //Read RPM Function Prototypes and Variables
    unsigned char extract_two_pid_bytes(const char* pid, uint8_t* A, uint8_t* B);
    void live_reading_mode(void);
    void print_RPM(void);
    void print_Vbatt(void);
    unsigned char hex_char_to_value(char c); //function definition for hex to charcter to value converter
    unsigned int A_rpm; //variable declarations for RPM formula
    unsigned int B_rpm;
    unsigned int RPM;
    char rpm_string[16]; //create a string call it rpm_string
    //Read Air Intake Function Prototypes and Variables
    unsigned char extract_single_pid_byte(const char* pid, uint8_t* A);
    void print_AI_Temp(void);
    char air_intake_string[16]; //create a string call it air_intake_string
    unsigned int A_air_intake;
    unsigned int air_intake_temp;                                

// ## Read Diagnostic Codes Mode Function Prototypes and Variables ##
    void diagnostic_trouble_codes(void);
    void decode_dtc(unsigned int b1, unsigned int b2, char* dtc); //first byte, second byte and the dtc character which is passing a pointer 
    char dtc_codes[MAX_DTC_COUNT][6]; // Each DTC is 5 chars + null terminator

// ## Clear Diagnostic Codes Mode Function Prototypes and Variables ##
    void clear_diagnostic_codes(void);

// ## Display System Information Mode Function Prototypes and Variables ##
    void print_ELMVer(void);
    void print_SAEVer(void);
    void display_system_info(void);         

    // MAIN loop
    void main(void){

    //OSCCON setup
    OSCCON = 0b01110000; //16Mhz FOSC
     
    
    //Initializations
    LCD_init(); //initialize the LCD
    LCD_clear(); //clear the LCD
    input_init(); //initialize the inputs
    ADC_init();
    UART1_Init(); //initialize UART1

    __delay_ms(long_delay_preset);
    UART1_SendString("ATE0\r"); //Turn off Echo from ELM327
    __delay_ms(short_delay_preset);

        while(1){
                welcome_splash(); //welcomes the user to OBDIIPIC, handles case if user doesnt have the OBDIIUART plugged in
                main_menu(); //holds all the menu options and modes
            }

}

//######### Input Initialization #############
void input_init(void){

    //Input Declarations
    TRISBbits.TRISB4 = 1; //Enter button input
    ANSELBbits.ANSB4 = 0; //Disable analog for B4

    TRISCbits.TRISC5 = 1; //Back button input
    ANSELCbits.ANSC5 = 0; //Disable analog for C5

    TRISAbits.TRISA0 = 1; //On Board Potentiometer input
    ANSELAbits.ANSA0 = 1; //Enable the potentiometer input as analog
}

// ######### ADC INITIALIZATION #################
void ADC_init(void){
    // ADC Configuration
    ADCON0 = 0b00000001;  // Enable ADC, select channel AN0
    ADCON1 = 0b00000000;  // VDD and VSS as references
    ADCON2 = 0b10101010;  // Right justified, 12TAD, Fosc/32
}

// ######## UART CONFIGURATION AND SETUP BLOCK######
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


// ## INTERRUPT SERVICE ROUTINE
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

// ######## WELCOME SPLASH/DISCONNECT HANDLE ######
void ccp1_init(void){
    CCP1CONbits.CCP1M3 = 1; //Compare Mode: Special Event Trigger
    CCP1CONbits.CCP1M2 = 0;
    CCP1CONbits.CCP1M1 = 1;
    CCP1CONbits.CCP1M0 = 1;

    //Using 16Mhz FOSC, we can count up to .1 Seconds
    //@16Mhz FOSC, 1:8 Prescalar, CCPR1 = 50,000 counts = 10 ms
    CCPR1H = 195;
    CCPR1L = 80;

}

void tmr1_init(void){
    // timer 1 setup for CCP1 Use
    T1CONbits.T1CKPS1 = 1; // Prescaler bits: 1:8
    T1CONbits.T1CKPS0 = 1; 
    T1CONbits.T1OSCEN = 0; // Timer1 Oscillator OFF
    T1CONbits.T1SYNC = 1;  // Do not synchronize external clock input
    T1CONbits.TMR1CS1 = 0; // Clock source = Fosc/4
    T1CONbits.TMR1CS0 = 0;
    T1CONbits.TMR1ON = 1;  // Turn on the timer
}

void welcome_splash(void) {
    volatile int plug_flag = 0; // 0 = disconnected, 1 = connected
    volatile unsigned int CCP1IF_counter = 0; // timer counter

    // Splash Screen when the OBDIIPIC turns on
    LCD_clear();
    LCD_cursor_set(1, 1);   
    LCD_write_string(">>> OBDIIPIC <<<");
    LCD_cursor_set(2, 1);
    LCD_write_string(">>>> V1.0 <<<<");

    UART1_SendString("ATL0\r"); //testing the no line wrap so 

    ccp1_init(); // Setup CCP1 module
    tmr1_init(); // Start Timer1

    CCP1IF_counter = 0;
    plug_flag = 0;

    while (1) { // stay here until successful 5 seconds of connection
        if (PIR1bits.CCP1IF) { // every 0.1 seconds
            PIR1bits.CCP1IF = 0; 
            T1CONbits.TMR1ON = 1; // Restart timer
            CCP1IF_counter++;

            LCD_cursor_set(2, 16);
            LCD_write_variable((CCP1IF_counter) / 10, 1);

            // Every second (10 ticks of 0.1s) send an ATI ping
            if (CCP1IF_counter % 10 == 0) {
                UART1_SendString("ATI\r");
            }
        }

        // Check if message was received
        if (message_complete) {
            plug_flag = 1; // Connected
            buffer_count = 0;
            message_complete = 0;
        }

        // If connected, continue counting
        if (plug_flag) {
            if (CCP1IF_counter >= splash_delay * 10) { 
                // Connected continuously for n seconds
                break; // Exit splash
            }
        } else {
            // No connection or lost connection — reset timer
            CCP1IF_counter = 0;
            LCD_cursor_set(2, 1);
            LCD_write_string("OBDII Not Found ");
            UART1_SendString("ATI\r"); // keep pinging to find ELM327
            __delay_ms(short_delay_preset);
        }
    }

    // ELM detected after successful 5 seconds
    LCD_clear();
    LCD_cursor_set(1, 1);
    LCD_write_string("<OBDII Detected>");
    LCD_cursor_set(2,1);
    LCD_write_string("================");
    __delay_ms(2 * long_delay_preset);

    UART1_SendString("ATE0\r"); // Disable Echo
    __delay_ms(short_delay_preset);
}



// ######## MAIN MENU BLOCK #########################################################################################################################################################
unsigned int readADC() {
    ADCON0bits.GO = 1;  // Start ADC conversion
    while (ADCON0bits.GO);  // Wait for conversion to finish
    __delay_ms(10);
    return ((ADRESH << 8) | ADRESL);  // Combine 8-bit results
}

void display_mm(void){ //Main Menu Display function
    LCD_cursor_set(1,1);
    LCD_write_string("MENU  <OBDIIPIC>");
    LCD_cursor_set(2,1);
    LCD_write_string("LRM DTC CDC DSI");
}

void parsing_notif(void) { //display a little loading icon
    LCD_cursor_set(1,16);
    LCD_write_string("~");
}
void clear_parsing_notif(void){
    LCD_cursor_set(1,16);
    LCD_write_string("^");
}

void main_menu(void){ //main menu loop, checks back and enter state and takes in mode set by potentiometer input
        
    LCD_clear();
    display_mm(); //main menu display function
    
    while(1){
        display_mm();
        result = readADC();
        if(result >= 0 && result <= 255){ //set first value range
            menu_sel = 0;
            LCD_cursor_set(2,1);
           
        }
        if(result >= 256 && result <=511){ //set second value range
            menu_sel = 1;
            LCD_cursor_set(2,5);
           
        }
        if(result >=512 && result <=767){ //set third value range
            menu_sel = 2;
            LCD_cursor_set(2,9);
        }
        if(result >= 768 && result <=1023 ){ //set fourth value range
            menu_sel = 3;
            LCD_cursor_set(2,13); 
        }
        LCD_configure_cursor_blink(1);

        if (PORTBbits.RB4 == 0) { // Enter button pressed (Assuming active low button)
            __delay_ms(20); // Simple debounce
            if (PORTBbits.RB4 == 0) { // Confirm still pressed
                LCD_configure_cursor_blink(0);
                LCD_clear();

                // Now go into the selected mode
                switch (menu_sel) {
                    case 0:
                        while (1) {
                            live_reading_mode();
                            // Check for back button
                            if (PORTCbits.RC5 == 0) {
                                __delay_ms(20);
                                if (PORTCbits.RC5 == 0) {
                                    LCD_clear();
                                    display_mm(); // Go back to main menu display
                                    menu_sel = -1; // reset mode
                                    break; // break out of inner while(1)
                                }
                            }
                        }
                        break;

                    case 1:
                        while (1) {
                            // Read Diagnostic Codes logic here
                            diagnostic_trouble_codes();
                            // Check for back button
                            if (PORTCbits.RC5 == 0) {
                                    __delay_ms(20); //button debounce
                                if (PORTCbits.RC5 == 0) {
                                    LCD_clear();
                                    display_mm(); 
                                    menu_sel = -1; //reset the mode
                                    break; //break out of inner while(1)
                                }
                            }
                        }
                        break;

                    case 2:
                        while (1) {
                                __delay_ms(200); //debounce the button press
                                clear_diagnostic_codes(); //run the mode
                                menu_sel = -1;
                                break; //once its done exit the while loop and shoot you back to the menu
                            if (PORTCbits.RC5 == 0) {
                                __delay_ms(20);
                                if (PORTCbits.RC5 == 0) {
                                    LCD_clear();
                                    display_mm();
                                    menu_sel = -1;
                                    break;
                                }
                            }
                        }
                        break;

                    case 3:
                        while (1) {
                            display_system_info(); // DSI mode
                            //stay here unless back button is pressed
                            if (PORTCbits.RC5 == 0) {
                               __delay_ms(20);
                                if (PORTCbits.RC5 == 0) {
                                    LCD_clear();
                                    display_mm();
                                    menu_sel = -1;
                                    break;
                                }
                            }
                        }
                        break;

                    default:
                        break;
                }
            }
        }

        __delay_ms(50); // Small delay for stability
    }
}







// ######## LIVE READING MODE BLOCK #################################################################################################################
unsigned char hex_char_to_value(char c) {  // Hex char to value handler function
    if (c >= '0' && c <= '9') return c - '0'; //if the character is between 0 and 9, (ASCII codes 48-57) subtract 0 to get its actual value
    if (c >= 'A' && c <= 'F') return c - 'A' + 10; //if the character is between A and F (ASCII codes 65-70) subtract A (which is 10 in hex) and add 10
    if (c >= 'a' && c <= 'f') return c - 'a' + 10; //if the character is between a and f (ASCII codes 97-102) subtract a and add 10
    return 0; // Error case if nothing matches in these places, return an error value of zero
}

// Tries to extract two data bytes following a response like "41 0C"
unsigned char extract_two_pid_bytes(const char* pid, uint8_t* A, uint8_t* B) {
    char* ptr = strstr(buffer, pid);
    if (ptr) {
        unsigned int a = 0, b = 0;
        if (sscanf(ptr + strlen(pid), "%x %x", &a, &b) == 2) {
            *A = (uint8_t)a;
            *B = (uint8_t)b;
            return 1; // success
        }
    }
    return 0; // fail
}


void print_RPM(void) {
    
    memset(buffer, 0, sizeof(buffer)); //clear the buffer before sending a message
    buffer_count = 0;
    message_complete = 0;
    
    UART1_SendString("010C\r");

    while(!message_complete) {
        if (PORTCbits.RC5 == 0) {
            __delay_ms(20);
            if (PORTCbits.RC5 == 0) {
                LCD_clear();
                display_mm();
                menu_sel = -1;
                return;
            }
        }
        parsing_notif();
    }
    clear_parsing_notif();

    uint8_t A = 0, B = 0;
    if (extract_two_pid_bytes("41 0C", &A, &B)) {
        RPM = ((A << 8) | B) / 4;
    } else {
        RPM = 0; // fallback
    }

    sprintf(rpm_string, "%u", RPM);
    LCD_cursor_set(2,1);
    LCD_write_string("     ");
    LCD_cursor_set(1,1);
    LCD_write_string("RPM");
    LCD_cursor_set(2,1);
    LCD_write_string(rpm_string);

    buffer_count = 0;
    message_complete = 0;
}


void print_Vbatt(void) { // Request and display VBatt AT Command

    UART1_SendString("ATRV\r"); // Request Battery Voltage using AT command
    while(!message_complete) {
        // Wait here until full reply received
        if (PORTCbits.RC5 == 0) { //back function
            __delay_ms(20);
            if (PORTCbits.RC5 == 0) {
                LCD_clear();
                display_mm(); // Go back to main menu display
                menu_sel = -1; // reset mode
                break; // break out of inner while(1)
            }
        }
        parsing_notif();
    }
    clear_parsing_notif();

    LCD_cursor_set(2,7);
    LCD_write_string("     "); //clear the data entry
    
    LCD_cursor_set(1,7);
    LCD_write_string("VBatt");
    LCD_cursor_set(2,7);

    if (!strchr(buffer, 'V')) { //Since the voltage reading always containts a V, we can use this to check if we are getting SEARCHING... or actual voltage
        LCD_write_string("N/A"); //So if we dont have a V in the incoming buffer, print this
    }
    else {
    LCD_write_string(buffer); //If we do have a V, 
    }

    buffer_count = 0;     // Clear buffer for next message
    message_complete = 0; // Ready for next
}

unsigned char extract_single_pid_byte(const char* pid, uint8_t* A) {
    char* ptr = strstr(buffer, pid);
    if (ptr) {
        unsigned int a = 0;
        if (sscanf(ptr + strlen(pid), "%x", &a) == 1) {
            *A = (uint8_t)a;
            return 1; // success
        }
    }
    return 0; // fail
}

void print_AI_Temp(void) {
    UART1_SendString("010F\r");

    while(!message_complete) {
        if (PORTCbits.RC5 == 0) {
            __delay_ms(20);
            if (PORTCbits.RC5 == 0) {
                LCD_clear();
                display_mm();
                menu_sel = -1;
                return;
            }
        }
        parsing_notif();
    }
    clear_parsing_notif();

    uint8_t A = 0;
    if (extract_single_pid_byte("41 0F", &A)) {
        air_intake_temp = A - 40;
    } else {
        air_intake_temp = 0;
    }

    sprintf(air_intake_string, "%u", air_intake_temp);
    LCD_cursor_set(2,13);
    LCD_write_string("   ");
    LCD_cursor_set(1,13);
    LCD_write_string("AIT");
    LCD_cursor_set(2,13);
    LCD_write_string(air_intake_string);
    LCD_cursor_set(2,15);
    LCD_write_string("C");

    buffer_count = 0;
    message_complete = 0;
}


void live_reading_mode(void){ //Function that calls all three from above
            print_RPM(); //Print RPM Values
            print_Vbatt(); //Print Vbatt;
            print_AI_Temp(); //Print Air Intake Temperature
           
}





// ## DISPLAY SYSTEM INFORMATION MODE BLOCK ########################################################################################################################################
void print_ELMVer(void) {

    UART1_SendString("ATI\r"); // Request AT Information Request

    while(!message_complete) {
        // Wait here until full reply received
        if (PORTCbits.RC5 == 0) {
            __delay_ms(20);
            if (PORTCbits.RC5 == 0) {
                LCD_clear();
                display_mm(); // Go back to main menu display
                menu_sel = -1; // reset mode
                break; // break out of inner while(1)
            }
        }
        parsing_notif();
    }
    clear_parsing_notif();

    LCD_cursor_set(1,1);
    LCD_write_string("OS:");
    LCD_write_string(buffer); //If we do have a V, 
    

    buffer_count = 0;     // Clear buffer for next message
    message_complete = 0; // Ready for next
}

void print_SAEVer(void){
    UART1_SendString("ATDP\r"); // Request Supported SAE version from vehicle
    
    while(!message_complete) {
        // Wait here until full reply received
        if (PORTCbits.RC5 == 0) { //back condition
            __delay_ms(20);
            if (PORTCbits.RC5 == 0) {
                LCD_clear();
                display_mm(); // Go back to main menu display
                menu_sel = -1; // reset mode
                break; // break out of inner while(1)
            }
        }
        parsing_notif();
    }
    clear_parsing_notif();

    LCD_cursor_set(2,1);
    LCD_write_string("P:");
    LCD_write_string(buffer); //If we do have a V, 
    

    buffer_count = 0;     // Clear buffer for next message
    message_complete = 0; // Ready for next
}

void display_system_info (void){
    LCD_cursor_set(1,4);
    LCD_cursor_set(2,5);
    print_ELMVer();
    print_SAEVer();
}

// ######## READ DIAGNOSTIC TROUBLE CODE BLOCK ########################################################################################################################################
void diagnostic_trouble_codes(void) {
    // Clear buffer and flags
    buffer_count = 0;
    message_complete = 0;
    memset(buffer, 0, sizeof(buffer));

    UART1_SendString("03\r");  // Send to ELM327

    while (!message_complete) {
        parsing_notif();  // Optional LCD spinner
    }
    clear_parsing_notif();

    LCD_clear();
    LCD_cursor_set(1, 1);
    LCD_write_string("Raw DTC's:");

    LCD_cursor_set(2, 1);
    for (int i = 0; i < 16 && buffer[i] != '\0'; i++) {
        if (buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == '>') break;
        LCD_write_char(buffer[i]);  // Display raw char
    }

    buffer_count = 0;
    message_complete = 0;
}






// ######## CLEAR DIAGNOSTIC ERROR CODE BLOCK #######################################################################################################################################
void clear_diagnostic_codes(void){

    int opt_sel = 0; //Y/N option select
    LCD_clear();
    LCD_cursor_set(1,1);
    LCD_write_string("Clear Code(s)?");
    LCD_cursor_set(2,1);
    LCD_write_string("Y/N <<<<<<<<<<<<");
    LCD_configure_cursor_blink(1);

    while(1){
        result = readADC();
        if(result >= 0 && result <= 511){ //set first value range
            opt_sel = 0;
            LCD_cursor_set(2,1);
           
        }
        if(result >= 512 && result <=1023){ //set second value range
            opt_sel = 1;
            LCD_cursor_set(2,3);
        }
            if (PORTCbits.RC5 == 0) { //back button polling
                __delay_ms(20);
                if (PORTCbits.RC5 == 0) {
                    LCD_clear();
                    display_mm(); // Go back to main menu display
                    menu_sel = -1; // reset mode
                    break; // break out of inner while(1)
                }
            }

        if (PORTBbits.RB4 == 0) { // Enter button pressed (Assuming active low button)
            __delay_ms(20); // Simple debounce
            if (PORTBbits.RB4 == 0) {
                switch(opt_sel){
                    case 0:

                    UART1_SendString("04\r"); //Send Clear Code
                    while(!message_complete){
                        //wait for accept message
                        parsing_notif();
                    }
                    clear_parsing_notif();

                    LCD_clear();
                    LCD_cursor_set(1,1);
                    LCD_write_string("Codes Cleared");
                    LCD_cursor_set(2,1);
                    LCD_write_string("To Menu...");

                    menu_sel = -1; //set menu flag value
                    __delay_ms(long_delay_preset); //2 second pause

                        break;

                    case 1:

                    LCD_clear();
                    LCD_cursor_set(1,1);
                    LCD_write_string("Nothing Cleared");
                    LCD_cursor_set(2,1);
                    LCD_write_string("To Menu...");

                    menu_sel = -1; //set menu flag value
                    __delay_ms(long_delay_preset); //2 second pause
                        break;

                    default:
                        break;
                }
            break;
            }
          }
    }
}

