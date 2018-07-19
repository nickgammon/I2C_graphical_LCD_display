
// Demo of KS0108B graphics LCD screen connected to MCP23017 16-port I/O expander

// Author: skv
// Date: 19.07.2018


#include <Wire.h>
#include <SPI.h>
#include <I2C_graphical_LCD_display.h>

I2C_graphical_LCD_display lcd;

void setup () 
{
  lcd.begin ();  
  lcd.lighton();
  // black box  
  lcd.clear (0, 0, 63, 63, 0xFF);
  lcd.clear (64, 0, 127, 63, 0x00);
}  // end of setup

char pix = 0B00110011;
boolean light = true;

void loop () 
{
byte x;
byte y;

x = random(128);
y = random(64);
lcd.setPixel(x, y, lcd.getPixel(x, y)^1);
//delay(100);
}  // nothing to see here, move along



