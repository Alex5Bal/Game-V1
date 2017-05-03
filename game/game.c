#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"
#include "gameLayers.h"
#include "movement.h"

#define GREEN_LED BIT6

u_int bgColor = COLOR_BLACK;	//background color
int redrawScreen = 1;

void main() /* Handles initializations, clock configuration, screen display, and enables interrupts */
{
  P1DIR |= GREEN_LED;
  P1OUT |= GREEN_LED;

  configureClocks();		//clock configuration
  lcd_init(); 				//LCD initialization
  buzzer_init(); 			//buzzer initialization
  p2sw_init(15); 			//switches initialization
  game_init();				//layers initialization
  enableWDTInterrupts();    //enable periodic interrupt
  or_sr(0x8);	            //GIE (enable interrupts)

  for(;;) {
    while (!redrawScreen) {
      P1OUT &= ~GREEN_LED;	//LED off
      or_sr(0x10); 	//CPU off
    }

    P1OUT |= GREEN_LED;		//LED on
    redrawScreen = 0;
    movLayerDraw(&mlBall, &paddle);
    movLayerDraw(&mlPaddle, &paddle);
    drawString5x7(5, 0, "SCORE:", COLOR_WHITE, COLOR_BLACK);	//screen score
    drawChar5x7(45, 0, score, COLOR_WHITE, COLOR_BLACK);		//score
    drawString5x7(80, 0, "LIVES:", COLOR_WHITE, COLOR_BLACK);	//screen lives
    drawChar5x7(120, 0, lives, COLOR_WHITE, COLOR_BLACK);		//lives
    drawString5x7(50, 150, "PONG", COLOR_WHITE, COLOR_BLACK);	//screen PONG
  }
}

void wdt_c_handler() /* Handles game states, game controls, and game sounds */
{
  static short count = 0;
  P1OUT |= GREEN_LED;	//LED on
  count++;
  u_int switches = p2sw_read();

  if(count == 5) {

	switch(state) {

		case 0: //initiates ball movement
			moveBall(&mlBall, &fence, &mlPaddle);
			break;

		case 1:	//prints "GAME OVER"
			drawString5x7(40, 80, "GAME OVER", COLOR_WHITE, COLOR_BLACK);
			gameOverSong();
			break;

		case 2:	//prints "YOU WIN"
			drawString5x7(45, 80, "YOU WIN", COLOR_WHITE, COLOR_BLACK);
			break;

    }

    if(switches & (1<<3)) {				//S4 handles left paddle movement
    	moveRight(&mlPaddle, &fence);
    }

    if(switches & (1<<0)) {				//S1 handles right paddle movement
    	moveLeft(&mlPaddle, &fence);
    }

    if(paddleSound) {					//initiates and terminates paddle-ball sound
    	static char counter = 0;

    	if(counter == 0)
    		makePaddleSound(1);
    	if (++counter == 20) {
    	    makePaddleSound(0);
    	    counter = 0;
    	    paddleSound = 0;
    	}
    }

    redrawScreen = 1;
    count = 0;
    P1OUT &= ~GREEN_LED;				//LED off
  }
}

void game_init()
{
   layerInit(&paddle);
   layerDraw(&paddle);
   layerGetBounds(&field, &fence);

}
