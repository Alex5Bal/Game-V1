#ifndef movement_included
#define movement_included

#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <shape.h>
#include <abCircle.h>

u_char score;
u_char lives;
static int state;
char paddleSound;

typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

void movLayerDraw(MovLayer *movLayers, Layer *layers);

void moveBall(MovLayer *mlBall, Region *fence1, MovLayer *mlPaddle);

void moveRight(MovLayer *ml, Region *fence);

void moveLeft(MovLayer *ml, Region *fence);

#endif
