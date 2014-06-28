OBJECTS=

MCU_NAME=mega
MCU_NUMBER=16
F_CPU=8000000ULL

MMCU=at$(MCU_NAME)$(MCU_NUMBER)
DEV_DEF=__AVR_AT$(MCU_NAME)$(MCU_NUMBER)__

MCUFLAGS=-mmcu=$(MMCU)

CFLAGS=-O2 $(MCUFLAGS) -D$(DEV_DEF) -DF_CPU=$(F_CPU)

LDFLAGS=-Wl,-Map=$(TARGET).map,--cref $(MCUFLAGS)

LIBSPATH=-L/opt/cross/avr/avr/lib/avr4
LIBS=-lc

INCS=-I../lib

TARGET=main
TARGET_HEX=$(TARGET).hex
TARGET_EPP_HEX=$(TARGET).epp
TARGET_EPP_BIN=$(TARGET).epp.bin

CC=avr-gcc
LD=avr-gcc
OBJCOPY=avr-objcopy
RM=rm -f
STRIP=avr-strip
SIZE=avr-size



all: $(OBJECTS)
	$(LD) -o $(TARGET) $(LDFLAGS) $^ $(LIBSPATH) $(LIBS)
	$(STRIP) $(TARGET)
	$(SIZE) -A $(TARGET)
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET) $(TARGET_HEX)
	$(OBJCOPY) -O ihex -j .eeprom --change-section-lma .eeprom=0 $(TARGET) $(TARGET_EPP_HEX)
	$(OBJCOPY) -O binary -j .eeprom --change-section-lma .eeprom=0 $(TARGET) $(TARGET_EPP_BIN)

clean:
	$(RM) $(OBJECTS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

font.o: ../lib/font/font.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

lcd8544.o: ../lib/lcd8544/lcd8544.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

timer2.o: ../lib/timer2/timer2.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

ports.o: ../lib/ports/ports.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

ssi.o: ../lib/ssi/ssi.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

display_7seg.o: ../lib/display_7seg/display_7seg.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

graphics.o: ../lib/graphics/graphics.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

future.o: ../lib/future/future.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

gyro6050.o: ../lib/gyro6050/gyro6050.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

lcd44780.o: ../lib/lcd44780/lcd44780.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

circular_buffer.o: ../lib/buffer/circular_buffer.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

dpy7ser.o: ../lib/dpy7ser/dpy7ser.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

ds18x20.o: ../lib/ds18x20/ds18x20.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

dpy7par.o: ../lib/dpy7par/dpy7par.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

timer0.o: ../lib/timer0/timer0.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

cordic16.o: ../lib/cordic/cordic16.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

cordic32.o: ../lib/cordic/cordic32.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

cordic10_6.o: ../lib/cordic/cordic10_6.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

uart.o: ../lib/uart/uart.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

delay.o: ../lib/utils/delay.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

ds1307.o: ../lib/ds1307/ds1307.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

i2c.o: ../lib/i2c/i2c.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

i2c_with_slave_listen.o: ../lib/i2c/i2c_with_slave_listen.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

one_wire_search.o: ../lib/one_wire/one_wire_search.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

one_wire.o: ../lib/one_wire/one_wire.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

int1.o: ../lib/ext_int/int1.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

int0.o: ../lib/ext_int/int0.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

spi.o: ../lib/spi/spi.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

adc.o: ../lib/adc/adc.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

ds1302.o: ../lib/ds1302/ds1302.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

lcd0108.o: ../lib/lcd0108/lcd0108.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

button.o: ../lib/button/button.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

timer1.o: ../lib/timer1/timer1.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)

counter.o: ../lib/counter/counter.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCS)
