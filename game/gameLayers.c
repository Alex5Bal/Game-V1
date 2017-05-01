#include <shape.h>
#include <abCircle.h>

Region fence = {{10,20}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}};
AbRect rect = {abRectGetBounds, abRectCheck, {12,2}};	//rectangular paddle

AbRectOutline fenceOutline = {		//field outline
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2-5, screenHeight/2-12}
};

Layer field = {		//field layer
  (AbShape *)&fenceOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  0,
};

Layer ball = {		//ball layer
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight/8)},
  {0,0}, {0,0},
  COLOR_WHITE,
  &field,
};

Layer paddle = {	//paddle layer
  (AbShape *)&rect,
  {screenWidth/2, screenHeight-16},
  {0,0}, {0,0},
  COLOR_WHITE,
  &ball
};
