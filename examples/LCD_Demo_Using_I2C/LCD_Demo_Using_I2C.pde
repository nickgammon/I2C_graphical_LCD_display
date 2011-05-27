
// Demo of KS0108B graphics LCD screen connected to MCP23017 16-port I/O expander

// Author: Nick Gammon
// Date: 14 February 2011


#include <Wire.h>
#include <SPI.h>
#include <I2C_graphical_LCD_display.h>

I2C_graphical_LCD_display lcd;

// example bitmap
byte picture [] PROGMEM = {
 0x1C, 0x22, 0x49, 0xA1, 0xA1, 0x49, 0x22, 0x1C,  // face  
 0x10, 0x08, 0x04, 0x62, 0x62, 0x04, 0x08, 0x10,  // star destroyer
 0x4C, 0x52, 0x4C, 0x40, 0x5F, 0x44, 0x4A, 0x51,  // OK logo
};


void setup () 
{
  lcd.begin ();  

  // draw all available letters
  for (int i = ' '; i <= 0x7f; i++)
    lcd.letter (i);

  // black box  
  lcd.clear (6, 40, 30, 63, 0xFF);

  // draw text in inverse
  lcd.gotoxy (40, 40);
  lcd.string ("Nick Gammon.", true);

  // bit blit in a picture
  lcd.gotoxy (40, 56);
  lcd.blit (picture, sizeof picture);
  
  // draw a framed rectangle
  lcd.frameRect (40, 49, 60, 53, 1, 1);

  // draw a white diagonal line
  lcd.line (6, 40, 30, 63, 0);
    
}  // end of setup

void loop () 
{}  // nothing to see here, move along



