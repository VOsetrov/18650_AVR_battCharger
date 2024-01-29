TARGET = arkControl 
SRC = main
CC = avr-gcc
CPU_Freq = F_CPU=16000000UL
CFLAG_1 = -Wall -Og
CFLAG_2 = -Os
LDFLAG = -mmcu=atmega328p -ffreestanding 
USB_PORT = /dev/ttyUSB0
 
$(SRC).hex: $(TARGET)
	avr-objcopy -O ihex -R .eeprom $(TARGET) $(SRC).hex 

$(TARGET): $(SRC).o
	$(CC) $(CFLAG_1) -g $(SRC).o -o $(TARGET)

$(SRC).o: $(SRC).c
	$(CC) $(CFLAG_1) $(LDFLAG) -D${CPU_Freq} -c $(SRC).c

deploy:	$(SRC).hex
	avrdude -F -V -c arduino -pATMEGA328p -P $(USB_PORT) -b 115200 -U flash:w:$(SRC).hex	

clean:
	rm $(TARGET) *.o *.hex

