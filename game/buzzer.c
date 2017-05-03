#include "libTimer.h"
#include "buzzer.h"
#include <msp430.h>

static int counter = 0;

void buzzer_init() {
  /*
    Direct timer A output "TA0.1" to P2.6.
    According to table 21 from data sheet:
    P2SEL2.6, P2SEL2.7, and P2SEL.7 must be zero
    P2SEL.6 must be 1
    Also: P2.6 direction must be output
  */
  timerAUpmode(); 	 //used to drive speaker
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6; 	//enable output to speaker (P2.6)
}

void makePaddleSound(char enable) { /* Handles sound produced when the ball strikes the paddle */
    if(enable){
        CCR0 = 5000;
        CCR1 = 4500 ;
    }
    else{
        CCR0 = 0;
        CCR1 = 0;
    }
}

void buzzer_set_period(short cycles){
  CCR0 = cycles;
  CCR1 = cycles >> 1; //one half cycle
}

void gameOverSong(){
  switch(counter){
  	  case 0:
  		  buzzer_set_period(900); counter++;
  		  break; //Lower C note
  	  case 1:
  		  buzzer_set_period(400); counter++;
  		  break; //C note
  	  /*case 2:
  		  buzzer_set_period(700); counter++;
  		   break; //G note
  	  case 3:
  		  buzzer_set_period(200); counter++;
  		   break; //E note
  	  case 4:
  		  buzzer_set_period(800); counter++;
  		  break; //F note*/
  	  case 2:
  		  buzzer_set_period(700);

  		  	  if(counter == 2) {
  		  		  counter = 0;
  		  	  }
  		  	  else {
  		  		  counter++;
  		  	  };
  		  	  break;//D note
  	/*  case 6:
  	  case 7:
  	  case 8:
  	  case 9:
  	  case 10:
  	  case 11:
  	  case 12:
  	  case 13:
  	  case 14:
  	  case 15:*/

  }
}
