#!/usr/bin/make
###############################################################################
# Makefile for the project PAD
###############################################################################

## General Flags
OBJDIR=output
PRJ = pad
MCU = atmega128
FCPU = 8000000UL
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
SD_OBJ = $(OBJDIR)/partition.o $(OBJDIR)/byteordering.o $(OBJDIR)/fat.o $(OBJDIR)/sd.o $(OBJDIR)/sd_raw.o
RTC_OBJ = $(OBJDIR)/i2c.o $(OBJDIR)/rtc.o
PC_OBJ = $(OBJDIR)/uart.o
OBJECTS = $(OBJDIR)/calc.o $(OBJDIR)/lcd.o $(OBJDIR)/teclado.o $(OBJDIR)/util.o $(OBJDIR)/spi_master.o $(OBJDIR)/menu.o $(OBJDIR)/anim.o $(RTC_OBJ) $(SD_OBJ) $(PC_OBJ) $(OBJDIR)/main.o


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
	avrdude -p $(MCU) -c usbtiny  -U lfuse:w:0xCE:m	 #    // grabación de Low Fuse internal clock

high_fuse:
	avrdude -p $(MCU) -c usbtiny  -U hfuse:w:0xC9:m	 #    // grabación de High Fuse

not_103c:
	avrdude -p $(MCU) -c usbtiny  -U efuse:w:0xFF:m	 #    // grabación de Low Fuse internal clock

erase:
	avrdude -p $(MCU) -c usbtiny  -e	                #    //borrado del micro

load_prg:
	avrdude -p $(MCU) -c usbtiny  -U flash:w:$(OBJDIR)/$(PRJ).hex:a #    // grabación del programa

verify:
	avrdude -p $(MCU) -c usbtiny  -U flash:v:$(OBJDIR)/$(PRJ).hex:a	#    // verificación

## Clean target
#.PHONY: clean
clean:
	rm -r $(OBJDIR)



