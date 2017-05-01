#ifndef buzzer_included
#define buzzer_included

void buzzerInit();
void buzzerAdvanceFrequency();
void buzzerSetPeriod(short cycles);
void makePaddleSound(char enable);

#endif
