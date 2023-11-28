TARGET = battCharger
SRC = main
CC = avr-gcc
CPU_Freq = F_CPU=16000000
CFLAG_1 = -Wall -Og -mmcu=atmega328p 
CFLAG_2 = -Os
CFLAG_3 = -std=gnu99
LDFLAG = 
USB_PORT = /dev/ttyUSB0
 
$(SRC).hex: $(TARGET)
	avr-objcopy -O ihex -R .eeprom $(TARGET) $(SRC).hex 

$(TARGET): $(SRC).o
	$(CC) $(CFLAG_1) $(SRC).o -o $(TARGET)

$(SRC).o: $(SRC).c
	$(CC) $(CFLAG_1) $(CFLAG_3) $(LDFLAG) -D${CPU_Freq} -c -o $(SRC).o $(SRC).c

deploy:	$(SRC).hex
	avrdude -F -V -c arduino -p ATMEGA328p -P $(USB_PORT) -b 115200 -U flash:w:$(SRC).hex	

clean:
	rm $(TARGET) *.o *.hex

