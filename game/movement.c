#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <shape.h>
#include <abCircle.h>
#include "gameLayers.h"

u_char score = '0';
u_char lives = '3';
int state = 0;
char paddleSound;

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
