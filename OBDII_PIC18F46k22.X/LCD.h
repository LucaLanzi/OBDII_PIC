
#ifndef LCD_H
    #define	LCD_H


    /**************************************************************************/
    /*************** Update the definitions below accordingly *****************/
    /**************************************************************************/
    
    #define LCD_RS_TRIS TRISDbits.TRISD2  //TRIS bit of the pin connected to "RS"
    #define LCD_EN_TRIS TRISDbits.TRISD3  //TRIS bit of the pin connected to "EN"
    #define LCD_D4_TRIS TRISDbits.TRISD4  //TRIS bit of the pin connected to "D4"
    #define LCD_D5_TRIS TRISDbits.TRISD5  //TRIS bit of the pin connected to "D5"
    #define LCD_D6_TRIS TRISDbits.TRISD6  //TRIS bit of the pin connected to "D6"
    #define LCD_D7_TRIS TRISDbits.TRISD7  //TRIS bit of the pin connected to "D7"

    #define LCD_RS_LAT LATDbits.LATD2  //LAT bit of the pin connected to "RS"
    #define LCD_EN_LAT LATDbits.LATD3  //LAT bit of the pin connected to "EN"
    #define LCD_D4_LAT LATDbits.LATD4  //LAT bit of the pin connected to "D4"
    #define LCD_D5_LAT LATDbits.LATD5  //LAT bit of the pin connected to "D5"
    #define LCD_D6_LAT LATDbits.LATD6  //LAT bit of the pin connected to "D6"
    #define LCD_D7_LAT LATDbits.LATD7  //LAT bit of the pin connected to "D7"

    #define _XTAL_FREQ 16000000  //Frequency of the sytem clock

    /**************************************************************************/
    /*************************** Necessary includes ***************************/
    /**************************************************************************/

    #include <xc.h>
    #include <stdint.h>
    
    /**************************************************************************/
    /*************************** Function prototypes **************************/
    /**************************************************************************/

    void LCD_init(void);
    void LCD_cursor_set(uint8_t, uint8_t);
    void LCD_cursor_increment(void);
    void LCD_cursor_decrement(void);
    void LCD_configure_lcd_display(uint8_t);
    void LCD_configure_cursor_display(uint8_t);
    void LCD_configure_cursor_blink(uint8_t);
    void LCD_clear(void);
    void LCD_shift_reset(void);
    void LCD_shift_left(void);
    void LCD_shift_right(void);    
    void LCD_configure_entry_id(uint8_t);
    void LCD_configure_entry_shift(uint8_t);
    void LCD_add_character(char*, uint8_t);
    
    void LCD_write_string(char*);
    void LCD_write_char(char);
    void LCD_write_variable(int32_t, uint8_t);
    void LCD_write_float(float, uint8_t, uint8_t);
    
    
#endif	/* LCD_H */