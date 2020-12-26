miscArduino
===========

Some arduino code


avrdude
==========
Uploading
----------
Upload to ATmega2560  
>$ avrdude avrdude -v -q -p m2560 -c wiring -P /dev/ttyUSB0 -D -U flash:w:/path/to/image.hex:i

Upload to ATmega328 (Uno etc.)  
>$ avrdude -p m328p -c usbtiny -P /dev/ttyUSB0 -U flash:w:firmware.hex

Programmer / Chip Verification
----------
>$ avrdude -c usbtiny -p m168

If you see a 'Reading' progress bar, a device signature, and some other text, your settings are correct


Cheat sheets
----------
[https://github.com/amirbawab/AVR-cheat-sheet](https://github.com/amirbawab/AVR-cheat-sheet) 