# ECE 3301 Final Project: OBDII Scanner using the PIC18F46K22 #
### By Luca Lanzillotta and Zihong Zheng ###
*Abstract:*
* [*Link to original Cornell Project*](https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/s2012/ppv5/index.html)
* Create an OBD II scanner that is compatible with any ISO OBD II standard to read basic vehicle telemetry and diagnostics. The design will utilize a Sparkfun OBD II to UART intpreter board and will utilize half duplex capabilities to send requests from the PIC18F46K22 and receive data to display on an LCD. Additionally the board will include buttons to navigate a menu to send and receive specific requests such as live data, diagnostic codes, clear diagnostic codes, and system information relating to the ELM327, which will all be displayable on an 2x16 LCD.

### Assignment Requirements: ###
* Use serial communication -> UART
* Use CCP register -> Using in compare mode
* Use TMR register -> Using as a value to compare to
* Use Interrupts -> Using an ISR to read UART

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

# Design Description: #
## System Block Diagram: ##

![System Block Diagram](Images/OBDII_UARTBlockDiagram.png)

## System State Diagram: ## 

![System State Diagram](images/SystemStateDiagram.png)

## System Schematic: ##

![OBDIIPIC Schematic](images/OBDIIPIC_Schematic.png)

## Startup Sequence: ##

### Welcome Screen ###
* The welcome screen is not only a splash screen for the user to observe, but also provides as an error checking mechanism for the proper connection to your vehicles OBDII port. The splash screen itself has a countdown timer that is setup with the CCP register of the PIC.
When the splash screen is displayed it checks if the OBDII protocol is detected, in the case it is not, it notifies the user and stalls at the welcome screen until the user plugs in the device. When the user plugs in the device in this state, it initiates a timer and displays a connected message. Below are the cases of which occur when the user starts the PIC with an OBDII device connected, without an OBDII device connected, and what occurs when the user plugs in after the PIC detects no device is connected.

Note: C is the counter for the countdown
Welcome Splash Modes:
* OBDII is plugged in:

        <<< OBDIIPIC >>>
        <<<< V1.0 >>>> C

* OBDII is not plugged in:
   
        <<< OBDIIPIC >>>
        OBDII Not Found 

* OBDII is not plugged in, then connected:

        OBDII Detected
        ==============

## Standard Operation Mode: ##

* The OBDII_PIC is a user friendly device, because of this it was important to include essential buttons to the functionality of any configurable embedded device, it was for this reason that the OBDIIPIC includes a main menu and intuitive tools to navigate it using with a scroll wheel and enter and back buttons. The scroll wheel is a simple potentiometer configured using the PIC's ADC, which displays a flashing cursor relative to its rotational position. The menu includes four modes that any OBDII device should have, the first is the Live Reading mode, written LR to conserve LCD space, this mode displays the current Engine RPM, Battery Voltage, and Air Intake Temperature. The mode also includes a parsing icon which notifies the user that information is being requested and displayed. The second mode is by far the most useful and challenging to program, as it interacts directly with the vehicles ECU when requesting OBDII PIDs. The second mode is the Diagnostic Trouble Codes, DTC for short. This mode allows the user to request OBDII PIDs using the 03 PID request that is standardized for OBDII. Upon requesting the diagnostic codes, the PIC recieves two bytes of data per code and displays them on the LCD. In the case where there are more than three diagnostic codes, the PIC must detect a shift in transmission protocol and adjust the reception of the data to be legible. Additionally, if there is more data than the LCD can display, the user can scroll through the diagnostic codes utilizing the embedded potentiometer. Next up is the third and most powerful mode; the Clear Diagnostic Codes, or CDC for short allows the user to clear any error codes generated from the ECU of the vehicle, this of course may be overwritten the time the vehicle starts, or if an error code is generated during normal operation. The mode allows for a Y/N style menu and returns to the main menu after either option is selected, which before returning indicates the request was either allowed or cancelled to confirm the users decision. Lastly is the Display System Information mode, or DSI for short. This mode tells the user which ELM protocol the OBDII to UART IC is using, in this case the current firmware version is 1.4b, and also notifies the user which SAE version the vehicle being used uses.
Due to the ease of use, if the user wishes to exit any mode at any instant, the back button provides itself as function to reset the PIC to the main menu to select another mode.
        
