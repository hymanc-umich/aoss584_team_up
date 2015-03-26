hex:
	avr-gcc -Os -DF_CPU=1000000 -mmcu=atmega48 -c rtftu.c
	avr-gcc -DF_CPU=1000000 -mmcu=atmega48 -o rtftu.elf rtftu.o
	avr-objcopy -O ihex rtftu.elf rtftu.hex
	rm *.o
	rm rtftu.elf
	
flash:
	avrdude -c arduino -p atmega48 -P /dev/ttyUSB0 -b 19200 -U flash:w:rtftu.hex