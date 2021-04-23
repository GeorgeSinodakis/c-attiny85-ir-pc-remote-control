DEVICE     = attiny85
FUSES      = -U lfuse:w:0xc1:m -U hfuse:w:0xcf:m -U efuse:w:0xff:m
FILENAME   = main.c
FLAGS	   = -Wall -Wl,-gc-sections -Wl,-relax -g -mcall-prologues -O3 -mmcu=$(DEVICE)
INCLUDE	   = C:\DEV\AVR\atmega328p\libraries

default: build upload clean

build:
	@avr-gcc $(FLAGS) -I $(INCLUDE) $(FILENAME) -o firmware.elf 
	@avr-objcopy -j .text -j .data -O ihex firmware.elf firmware.hex 
	@avr-size --format=avr --mcu=$(DEVICE) firmware.elf

dump:
	avr-objdump -h -S firmware.elf > firmware.lss

upload:
	avrdude -e -F -v -p $(DEVICE) -c usbasp-clone $(FUSES) -D -U flash:w:firmware.hex:i 
	
clean:
	@-del -f firmware.*
