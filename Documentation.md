# ECE 3301 Final Project: OBDII Scanner using the PIC18F46K22 ##
### By Luca Lanzillotta and Zihong Zheng ###
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

## Design Notes: ##

### Startup Sequence: ###
    * Welcome Display, OBDII-PIC, created by Luca and Zihong
    Example:
        Welcome to OBDII-PIC
        By Luca and Zihong      V1.0
    (waits 1 second, use TMR on pic)

    * Detects which ISO standard is being used
        * Wait for serial 'OK' once board has detected OBDII standard
        * Notify user "OBDII-PIC Configured"
        * SAE ISO version: xxxxxxxx
    (waits 1 second)

### Standard Operation Mode ###
    * To be displayed on a 2x16 LCD Display using the PIC18F46K22

    * Goes to a menu where you can do the following:
        Using a D-PAD style button layout, you can move a cursor and select:
        The Cursor should be flashing in the position it is in. Controls are Left, Right, Enter and Back
        Example:

        Menu
            L.R  S.E.C  C.E.C  S.I

        * Each acronym is an example of the 4 modes of operation

        * Live readings:
            * Preset: RPM, Oil Pressure, Battery Voltage, Fuel Percentage (Live display on 1st row of LCD)
            Example: 

            Live Reading Mode:
                RPM     Oil     Batt. V  Fuel %
                1000    xxPSI   12.4V      30%

        * Scan Error Codes (OBDII/AT values/codes)
            Example:
            
            Error Code Mode:
                Error Code(s): xxxxxxx
            (User can use up and down keys to navigate through them)

        * Clear Error Codes (Clear Code Signal)
            Example:
                Clear Error Code(s)? Y/N
            (Cursor Should blink over the selection preference)

        * Get System info (ISO Standard and Firmware version)
            Example:

            System Info Mode:
                ISO Ver: xxxxxx, Firm. V: xxxxx
### Shut Down Operation: ###
    * Just unplug the damn thing


### OBDII-UART ###

* The UART to OBD-II board uses the following 2 chips:
    * STN1110 for OBD-II, and MCP2551 for CAN
The STN1110 is what we will be interfacing with in order to pull OBDII codes from the car, these are commands that we can extract via the following:
    * The DB9 to OBDII supplies power and serial interfacing from the car to the board, the microcontroller must be wired seperately
    * The OBDII to UART board uses Tx, Rx, and GND which go to the PIC
    * One incredibly important aspect about the OBDII to UART board is that it automatically detects which OBDII protocol to use when it initializes with your car
        * When the car is set to the first position and the OBDII connector is hooked up, the car tells the device what ISO standard it is using, and the device automatically configures itself.
    * The existing documentation uses an FTDI board, which is a serial to USB interface device, the documentation utilizes:
        * BAUD: 9600 bps
        * Data bits: 8
        * Stop bits: 1
        * Parity bits: no parity
        My Assumption is that this UART configuration can be set for the PIC18F46K22 and can therefore read and write to the serial buffer of the STN1110.
        Making it easy to send and recieve commands, essentially we just need to establish a way to send and recieve information using the AT protocol, and then display it on an LCD
    * The PIC must send AT commands, a type of syntax/protocol for the STN1110/ELM327DSI to recieve commands from devices, the list of commands are available [here](https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf)
    
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
* [PIC18F46K22 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F2X-4XK22-Data-Sheet-40001412H.pdf) (Page 259)
