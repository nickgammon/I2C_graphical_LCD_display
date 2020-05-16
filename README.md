I2C_graphical_LCD_display
=========================

Arduino library for LCD displays using I2C protocol.


For details about the theory, wiring, schematic, etc. see:

http://www.gammon.com.au/forum/?id=10940

LCD display: KS0108
IO expander: MCP23017 (also supported is the SPI version of the chip: MCP23S17)

Library documentation also on the above web page.

This library also supports the "backpack" adaptor designed by DIY MORE found here:

https://www.diymore.cc/products/diymore-iic-i2c-twi-spi-serial-interface-expanded-board-module-port-for-arduino-uno-r3-1602-lcd-2004-lcd-12864-lcd-display

To enable the library to work with this adaptor, after installing, you must edit I2C_graphical_LCD_display.h and uncomment line 55.
