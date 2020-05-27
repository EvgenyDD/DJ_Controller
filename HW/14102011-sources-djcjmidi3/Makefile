###############################################################################
# Makefile for MIDI DJ CJ CONTROLLER 3.0 USB
###############################################################################

## General Flags
PROJECT = djcj3
MCU = atmega8
F_CPU = 12000000
TARGET = $(PROJECT).elf
DEBUG =  -DDEBUG_LEVEL=0
CC = avr-gcc
AVRDUDE = avrdude -c usbasp -p$(MCU)

## Options common to compile, link and assembly rules
COMMON = -g -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -DF_CPU=12000000UL -Os -fsigned-char  $(DEBUG)
CFLAGS += -ffunction-sections

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map
LDFLAGS +=  -Wl,--gc-sections


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom


## Include Directories
INCLUDES = -I"." -I"../usbdrv" -I"../." 

## Objects that must be built in order to link
OBJECTS = usbdrv.o usbdrvasm.o oddebug.o lcd_lib.o main.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) $(PROJECT).hex $(PROJECT).lss

$(OBJECTS): usbconfig.h Makefile

## Compile
usbdrv.o: ../usbdrv/usbdrv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usbdrvasm.o: ../usbdrv/usbdrvasm.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

oddebug.o: ../usbdrv/oddebug.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

lcd_lib.o: lcd_lib.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
