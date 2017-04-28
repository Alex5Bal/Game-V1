/** \file pong.c
 *  \brief This is an implementation of the Pong game.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

int
abSlicedRectCheck(const AbRect *rect, const Vec2 *centerPos, const Vec2 *pixel)
{
  Vec2 relPos;
  vec2Sub(&relPos, pixel, centerPos); /* vector from center to pixel */

  /* reject pixels in slice */
  if (relPos.axes[0] >= 0 && relPos.axes[1]/2 < relPos.axes[1])
    return 0;
  else
    return abRectCheck(rect, centerPos, pixel);
}

AbRect letterP = {abRectGetBounds, abSlicedRectCheck, 10,15};;
Region fence = {{10,20}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}};
AbRect rect = {abRectGetBounds, abRectCheck, {2,10}};

u_char player1Score = '0';
u_char player2Score = '0';
static int state = 0;

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2-5, screenHeight/2-15}
};

Layer letterPLayer = {
  (AbShape *)&letterP,
  {screenWidth/2-4, screenHeight/2+7}, /* position */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  0,
};

Layer fieldLayer = {
  (AbShape *)&fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_BLACK,
  &letterPLayer
};

Layer layer3 = {		/**< Layer with an yellow circle */
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight/2)}, /**<center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_YELLOW,
  &fieldLayer,
};

Layer layer1 = {		/* playing field as a layer */
  (AbShape *) &rect,
  {screenWidth/2-50, screenHeight/2+5},     //current pos
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer3
};

Layer layer2 = {		/**< Layer with a red rect */
  (AbShape *)&rect,
  {screenWidth/2+50, screenHeight/2+5}, //current pos
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GREEN,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml1 = { &layer1, {0,3}, 0 }; //Red paddle
MovLayer ml3 = { &layer3, {2,4}, 0 }; //Yellow circle
MovLayer ml2 = { &layer2, {0,3}, 0 }; //Green circle

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { // for each moving layer
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */

  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { // for each moving layer
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
    		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer;
	     probeLayer = probeLayer->next) { // probe all layers, in order
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break;
	   } // if probe check makes the ball visible when moving
	} // for checking all layers at col, row
	lcd_writeColor(color);
      } // for col
    } // for row
  } // for moving layer being updated
}

/** Advances a moving shape within a fence
 *
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void moveBall(MovLayer *ml, Region *fence1, MovLayer *ml2, MovLayer *ml3)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  int velocity;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++){
      if((shapeBoundary.topLeft.axes[axis] < fence1->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence1->botRight.axes[axis])){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }
      else if((abShapeCheck(ml2->layer->abShape, &ml2->layer->posNext, &ml->layer->posNext))){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }
      else if((abShapeCheck(ml3->layer->abShape, &ml3->layer->posNext, &ml->layer->posNext))){
	velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }
      else if((shapeBoundary.topLeft.axes[0] < fence1->topLeft.axes[0])){
	newPos.axes[0] = screenWidth/2;
	newPos.axes[1] = screenHeight/2;
	player2Score = player2Score - 255;
      }
      else if((shapeBoundary.botRight.axes[0] > fence1->botRight.axes[0])){
	newPos.axes[0] = screenWidth/2;
	newPos.axes[1] = screenHeight/2;
	player1Score = player1Score - 255;
      }
      if(player1Score == '5' || player2Score == '5'){
	state = 1;
      }
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void moveDown(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 1; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	// this if statement handles when a collision happens in the fence
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void moveUp(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Sub(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 1; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	// this if statement handles when a collision happens in the fence
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}

u_int bgColor = COLOR_BLUE;     /**< The background color */
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
  layerInit(&layer2); //Passes the first element from a MoveLayer LL to initilize shapes
  layerDraw(&layer2); //Passes the first element from a MoveLayer LL to draw shapes
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
    movLayerDraw(&ml3, &layer2);
    movLayerDraw(&ml2, &layer2);
    movLayerDraw(&ml1, &layer2);
    drawChar5x7(5, 5, player1Score, COLOR_BLACK, COLOR_BLUE); //Scoreboard
    drawChar5x7(115, 5, player2Score, COLOR_BLACK, COLOR_BLUE); //Scoreboard
    drawString5x7(5, 150, "X X X X PONG X X X X", COLOR_BLACK, COLOR_BLUE);
    drawString5x7(38, 5, "<-SCORE->", COLOR_BLACK, COLOR_BLUE);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  u_int switches = p2sw_read();
  if(count == 10){
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
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
