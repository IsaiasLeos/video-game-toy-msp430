#ifndef buzzer_included
#define buzzer_included

static int musicState;
void buzzer_init();
void buzzer_set_period(short cycles);

void play(long note, long tempo);

#endif
