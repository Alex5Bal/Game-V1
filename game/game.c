#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6

u_char score = '0';
u_char lives = '3';
static int state = 0;
char bullet_sound;
//static int position = 0;

Region fence = {{10,20}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}};
AbRect rect = {abRectGetBounds, abRectCheck, {12,2}};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2-5, screenHeight/2-12}
};

Layer field = {
  (AbShape *)&fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  0,
};

Layer ball = {		/**< Layer with an yellow circle */
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight/8)}, /**<center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &field,
};

Layer paddle = {		/* playing field as a layer */
  (AbShape *)&rect,
  {screenWidth/2, screenHeight-16},     //current pos
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &ball
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
MovLayer mlPaddle = { &paddle, {2,0}, 0 }; //paddle
MovLayer mlBall = { &ball, {3,4}, 0 }; //ball

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
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], bounds.botRight.axes[0], bounds.botRight.axes[1]);

    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {

    	for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
    		Vec2 pixelPos = {col, row};
    		u_int color = bgColor;
    		Layer *probeLayer;

    		for (probeLayer = layers; probeLayer; probeLayer = probeLayer->next) { // probe all layers, in order
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
void moveBall(MovLayer *mlBall, Region *fence1, MovLayer *mlPaddle)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  int velocity;

  for (; mlBall; mlBall = mlBall->next) {
    vec2Add(&newPos, &mlBall->layer->posNext, &mlBall->velocity);
    abShapeGetBounds(mlBall->layer->abShape, &newPos, &shapeBoundary);

    for (axis = 0; axis < 2; axis ++) {

    	if((shapeBoundary.topLeft.axes[axis] < fence1->topLeft.axes[axis]) ||
    		(shapeBoundary.botRight.axes[axis] > fence1->botRight.axes[axis])) {

    		velocity = mlBall->velocity.axes[axis] = -mlBall->velocity.axes[axis];
    		newPos.axes[axis] += (2*velocity);
    	}
    	else if((abShapeCheck(mlPaddle->layer->abShape, &mlPaddle->layer->posNext, &mlBall->layer->posNext))) {
    		velocity = mlBall->velocity.axes[axis] = -mlBall->velocity.axes[axis];
    		newPos.axes[axis] += (4*velocity);
    		bullet_sound = 1;
    		if (score <= '8')
    			score += 1;

    	}
    	else if((shapeBoundary.botRight.axes[1] > fence1->botRight.axes[1]) && (lives != '0')) {
    		lives -= 1;
    	}
    	if(lives == '0')
    		state = 1;

    	if (score > '8')
    	    state = 2;

    } /**< for axis */

    mlBall->layer->posNext = newPos;

  } /**< for ml */
}

void moveRight(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;

  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);

    for (axis = 0; axis < 2; axis++) {

    	if (shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis])
    		newPos.axes[axis] += 1;
    	else if(shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])
    	    newPos.axes[axis] += -1;
      	// this if statement handles when a collision happens in the fence

    } /**< for axis */

    ml->layer->posNext = newPos;

  } /**< for ml */
}

void moveLeft(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;

  for (; ml; ml = ml->next) {
    vec2Sub(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);

    for (axis = 0; axis < 2; axis ++) {

    	if (shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis])
    	    	newPos.axes[axis] += 1;
    	else if(shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])
    	    	newPos.axes[axis] += -1;

    	// this if statement handles when a collision happens in the fence
    } /**< for axis */

    ml->layer->posNext = newPos;

  } /**< for ml */
}

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
    //drawString5x7(0, 150, "S1-R", COLOR_WHITE, COLOR_BLACK);
   //drawString5x7(100, 150, "S4-L", COLOR_WHITE, COLOR_BLACK);
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

    if(bullet_sound) {
    	static char sound_count = 0;
    		if(sound_count == 0)
    			make_bullet_sound(1);
    	    if (++sound_count == 50) {
    	        make_bullet_sound(0);
    	        sound_count = 0;
    	        bullet_sound = 0;
    	    }
    }

    redrawScreen = 1;
    count = 0;
    P1OUT &= ~GREEN_LED;	/**< Green LED off when cpu off */
  }
}/****END****/


void make_bullet_sound(char sound_enable){
// Makes the actual sound of a bullet being realeased, every time the user
// realeases a bullet, the game will make an special sound
    if(sound_enable){
        CCR0 = 2000;
        CCR1 = 1000;
    }else{
        CCR0 = 0;
        CCR1 = 0;
    }

}


/*void __interrupt_vec(PORT2_VECTOR) Port_2(){
    if (P2IFG & SWITCHES) {	          // did a button cause this interrupt?
        P2IFG &= ~SWITCHES;		      // clear pending switches interrupts
        switch_interrupt_handler();
    }

}*/

/*void __interrupt_vec(WDT_VECTOR) WDT(){	        // 250 interrupts/sec
	if(enable_sound){
		if(bullet_sound) {
			static char sound_count = 0;
	        	if(sound_count == 0)
	        		make_bullet_sound(1);
	        	if (++sound_count == 50) {
	                make_bullet_sound(0);
	                sound_count = 0;
	                bullet_sound = 0;
	        	}
	    }
	}



	if(state == 0){
        static char decisecond_count = 0;
        if (++decisecond_count == 110) {
            drawBlinkText("Press S1");
            decisecond_count = 0;
        }
    }
    if(state == 1){
        if(bullet_was_fired){
            static char second_count = 0;                     // state is 3 when a song is paused
            if (++second_count == 10) {
                    fire_bullet_handler();
                    second_count = 0;
            }
        }

       static char second_count = 0;                     // state is 3 when a song is paused
       if (++second_count == 20) {
           static char alien_turn = 2;
           if(alien_turn == 2){
                mov_aliens2();
                alien_turn = 1;
           }
           else if(alien_turn == 1){
                mov_aliens();
                alien_turn = 2;
           }
           second_count = 0;
       }
       static short level_count = 0;                     // state is 3 when a song is paused
       if(game_level > 1 && ++level_count == 800) {
           level_count = 0;
           if(ALIEN1_LIVES)
               change_velDirection(&mvA1);
           if(ALIEN2_LIVES)
               change_velDirection(&mvA2);
       }
       if (game_level == 3 && level_count%400 == 1) {
           change_alienColor();
       }
       static char score_count = 0;                     // state is 3 when a song is paused
       if (score && ++score_count == 250) {
           drawScore(--score);
           score_count = 0;
           if(!score)
               gameover_state();
       }

    }
    if(enable_sound){
       if(bullet_sound){
           static char sound_count = 0;
           if(sound_count == 0)
               make_bullet_sound(1);
           if (++sound_count == 50) {
                make_bullet_sound(0);
                sound_count = 0;
                bullet_sound = 0;
           }
      }
      if(selected_song){
        static char music_count = 0;
        if (++music_count == 50) {
            buzzer_advance_frequency();     // buzzer_advance_frequency generates the sound
            music_count = 0;
            note++;
        }
      }

    }
    else{
        bullet_sound = 0;
        selected_song = 0;
    }

}*/

