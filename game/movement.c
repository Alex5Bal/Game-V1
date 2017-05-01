#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <shape.h>
#include <abCircle.h>
#include "gameLayers.h"

u_char score = '0';		//game score
u_char lives = '3';		//game lives
int state = 0;			//game state
char paddleSound;		//sound signaler

typedef struct MovLayer_s { /* Linked list of layer types */
  Layer *layer;
  Vec2 velocity;			//direction and magnitude
  struct MovLayer_s *next;
} MovLayer;

MovLayer mlPaddle = { &paddle, {2,0}, 0 }; 	//paddle
MovLayer mlBall = { &ball, {3,4}, 0 }; 		//ball

void movLayerDraw(MovLayer *movLayers, Layer *layers) /* Draws layers at appropriate positions */
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);		//disable interrupts (GIE off)

  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) {
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }

  or_sr(8);			//enable interrupts (GIE on)

  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) {
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], bounds.botRight.axes[0], bounds.botRight.axes[1]);

    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {

    	for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
    		Vec2 pixelPos = {col, row};
    		u_int color = bgColor;
    		Layer *probeLayer;

    		for (probeLayer = layers; probeLayer; probeLayer = probeLayer->next) {
    			if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
    				color = probeLayer->color;
    				break;
    			} 	//if probe check makes the ball visible when moving
    		} 		//for checking all layers at column, row

    		lcd_writeColor(color);
      } 			//for column
    } 				//for row
  } 				//for moving layer being updated
}

void moveBall(MovLayer *mlBall, Region *fence1, MovLayer *mlPaddle) /* Determines effects of the ball */
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
    		newPos.axes[axis] += (3*velocity);
    		paddleSound = 1;
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

    }

    mlBall->layer->posNext = newPos;
  }
}

void moveRight(MovLayer *ml, Region *fence) /* Controls paddle position when moving right */
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
    }

    ml->layer->posNext = newPos;
  }
}

void moveLeft(MovLayer *ml, Region *fence) /* Controls paddle position when moving left */
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
    }

    ml->layer->posNext = newPos;

  }
}