### Main Menu: ###

        MENU  <OBDIIPIC>
        LRM RDC CDC DSI
    

### Live Reading mode: ###
* Display RPM, Battery Voltage, and Air Intake Temperature
        
        RPM  VBatt AIT    
        0000 0.0V  ##C

### Read Diagnostic Codes: ###
* Display OBDII PID Error Codes 
* Request an "03" OBDII service code, The ECU will return the DTCs in hex encoded format, which can be broken into:
1st character:
P = Powertrain
C = Chassis
B = Body
U = Network

2nd character:
0 – Indicates a generic (SAE defined) code
1 – Indicates a manufacturer-specific (OEM) code
2 – Category dependent:
For the 'P' category this indicates a generic (SAE defined) code
For other categories indicates a manufacturer-specific (OEM) code
3 – Category dependent:
For the 'P' category this is indicates a code that has been 'jointly' defined
For other categories this has been reserved for future use

3rd character: Denotes a particular vehicle system that the fault relates to

0 – Fuel and air metering and auxiliary emission controls
1 – Fuel and air metering
2 – Fuel and air metering (injector circuit)
3 – Ignition systems or misfires
4 – Auxiliary emission controls
5 – Vehicle speed control and idle control systems
6 – Computer and output circuit
7 – Transmission
8 – Transmission
A-F – Hybrid Trouble Codes

Finally the fourth and fifth characters define the exact problem detected.

### Clear Diagnostic Codes: ###
* Send the "04" OBDII PID request if the user selects "Y"

        Clear Error Code(s)
        Y/N <<<<<<<<<<<<<<< 

* User selects "N"

        Nothing Cleared
        To Menu...

* User selects "Y"

        Codes Cleared
        To Menu...
        
### Display System Information: ###
* Displays ELM firmware version and ECU SAE type
        
        OS: ELM xxxxx
        SAE: ISO xxxx


## Shut Down Operation: ##
* Because the PIC is USB powered on the curiosity board, which is the method I was using to develop, there isnt really a need for a power on/off mode of any kind, however, what is entirely possible is configuring a DC/DC buck converter to step down the 12V to the OBDII-UART device from the Vbatt line and route it to a switch to trigger the PIC itself as either on or off. In my current setup I am just using the USB cable for programming and debugging.


## OBDII-UART Notes: ##

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
    
# Reference Documents and Useful Links: #

## OBDII-UART Documentation: ##
* [Original Project Documentation - OBDII Data Logger Cornell University Project](https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/s2012/ppv5/index.html)
* [Sparkfun OBD-II to UART board](https://www.sparkfun.com/sparkfun-obd-ii-uart.html)
* [Sparkfun OBD-II to UART board schematic](https://cdn.sparkfun.com/assets/2/4/d/c/d/520ab4c5757b7f5e0acc8c0e.pdf)
* [Sparkfun OBD-II to UART Implementation Guide](https://learn.sparkfun.com/tutorials/obd-ii-uart-hookup-guide/all)
* [OBD to RS232 Interpreter](https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf)
* [OBD Protocol Information](https://en.wikipedia.org/wiki/On-board_diagnostics#Standard_interfaces)
* [OBD PID Information](https://en.wikipedia.org/wiki/OBD-II_PIDs)
* [Display Hookup Guide](https://learn.sparkfun.com/tutorials/basic-character-lcd-hookup-guide)
* [Github Syntax and Formatting](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax)

## PIC18F46K22 Documentation: ##
* [PIC18F46K22 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F2X-4XK22-Data-Sheet-40001412H.pdf)
