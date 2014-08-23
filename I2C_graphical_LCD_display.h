/*
 I2C_graphical_LCD_display.h
 
 
 Written by Nick Gammon.
 Date: 14 February 2011.
 
 HISTORY
 
 Version 1.0 : 15 February 2011
 Version 1.1 : 15 February 2011  -- added write-through cache
 Version 1.2 : 19 February 2011  -- allowed for more than 256 bytes in lcd.blit
 Version 1.3 : 21 February 2011  -- swapped some pins around to make it easier to make circuit boards *
 Version 1.4 : 24 February 2011  -- added code to raise reset line properly, also scrolling code *
 Version 1.5 : 28 February 2011  -- added support for SPI interface
 Version 1.6 : 13 March 2011     -- fixed bug in reading data from SPI interface
 Version 1.7 : 13 April 2011     -- made the bitmap for letter "Q" look a bit better
 Version 1.8 : 10 March 2012     -- adapted to work on Arduino IDE 1.0 onwards
 Version 1.9 : 26 May 2012       -- default to I2C rather than SPI on the begin() function
 -- also increased initial LCD_BUSY_DELAY from 20 to 50 uS
 Version 1.10:  8 July 2012      -- fixed issue with dropping enable before reading from display
 Version 1.11: 15 August 2014    -- added support for Print class, and an inverse mode
 
 * These changes required hardware changes to pin configurations
 

 PERMISSION TO DISTRIBUTE
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 and associated documentation files (the "Software"), to deal in the Software without restriction, 
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in 
 all copies or substantial portions of the Software.
 
 
 LIMITATION OF LIABILITY
 
 The software is provided "as is", without warranty of any kind, express or implied, 
 including but not limited to the warranties of merchantability, fitness for a particular 
 purpose and noninfringement. In no event shall the authors or copyright holders be liable 
 for any claim, damages or other liability, whether in an action of contract, 
 tort or otherwise, arising from, out of or in connection with the software 
 or the use or other dealings in the software. 
 
 */


#ifndef I2C_graphical_LCD_display_H
#define I2C_graphical_LCD_display_H

// #define WRITETHROUGH_CACHE

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include <WProgram.h>
#endif

#include <avr/pgmspace.h>

// MCP23017 registers (everything except direction defaults to 0)

#define IODIRA   0x00   // IO direction  (0 = output, 1 = input (Default))
#define IODIRB   0x01
#define IOPOLA   0x02   // IO polarity   (0 = normal, 1 = inverse)
#define IOPOLB   0x03
#define GPINTENA 0x04   // Interrupt on change (0 = disable, 1 = enable)
#define GPINTENB 0x05
#define DEFVALA  0x06   // Default comparison for interrupt on change (interrupts on opposite)
#define DEFVALB  0x07
#define INTCONA  0x08   // Interrupt control (0 = interrupt on change from previous, 1 = interrupt on change from DEFVAL)
#define INTCONB  0x09
#define IOCON    0x0A   // IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
//#define IOCON 0x0B  // same as 0x0A
#define GPPUA    0x0C   // Pull-up resistor (0 = disabled, 1 = enabled)
#define GPPUB    0x0D
#define INFTFA   0x0E   // Interrupt flag (read only) : (0 = no interrupt, 1 = pin caused interrupt)
#define INFTFB   0x0F
#define INTCAPA  0x10   // Interrupt capture (read only) : value of GPIO at time of last interrupt
#define INTCAPB  0x11
#define GPIOA    0x12   // Port value. Write to change, read to obtain value
#define GPIOB    0x13
#define OLLATA   0x14   // Output latch. Write to latch output.
#define OLLATB   0x15



/*

 My mappings of the KS0108 registers:
 

 LCD PIN  MCP23017 PIN  Name   Purpose
 
 ---- Wire these pins together as shown ------
 
 --- Port "A" - control lines
 
  6      28 (GPA7)     E      Enable data transfer on 1 -> 0 transition  (see LCD_ENABLE)
  5      27 (GPA6)     R/~W   1 = read, 0 = write (to LCD) (see LCD_READ)
  4      26 (GPA5)     D/~I   1 = data, 0 = instruction    (see LCD_DATA)
 17      25 (GPA4)     ~RST   1 = not reset, 0 = reset
 16      24 (GPA3)     CS2    Chip select for IC2 (1 = active)  (see LCD_CS2)
 15      23 (GPA2)     CS1    Chip select for IC1 (1 = active)  (see LCD_CS1)
 
 --- Port "B" - data lines
 
  7      1  (GPB0)     DB0    Data bit 0
  8      2  (GPB1)     DB1    Data bit 1
  9      3  (GPB2)     DB2    Data bit 2
 10      4  (GPB3)     DB3    Data bit 3
 11      5  (GPB4)     DB4    Data bit 4
 12      6  (GPB5)     DB5    Data bit 5
 13      7  (GPB6)     DB6    Data bit 6
 14      8  (GPB7)     DB7    Data bit 7
 
 ---- Pins on LCD display which are not connected to the I/O expander ----
 
  1                    GND    LCD logic ground
  2                    +5V    LCD logic power
  3                    V0     Contrast - connect to contrast pot, middle (wiper)
 17                   ~RST    Tie to +5V via 10K resistor (reset signal)
 18                    Vee    Negative voltage - connect to contrast pot, one side *
 19                    A      Power supply for LED light (+5V)  A=anode
 20                    K      GND for LED light                 K=cathode
 
 * Third leg of contrast pot is wired to ground.
 
 ---- Pins on MCP23017 which are not connected to the LCD display ----
 
  9   (VDD)            +5V    Power for MCP23017
 10   (VSS)            GND    Ground for MCP23017
 11   (CS)             SS     (Slave Select) - connect to Arduino pin D10 if using SPI (D53 on the Mega)
 12   (SCL/SCK)        SCL    (Clock) - connect to Arduino pin A5 for I2C (D21 on the Mega) (for SPI SCK: D13, or D52 on the Mega)
 13   (SDA/SI)         SDA    (Data)  - connect to Arduino pin A4 for I2C (D20 on the Mega) (for SPI MOSI: D11, or D51 on the Mega)
 14   (SO)             MISO   (SPI slave out) - connect to Arduino pin D12 if using SPI (D50 on the Mega)
 15   (A0)             Address jumper 0 - connect to ground (unless you want a different address)
 16   (A1)             Address jumper 1 - connect to ground
 17   (A2)             Address jumper 2 - connect to ground
 18   (~RST)           Tie to +5V via 10K resistor (reset signal)
 19   (INTA)           Interrupt for port A (not used)
 20   (INTB)           Interrupt for port B (not used)
 21   (GPA0)           Not used
 22   (GPA1)           Not used
 23   (GPA2)           Not used
 
 */


