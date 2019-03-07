#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

/* Keep track of which noise is currently being played. */
static int musicState = 0;
/* Call before Speaker is used */
void buzzer_init(){
  timerAUpmode();//used to drive speaker
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7; 
  P2SEL |= BIT6;
  P2DIR = BIT6;//enable output to speaker (P2.6)
}

/* Make a noise on Frequency Given */
void buzzer_set_period(short note){
  CCR0 = note;
  CCR1 = note >> 1;
}
