# avrdude

## Upload hex to ATmega328P

>avrdude -p m328p -P /dev/ttyUSB0 -c avrisp -b 19200 -U flash:w:graphicstest_for_atmega328p.ino.arduino_standard.hex  