// GPA port - these show which wires from the LCD are connected to which pins on the I/O expander

#define LCD_CS1    0b00000100   // chip select 1  (pin 23)                            0x04
#define LCD_CS2    0b00001000   // chip select 2  (pin 24)                            0x08
#define LCD_RESET  0b00010000   // reset (pin 25)                                     0x10
#define LCD_DATA   0b00100000   // 1xxxxxxx = data; 0xxxxxxx = instruction  (pin 26)  0x20
#define LCD_READ   0b01000000   // x1xxxxxx = read; x0xxxxxx = write  (pin 27)        0x40
#define LCD_ENABLE 0b10000000   // enable by toggling high/low  (pin 28)              0x80


// Commands sent when LCD in "instruction" mode (LCD_DATA bit set to 0)

#define LCD_ON          0x3F
#define LCD_OFF         0x3E
#define LCD_SET_ADD     0x40   // plus X address (0 to 63) 
#define LCD_SET_PAGE    0xB8   // plus Y address (0 to 7)
#define LCD_DISP_START  0xC0   // plus X address (0 to 63) - for scrolling

class I2C_graphical_LCD_display : public Print
{
private:
  
  byte _chipSelect;  // currently-selected chip (LCD_CS1 or LCD_CS2)
  byte _lcdx;        // current x position (0 - 127)
  byte _lcdy;        // current y position (0 - 63)
  
  byte _port;        // port that the MCP23017 is on (should be 0x20 to 0x27)
  byte _ssPin;       // if non-zero use SPI rather than I2C (and this is the SS pin)

  void expanderWrite (const byte reg, const byte data);
  byte readData ();
  void startSend ();    // prepare for sending to MCP23017  (eg. set SS low)
  void doSend (const byte what);  // send a byte to the MCP23017
  void endSend ();      // finished sending  (eg. set SS high)

  boolean _invmode;
  
  
#ifdef WRITETHROUGH_CACHE
  byte _cache [64 * 128 / 8];
  int  _cacheOffset;
#endif
  
public:
  
  // constructor
  I2C_graphical_LCD_display () : _port (0x20), _ssPin (10), _invmode(false) {};
  
  void begin (const byte port = 0x20, const byte i2cAddress = 0, const byte ssPin = 0);
  void cmd (const byte data);
  void gotoxy (byte x, byte y);
  void writeData (byte data, const boolean inv);
  void writeData (byte data) { writeData(data, _invmode);}
  void letter (byte c, const boolean inv);
  void letter (byte c) {letter(c, _invmode);}
  void string (const char * s, const boolean inv);
  void string (const char * s) {string(s, _invmode);}
  void blit (const byte * pic, const unsigned int size);
  void clear (const byte x1 = 0,    // start pixel
              const byte y1 = 0,     
              const byte x2 = 127,  // end pixel
              const byte y2 = 63,   
              const byte val = 0);   // what to fill with 
  void setPixel (const byte x, const byte y, const byte val = 1);
  void fillRect (const byte x1 = 0,   // start pixel
                const byte y1 = 0,     
                const byte x2 = 127, // end pixel
                const byte y2 = 63,    
                const byte val = 1);  // what to draw (0 = white, 1 = black) 
  void frameRect (const byte x1 = 0,    // start pixel
                 const byte y1 = 0,     
                 const byte x2 = 127, // end pixel
                 const byte y2 = 63,    
                 const byte val = 1,    // what to draw (0 = white, 1 = black) 
                 const byte width = 1);
  void line  (const byte x1 = 0,    // start pixel
              const byte y1 = 0,     
              const byte x2 = 127,  // end pixel
              const byte y2 = 63,   
              const byte val = 1);  // what to draw (0 = white, 1 = black) 
  void scroll (const byte y = 0);   // set scroll position

#if defined(ARDUINO) && ARDUINO >= 100
	size_t write(uint8_t c) {letter(c, _invmode); return 1; }
#else
	void write(uint8_t c) { letter(c, _invmode); }
#endif

  void setInv(boolean inv) {_invmode = inv;} // set inverse mode state true == inverse
  
};

#endif  // I2C_graphical_LCD_display_H


