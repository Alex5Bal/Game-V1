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

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

/** Initializes everything, enables interrupts and green LED,
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		//Green led on when CPU on
  P1OUT |= GREEN_LED;

  configureClocks(); //initialize clocks
  lcd_init(); //initialize lcd
  buzzer_init(); //initialize buzzer
  p2sw_init(15); //initialize switches
  layerInit(&paddle); //Passes the first element from a MoveLayer LL to initialize shapes
  layerDraw(&paddle); //Passes the first element from a MoveLayer LL to draw shapes
  layerGetBounds(&field, &fence);
  enableWDTInterrupts();      // enable periodic interrupt
  or_sr(0x8);	              // GIE (enable interrupts)

  for(;;) {
    while (!redrawScreen) { // Pause CPU if screen doesn't need updating
      P1OUT &= ~GREEN_LED; // Green led off witHo CPU
      or_sr(0x10); //< CPU OFF
    }

    P1OUT |= GREEN_LED; // Green led on when CPU on
    redrawScreen = 0;
    movLayerDraw(&mlBall, &paddle);
    movLayerDraw(&mlPaddle, &paddle);
    drawString5x7(5, 0, "SCORE:", COLOR_WHITE, COLOR_BLACK);
    drawChar5x7(45, 0, score, COLOR_WHITE, COLOR_BLACK);
    drawString5x7(80, 0, "LIVES:", COLOR_WHITE, COLOR_BLACK);
    drawChar5x7(120, 0, lives, COLOR_WHITE, COLOR_BLACK);
    drawString5x7(50, 150, "PONG", COLOR_WHITE, COLOR_BLACK);
  }
}

void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  u_int switches = p2sw_read();

  if(count == 5) {

	switch(state) {

		case 0:
			moveBall(&mlBall, &fence, &mlPaddle);
			break;

		case 1:
			drawString5x7(40, 80, "GAME OVER", COLOR_WHITE, COLOR_BLACK);
			break;

		case 2:
			drawString5x7(45, 80, "YOU WIN", COLOR_WHITE, COLOR_BLACK);
			break;

    }

    if(switches & (1<<3)) {
    	moveRight(&mlPaddle, &fence);
    }

    if(switches & (1<<0)) {
    	moveLeft(&mlPaddle, &fence);
    }

    if(paddleSound) {
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
    P1OUT &= ~GREEN_LED;	/**< Green LED off when cpu off */
  }
}

