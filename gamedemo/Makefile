# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:gamedemo.elf

#additional rules for files
gamedemo.elf: ${COMMON_OBJECTS} gamedemo.o wdt_handler.o p2switches.o buzzer.o triangle.o musicStates.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd -lShape -lCircle -lp2sw

load: gamedemo.elf
	mspdebug rf2500 "prog $^"
	
clean:
	rm -f *.o *.elf *#
