#!/usr/bin/make
###############################################################################
# Makefile para el Proyecto del Motor Driver. Permite compilar su código.
###############################################################################

## General Flags
OBJDIR=output
PRJ = motdrv
MCUA = m168p
MCU = atmega168p
FCPU = 20000000UL
TARGET = $(OBJDIR)/$(PRJ).elf
CC = avr-gcc
OPT = s

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -gdwarf-2 -std=gnu99 -DF_CPU=$(FCPU) -O$(OPT) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(OBJDIR)/$(*F).o -MF $(OBJDIR)/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(OBJDIR)/$(PRJ).map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Libraries
LIBS = -lm -lc

## Objects that must be built in order to link
OBJECTS = $(OBJDIR)/spi_slave.o $(OBJDIR)/motor.o $(OBJDIR)/calc.o  $(OBJDIR)/main.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(OBJDIR) $(TARGET) $(OBJDIR)/$(PRJ).hex $(OBJDIR)/$(PRJ).eep $(OBJDIR)/$(PRJ).lss size

## Compile
$(OBJDIR):
	mkdir $@

$(OBJDIR)/%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $< -o $@

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

$(OBJDIR)/%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

$(OBJDIR)/%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

$(OBJDIR)/%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

$(OBJDIR)/$(PRJ).asm: ${TARGET}
	avr-objdump -S $< > $@

asm: $(OBJDIR)/$(PRJ).asm

## FUSES
low_fuse:
	avrdude -p $(MCUA) -c usbtiny  -U lfuse:w:0xF7:m	 #    // grabación de Low Fuse internal clock

high_fuse:
	avrdude -p $(MCUA) -c usbtiny  -U hfuse:w:0xDF:m	 #    // grabación de High Fuse

def_fuse:
	avrdude -p $(MCUA) -c usbtiny  -U lfuse:w:0x62:m	 #    // grabación de Low Fuse internal clock

erase:
	avrdude -p $(MCUA) -c usbtiny  -e	                #    //borrado del micro

load_prg:
	avrdude -p $(MCUA) -c usbtiny  -U flash:w:$(OBJDIR)/$(PRJ).hex:a #    // grabación del programa

verify:
	avrdude -p $(MCUA) -c usbtiny  -U flash:v:$(OBJDIR)/$(PRJ).hex:a	#    // verificación

## Clean target
#.PHONY: clean
clean:
	rm -r $(OBJDIR)


