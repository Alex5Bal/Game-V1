#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

int abSlicedRectCheck(const AbRect *rect, const Vec2 *centerPos, const Vec2 *pixel)
{
  Vec2 relPos;
  vec2Sub(&relPos, pixel, centerPos); /* vector from center to pixel */

  /* reject pixels in slice */
  if (relPos.axes[0] >= 0 && relPos.axes[1]/2 < relPos.axes[1])
    return 0;
  else
    return abRectCheck(rect, centerPos, pixel);
}

Region fence = {{10,20}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}};
AbRect rect = {abRectGetBounds, abRectCheck, {10,2}};

u_char player1Score = '0';
u_char lives = '0';
static int state = 0;

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2-5, screenHeight/2-10}
};

Layer fieldLayer = {
  (AbShape *)&fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  0,
};

Layer layer3 = {		/**< Layer with an yellow circle */
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight/8)}, /**<center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &fieldLayer,
};

Layer layer1 = {		/* playing field as a layer */
  (AbShape *)&rect,
  {screenWidth/2, screenHeight-15},     //current pos
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &layer3
};

//Layer layer2 = {		/**< Layer with a red rect */
 /* (AbShape *)&rect,
  {screenWidth/2+50, screenHeight/2+5}, //current pos
  {0,0}, {0,0},				    /* last & next pos */
  /*COLOR_GREEN,
  &layer1,
};*/

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */
Region fieldFence;		/**< fence around playing field  */

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
  layerInit(&layer1); //Passes the first element from a MoveLayer LL to initilize shapes
  layerDraw(&layer1); //Passes the first element from a MoveLayer LL to draw shapes
  layerGetBounds(&fieldLayer, &fieldFence);
  enableWDTInterrupts();      // enable periodic interrupt
  or_sr(0x8);	              // GIE (enable interrupts)

  u_int switches = p2sw_read();

  for(;;){
    while (!redrawScreen) { // Pause CPU if screen doesn't need updating
      P1OUT &= ~GREEN_LED; // Green led off witHo CPU
      or_sr(0x10); //< CPU OFF
    }
    P1OUT |= GREEN_LED; // Green led on when CPU on
    redrawScreen = 0;
    //movLayerDraw(&ml3, &layer2);
    //movLayerDraw(&ml2, &layer2);
    //movLayerDraw(&ml1, &layer2);
    drawString5x7(5, 0, "SCORE: ", COLOR_WHITE, COLOR_BLACK);
    drawChar5x7(45, 0, player1Score, COLOR_WHITE, COLOR_BLACK); //Scoreboard
    drawString5x7(60, 0, "LIVES: ", COLOR_WHITE, COLOR_BLACK);
    drawChar5x7(95, 0, lives, COLOR_WHITE, COLOR_BLACK);
    //drawChar5x7(115, 5, player2Score, COLOR_BLACK, COLOR_BLUE); //Scoreboard
    //drawString5x7(5, 150, "X X X X PONG X X X X", COLOR_BLACK, COLOR_BLUE);
    //drawString5x7(38, 5, "<-SCORE->", COLOR_BLACK, COLOR_BLUE);
  }
}

void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  u_int switches = p2sw_read();
  /*if(count == 10){
    switch(state){
    case 0:
      moveBall(&ml3, &fieldFence, &ml1, &ml2);
      break;
    case 1:
      layerDraw(&layer2);
      if(player1Score > player2Score)
	drawString5x7(28, 50, "PLAYER 1 WON!", COLOR_BLACK, COLOR_BLUE);
      else if(player1Score < player2Score)
	drawString5x7(28, 50, "PLAYER 2 WON!", COLOR_BLACK, COLOR_BLUE);
      break;
    }
    starWarsTheme();
    if(switches & (1<<3)){
      moveUp(&ml2, &fieldFence);
    }
    if(switches & (1<<2)){
      moveDown(&ml2, &fieldFence);
    }
    if(switches & (1<<1)){
      moveUp(&ml1, &fieldFence);
    }
    if(switches & (1<<0)){
      moveDown(&ml1, &fieldFence);
    }
    redrawScreen = 1;
    count = 0;
  }*/
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}/****END****/
