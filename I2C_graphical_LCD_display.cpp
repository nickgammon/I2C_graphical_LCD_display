/*
 I2C_graphical_LCD_display.cpp
 
 
 Written by Nick Gammon
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


#include "I2C_graphical_LCD_display.h"

// WARNING: Put this following line into your main program or you will get compiler errors:
#include <Wire.h>
#include <SPI.h>

// SPI is so fast we need to give the LCD time to catch up.
// This is the number of microseconds we wait. Something like 20 to 50 is probably reasonable.
//  Increase this value if the display is either not working, or losing data.

#define LCD_BUSY_DELAY 50   // microseconds

// font data - each character is 8 pixels deep and 5 pixels wide

byte font [96] [5] PROGMEM = {
  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // space  (0x20)
  { 0x00, 0x00, 0x2F, 0x00, 0x00 }, // !
  { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
  { 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // #
  { 0x24, 0x2A, 0x7F, 0x2A, 0x12 }, // $
  { 0x23, 0x13, 0x08, 0x64, 0x62 }, // %
  { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
  { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
  { 0x00, 0x1C, 0x22, 0x41, 0x00 }, // (
  { 0x00, 0x41, 0x22, 0x1C, 0x00 }, // (
  { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // *
  { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
  { 0x00, 0x50, 0x30, 0x00, 0x00 }, // ,
  { 0x08, 0x08, 0x08, 0x08, 0x08 }, // -
  { 0x00, 0x30, 0x30, 0x00, 0x00 }, // .
  { 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
   
  { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0  (0x30)
  { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
  { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
  { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
  { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4
  { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
  { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
  { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
  { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
  { 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
  { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
  { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
  { 0x08, 0x14, 0x22, 0x41, 0x00 }, // <
  { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
  { 0x00, 0x41, 0x22, 0x14, 0x08 }, // >
  { 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
  
  { 0x32, 0x49, 0x79, 0x41, 0x3E }, // @  (0x40)
  { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
  { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
  { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
  { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
  { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
  { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // F
  { 0x3E, 0x41, 0x49, 0x49, 0x7A }, // G
  { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
  { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
  { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
  { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
  { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
  { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // M
  { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
  { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
  
  { 0x3F, 0x09, 0x09, 0x09, 0x06 }, // P  (0x50)
  { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
  { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // R
  { 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
  { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
  { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
  { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
  { 0x3F, 0x40, 0x30, 0x40, 0x3F }, // W
  { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
  { 0x07, 0x08, 0x70, 0x08, 0x07 }, // Y
  { 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
  { 0x00, 0x7F, 0x41, 0x41, 0x00 }, // [
  { 0x02, 0x04, 0x08, 0x10, 0x20 }, // backslash
  { 0x00, 0x41, 0x41, 0x7F, 0x00 }, // ]
  { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
  { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
  
  { 0x00, 0x01, 0x02, 0x04, 0x00 }, // `  (0x60)
  { 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
  { 0x7F, 0x50, 0x48, 0x48, 0x30 }, // b
  { 0x38, 0x44, 0x44, 0x44, 0x20 }, // c
  { 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
  { 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
  { 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f
  { 0x0C, 0x52, 0x52, 0x52, 0x3E }, // g
  { 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
  { 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
  { 0x20, 0x40, 0x44, 0x3D, 0x00 }, // j
  { 0x7F, 0x10, 0x28, 0x44, 0x00 }, // k
  { 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
  { 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
  { 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
  { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
  
  { 0x7C, 0x14, 0x14, 0x14, 0x08 }, // p  (0x70)
  { 0x08, 0x14, 0x14, 0x08, 0x7C }, // q
  { 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
  { 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
  { 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
  { 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
  { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
  { 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
  { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
  { 0x0C, 0x50, 0x50, 0x50, 0x3C }, // y
  { 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
  { 0x00, 0x08, 0x36, 0x41, 0x00 }, // {
  { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
  { 0x00, 0x41, 0x36, 0x08, 0x00 }, // }
  { 0x30, 0x08, 0x10, 0x20, 0x18 }, // ~
  { 0x7F, 0x55, 0x49, 0x55, 0x7F }  // unknown char (0x7F)
  
};

// glue routines for version 1.0+ of the IDE
uint8_t i2c_read ()
{
#if defined(ARDUINO) && ARDUINO >= 100
  return Wire.read ();
#else
  return Wire.receive ();
#endif
} // end of Nunchuk::i2c_read

void i2c_write (int data)
{
#if defined(ARDUINO) && ARDUINO >= 100
  Wire.write (data);
#else
  Wire.send (data);
#endif
} // end of Nunchuk::i2c_write


// prepare for sending to MCP23017 
void I2C_graphical_LCD_display::startSend ()   
{
  
  if (_ssPin)
    {
    delayMicroseconds (LCD_BUSY_DELAY);
    digitalWrite (_ssPin, LOW); 
    SPI.transfer (_port << 1);
    }
  else
    Wire.beginTransmission (_port);
  
}  // end of I2C_graphical_LCD_display::startSend

// send a byte via SPI or I2C
void I2C_graphical_LCD_display::doSend (const byte what)   
{
  if (_ssPin)
    SPI.transfer (what);
  else
    i2c_write (what);
}  // end of I2C_graphical_LCD_display::doSend

// finish sending to MCP23017 
void I2C_graphical_LCD_display::endSend ()   
{
  if (_ssPin)
    digitalWrite (_ssPin, HIGH); 
  else
    Wire.endTransmission ();
 
}  // end of I2C_graphical_LCD_display::endSend


// set up - call before using
// specify 
//  * the port that the MCP23017 is on (default 0x20)
//  * the i2c port (default 0)
//  * the SPI SS (slave select) pin - leave as default of zero for I2C operation

// turns LCD on, clears memory, sets the cursor to 0,0

// Approx time to run: 600 ms on Arduino Uno
void I2C_graphical_LCD_display::begin (const byte port, 
                                       const byte i2cAddress,
                                       const byte ssPin)
{
  
  _port = port;   // remember port
  _ssPin = ssPin; // and SPI slave select pin
  
  if (_ssPin)
    SPI.begin ();
  else
    Wire.begin (i2cAddress);   

// un-comment next line for faster I2C communications:
//   TWBR = 12;

  // byte mode (not sequential)
  expanderWrite (IOCON, 0b00100000);
  
  // all pins as outputs
  expanderWrite (IODIRA, 0);
  expanderWrite (IODIRB, 0);
  
  // take reset line low
  startSend ();
    doSend (GPIOA);
    doSend (0); // all lines low
  endSend ();
  
  // now raise reset (and enable) line and wait briefly
  expanderWrite (GPIOA, LCD_ENABLE | LCD_RESET);
  delay (1);
  
  // turn LCD chip 1 on
  _chipSelect = LCD_CS1;
  cmd (LCD_ON);

  // turn LCD chip 2 on
  _chipSelect = LCD_CS2;
  cmd (LCD_ON);
  
  // clear entire LCD display
  clear ();
  
  // and put the cursor in the top-left corner
  gotoxy (0, 0);
  
  // ensure scroll is set to zero
  scroll (0);   
  
}  // end of I2C_graphical_LCD_display::I2C_graphical_LCD_display (constructor)


// send command to LCD display (chip 1 or 2 as in chipSelect variable)
// for example, setting page (Y) or address (X)
void I2C_graphical_LCD_display::cmd (const byte data)
{
  startSend ();
    doSend (GPIOA);                      // control port
    doSend (LCD_RESET | LCD_ENABLE | _chipSelect);   // set enable high (D/I is low meaning instruction) 
    doSend (data);                       // (command written to GPIOB)
    doSend (LCD_RESET | _chipSelect);    // (GPIOA again) pull enable low to toggle data 
  endSend ();
} // end of I2C_graphical_LCD_display::cmd 

// set our "cursor" to the x/y position
// works out whether this refers to chip 1 or chip 2 and sets chipSelect appropriately

// Approx time to run: 33 ms on Arduino Uno
void I2C_graphical_LCD_display::gotoxy (byte x, 
                                        byte y)
{
  
  if (x > 127) 
    x = 0;                
  if (y > 63)  
    y = 0;
  
#ifdef WRITETHROUGH_CACHE
  _cacheOffset = 0;
#endif
  
  // work out which chip
  if (x >= 64)
    {
    x -= 64;  
    _chipSelect = LCD_CS2;
#ifdef WRITETHROUGH_CACHE
    _cacheOffset = 64 * 64 / 8;  // half-way through cache
#endif
      
    }
  else
    _chipSelect = LCD_CS1;
  
  // remember for incrementing later
  _lcdx = x;
  _lcdy = y;
  
  // command LCD to the correct page and address
  cmd (LCD_SET_PAGE | (y >> 3) );  // 8 pixels to a page
  cmd (LCD_SET_ADD  | x );          
  
#ifdef WRITETHROUGH_CACHE
  _cacheOffset += (x << 3) | y >> 3;
#endif  
}  // end of I2C_graphical_LCD_display::gotoxy 


// set register "reg" on expander to "data"
// for example, IO direction
void I2C_graphical_LCD_display::expanderWrite (const byte reg, 
                                               const byte data ) 
{
  startSend ();
    doSend (reg);
    doSend (data);
  endSend ();
} // end of I2C_graphical_LCD_display::expanderWrite

// read the byte corresponding to the selected x,y position
byte I2C_graphical_LCD_display::I2C_graphical_LCD_display::readData ()
{
  
#ifdef WRITETHROUGH_CACHE
  return _cache [_cacheOffset];
#endif
  
  // data port (on the MCP23017) is now input
  expanderWrite (IODIRB, 0xFF);
  
  // lol, see the KS0108 spec sheet - you need to read twice to get the data
  for (int i = 0; i < 2; i++)
    {
    startSend ();
      doSend (GPIOA);                  // control port
      doSend (LCD_RESET | LCD_READ | LCD_DATA | LCD_ENABLE | _chipSelect);  // set enable high 
    endSend ();

    startSend ();
      doSend (GPIOA);                  // control port
      doSend (LCD_RESET | LCD_READ | LCD_DATA | _chipSelect);  // pull enable low to toggle data 
    endSend ();
    }
  
  byte data;

  if (_ssPin)
    {
    digitalWrite (_ssPin, LOW); 
    SPI.transfer ((_port << 1) | 1);  // read operation has low-bit set
    SPI.transfer (GPIOB);             // which register to read from
    data = SPI.transfer (0);          // get byte back
    digitalWrite (_ssPin, HIGH); 
    }
  else
    {
    // initiate blocking read into internal buffer
    Wire.requestFrom (_port, (byte) 1);
    
    // don't bother checking if available, Wire.receive does that anyway
    //  also it returns 0x00 if nothing there, so we don't need to bother doing that
    data = i2c_read ();
    }  
  
  // data port (on the MCP23017) is now output again
  expanderWrite (IODIRB, 0);
  
  return data;
  
}  // end of I2C_graphical_LCD_display::readData

// write a byte to the LCD display at the selected x,y position
// if inv true, invert the data
// writing advances the cursor 1 pixel to the right
// it wraps to the next "line" if necessary (a line is 8 pixels deep)
void I2C_graphical_LCD_display::writeData (byte data, 
                                           const boolean inv)
{
  
  // invert data to be written if wanted
  if (inv)
    data ^= 0xFF;
  
  // note that the MCP23017 automatically toggles between port A and port B
  // so the four sends do this:
  //   1. Choose initial port as GPIOA (general IO port A)
  //   2. Port A: set E high
  //   3. Port B: send the data byte
  //   4. Port A: set E low to toggle the transfer of data

  startSend ();
    doSend (GPIOA);                  // control port
    doSend (LCD_RESET | LCD_DATA | LCD_ENABLE | _chipSelect);  // set enable high 
    doSend (data);                   // (screen data written to GPIOB)
    doSend (LCD_RESET | LCD_DATA | _chipSelect);  // (GPIOA again) pull enable low to toggle data 
  endSend ();

#ifdef WRITETHROUGH_CACHE
  _cache [_cacheOffset] = data;
#endif 
  
  // we have now moved right one pixel (in the LCD hardware)
  _lcdx++;

  
  // see if we moved from chip 1 to chip 2, or wrapped at end of line
  if (_lcdx >= 64)
    {
    if (_chipSelect == LCD_CS1)  // on chip 1, move to chip 2
      gotoxy (64, _lcdy);
    else
      gotoxy (0, _lcdy + 8);  // go back to chip 1, down one line
    }  // if >= 64
  else
    {
#ifdef WRITETHROUGH_CACHE
    _cacheOffset += 8;
#endif
    }
  
}  // end of I2C_graphical_LCD_display::writeData


// write one letter (space to 0x7F), inverted or normal

// Approx time to run: 4 ms on Arduino Uno
void I2C_graphical_LCD_display::letter (byte c, 
                                        const boolean inv)
{
  if (c < 0x20 || c > 0x7F)
    c = 0x7F;  // unknown glyph
  
  c -= 0x20; // force into range of our font table (which starts at 0x20)
  
  // no room for a whole character? drop down a line
  // letters are 5 wide, so once we are past 59, there isn't room before we hit 63
  if (_lcdx >= 60 && _chipSelect == LCD_CS2)
    gotoxy (0, _lcdy + 8);
  
  // font data is in PROGMEM memory (firmware)
  for (byte x = 0; x < 5; x++)
    writeData (pgm_read_byte (&font [c] [x]), inv);
  writeData (0, inv);  // one-pixel gap between letters
  
}  // end of I2C_graphical_LCD_display::letter

// write an entire null-terminated string to the LCD: inverted or normal
void I2C_graphical_LCD_display::string (const char * s, 
                                        const boolean inv)
{
  char c;
  while (c = *s++)
    letter (c, inv); 
}  // end of I2C_graphical_LCD_display::string

// blits (copies) a series of bytes to the LCD display from an array in PROGMEM

// Approx time to run: 2 ms/byte on Arduino Uno
void I2C_graphical_LCD_display::blit (const byte * pic, 
                                      const unsigned int size)
{
  for (unsigned int x = 0; x < size; x++, pic++)
    writeData (pgm_read_byte (pic));
}  // end of I2C_graphical_LCD_display::blit

// clear rectangle x1,y1,x2,y2 (inclusive) to val (eg. 0x00 for black, 0xFF for white)
// default is entire screen to black
// rectangle is forced to nearest (lower) 8 pixels vertically
// this if faster than lcd_fill_rect because it doesn't read from the display

// Approx time to run: 120 ms on Arduino Uno for 20 x 50 pixel rectangle
void I2C_graphical_LCD_display::clear (const byte x1,    // start pixel
                                       const byte y1,     
                                       const byte x2,  // end pixel
                                       const byte y2,   
                                       const byte val)   // what to fill with 
{
  for (byte y = y1; y <= y2; y += 8)
    {
    gotoxy (x1, y);
    for (byte x = x1; x <= x2; x++)
      writeData (val);
    } // end of for y
  
  gotoxy (x1, y1);
} // end of I2C_graphical_LCD_display::clear

// set or clear a pixel at x,y
// warning: this is slow because we have to read the existing pixel in from the LCD display
// so we can change a single bit in it
void I2C_graphical_LCD_display::setPixel (const byte x, 
                                          const byte y, 
                                          const byte val)
{
  // select appropriate page and byte
  gotoxy (x, y);
  
  // get existing pixel values
  byte c = readData ();
  
  // toggle or clear this particular one as required
  if (val)
    c |=   1 << (y & 7);    // set pixel
  else
    c &= ~(1 << (y & 7));   // clear pixel
  
  // go back to that place
  gotoxy (x, y);
  
  // write changed data back
  writeData (c);
  
}  // end of I2C_graphical_LCD_display::setPixel

// fill the rectangle x1,y1,x2,y2 (inclusive) with black (1) or white (0)
// if possible use lcd_clear instead because it is much faster
// however lcd_clear clears batches of 8 vertical pixels

// Approx time to run: 5230 ms on Arduino Uno for 20 x 50 pixel rectangle
//    (Yep, that's over 5 seconds!)
void I2C_graphical_LCD_display::fillRect (const byte x1, // start pixel
                                          const byte y1,     
                                          const byte x2, // end pixel
                                          const byte y2,    
                                          const byte val)  // what to draw (0 = white, 1 = black) 
{
  for (byte y = y1; y <= y2; y++)
    for (byte x = x1; x <= x2; x++)
      setPixel (x, y, val);
}  // end of I2C_graphical_LCD_display::fillRect

// frame the rectangle x1,y1,x2,y2 (inclusive) with black (1) or white (0)
// width is width of frame, frames grow inwards

// Approx time to run:  730 ms on Arduino Uno for 20 x 50 pixel rectangle with 1-pixel wide border
//             1430 ms on Arduino Uno for 20 x 50 pixel rectangle with 2-pixel wide border
void I2C_graphical_LCD_display::frameRect (const byte x1, // start pixel
                                           const byte y1,     
                                           const byte x2, // end pixel
                                           const byte y2,    
                                           const byte val,    // what to draw (0 = white, 1 = black) 
                                           const byte width)
{
  byte x, y, i;
  
  // top line
  for (x = x1; x <= x2; x++)
    for (i = 0; i < width; i++)
      setPixel (x, y1 + i, val);
  
  // bottom line
  for (x = x1; x <= x2; x++)
    for (i = 0; i < width; i++)
      setPixel (x, y2 - i, val);
  
  // left line
  for (y = y1; y <= y2; y++)
    for (i = 0; i < width; i++)
      setPixel (x1 + i, y, val);
  
  // right line
  for (y = y1; y <= y2; y++)
    for (i = 0; i < width; i++)
      setPixel (x2 - i, y, val);
  
}  // end of I2C_graphical_LCD_display::frameRect

// draw a line from x1,y1 to x2,y2 (inclusive) with black (1) or white (0)
// Warning: fairly slow, as is anything that draws individual pixels
void I2C_graphical_LCD_display::line  (const byte x1,  // start pixel
                                       const byte y1,     
                                       const byte x2,  // end pixel
                                       const byte y2,   
                                       const byte val)  // what to draw (0 = white, 1 = black) 
{
  byte x, y;
  
  // vertical line? do quick way
  if (x1 == x2)
    {
    for (y = y1; y <= y2; y++)
      setPixel (x1, y, val);
    return;
  }
  
  // horizontal line? do quick way
  if (y1 == y2)
    { 
    for (x = x1; x <= x2; x++)
      setPixel (x, y1, val);
    return;
    }
  
  int x_diff = x2 - x1,
      y_diff = y2 - y1;
  
  // if x difference > y difference, draw every x pixels
  if (abs(x_diff) > abs (y_diff))
  {
    int x_inc = 1;
    int y_inc = (y_diff << 8) / x_diff;
    int y_temp = y1 << 8;
    if (x_diff < 0)
      x_inc = -1;
    
    for (x = x1; x != x2; x += x_inc)
      {
      setPixel (x, y_temp >> 8, val);
      y_temp += y_inc;
      }
    
    return;
  }

  // otherwise draw every y pixels
  int x_inc = (x_diff << 8) / y_diff;
  int y_inc = 1;
  if (y_diff < 0)
    y_inc = -1;
  
  int x_temp = x1 << 8;
  for (y = y1; y != y2; y += y_inc)
  {
    setPixel (x_temp >> 8, y, val);
    x_temp += x_inc;
  }
  
  
} // end of I2C_graphical_LCD_display::line

// set scroll position to y
void I2C_graphical_LCD_display::scroll (const byte y)   // set scroll position
{
  byte old_cs = _chipSelect;
  _chipSelect = LCD_CS1;
  cmd (LCD_DISP_START | (y & 0x3F) );  // set scroll position
  _chipSelect = LCD_CS2;
  cmd (LCD_DISP_START | (y & 0x3F) );  // set scroll position
  _chipSelect = old_cs;
} // end of I2C_graphical_LCD_display::scroll
