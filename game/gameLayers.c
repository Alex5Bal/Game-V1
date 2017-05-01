#include <shape.h>
#include <abCircle.h>

Region fence = {{10,20}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}};
AbRect rect = {abRectGetBounds, abRectCheck, {12,2}};

AbRectOutline fenceOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2-5, screenHeight/2-12}
};

Layer field = {
  (AbShape *)&fenceOutline,
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
