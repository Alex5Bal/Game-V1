#include "libTimer.h"
#include "buzzer.h"
#include <msp430.h>

unsigned int period = 1000;
signed int rate = 1000;

#define MIN_PERIOD 1000
#define MAX_PERIOD 6000

void buzzer_init(){
  /*
    Direct timer A output "TA0.1" to P2.6.
    According to table 21 from data sheet:
    P2SEL2.6, P2SEL2.7, and P2SEL.7 must be zero
    P2SEL.6 must be 1
    Also: P2.6 direction must be output
  */
  timerAUpmode(); //used to drive speaker
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6; //enable output to speaker (P2.6)
}

void makePaddleSound(char enable) {
// Makes the actual sound of a bullet being realeased, every time the user
// realeases a bullet, the game will make an special sound
    if(enable){
        CCR0 = 5000;
        CCR1 = 4500 ;
    }
    else{
        CCR0 = 0;
        CCR1 = 0;
    }

}
