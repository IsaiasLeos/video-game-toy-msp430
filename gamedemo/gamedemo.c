#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include "shape.h"
#include <abCircle.h>
#include <p2switches.h>
#include "layers.h"
#include "buzzer.h"

#define GREEN_LED BIT6
#define RED_LED BIT0
static int shapeCount = 0;
static int x = 0, y = 0;//Keep track of the player's coordinates
static char playerHelp = 0;//Keep track whether the player requires help
static char itemStates = 1;
static char redrawShape = 0;
static char collide = 0;
static char death = 0;

u_int itemColor = COLOR_BLACK;

void buzzer_init();//Enable buzzer
void playMusic();//Player buzzer sound

AbRect head = {abRectGetBounds, abRectCheck, 2,2};
AbRect neck = {abRectGetBounds, abRectCheck, 1,1};
AbTriangle itemShape = {abTriangleGetBounds, abTriangleCheck, 20};
AbRect arm = {abRectGetBounds, abRectCheck, 1,4};
AbRect body = {abRectGetBounds, abRectCheck, 3,5};
AbRect leg = {abRectGetBounds, abRectCheck, 1,4};

AbRectOutline fieldOutline = {/* Gaming Field: Size of MSP430 Screen */
    abRectOutlineGetBounds, abRectOutlineCheck,   
    {(screenWidth/2)-3, (screenHeight/2)-3}
};

AbRectOutline playerOutline = {/* Outline of the Item */
    abRectOutlineGetBounds, abRectOutlineCheck,   
    {(screenWidth/2) - 60, (screenHeight/2) - 70}
};


AbRectOutline itemOutline = {/* Outline of the Item */
    abRectOutlineGetBounds, abRectOutlineCheck,   
    {(screenWidth/2) - 60, (screenWidth/2) - 60}
};


Layer itemDesign = {/* Visual layer of the item */
    (AbShape *)&itemShape,
    {0, 0},//Spawning Location of the Item
    {0,0}, {0,0},
    COLOR_BLUE,//Initial Color of the Item
    0,
};

Layer itemLayer = {/* Visual layer of the item */
    (AbShape *)&itemOutline,
    {0, 0},//Spawning Location of the Item
    {0,0}, {0,0},
    COLOR_BLUE,//Initial Color of the Item
    &itemDesign,
};


Layer layer5 = {/* Player's Body */
    (AbShape *)&body,
    {screenWidth/2, screenHeight/2},//Location of ite appearing initially
    {0,0}, {0,0},
    COLOR_BLUE,
    &itemLayer,
};

Layer layer4 = {/* Right Arm of the Player */
    (AbShape *)&arm,
    {(screenWidth/2)+5, (screenHeight/2)-1},
    {0,0}, {0,0},
    COLOR_BLUE,
    &layer5,
};

Layer layer3 = {/* Left Arm of the Player */
    (AbShape *)&arm,
    {(screenWidth/2)-5, (screenHeight/2)-1},
    {0,0}, {0,0},
    COLOR_BLUE,
    &layer4,
};

Layer layer2 = {/* Right Leg of the Player */
    (AbShape *)&leg,
    {(screenWidth/2)+2, (screenHeight/2)+9},
    {0,0}, {0,0},
    COLOR_BLUE,
    &layer3,
};

Layer layer1 = {/* Left Leg of the Player */
    (AbShape *)&leg,
    {(screenWidth/2)-2, (screenHeight/2)+10},
    {0,0}, {0,0},
    COLOR_BLUE,
    &layer2,
};

Layer layer0 = {/* Head of the Player */
    (AbShape *)&head,
    {(screenWidth/2), (screenHeight/2)-9},
    {0,0}, {0,0},
    COLOR_BLUE,
    &layer1,
};

Layer hitbox = {/* Player's Hitbox */
    (AbShape *)&playerOutline,
    {screenWidth/2, screenHeight/2+4},//Location of ite appearing initially
    {0,0}, {0,0},
    COLOR_BLACK,
    &layer0,
};


/* Layers that are going to move, with zero initial velocity */
MovLayer ml5 = {&layer5, {0,0}, 0};
MovLayer ml4 = {&layer4, {0,0}, &ml5};
MovLayer ml3 = {&layer3, {0,0}, &ml4};
MovLayer ml2 = {&layer2, {0,0}, &ml3};
MovLayer ml1 = {&layer1, {0,0}, &ml2};
MovLayer ml0 = {&layer0, {0,0}, &ml1};
MovLayer mlh = {&hitbox, {0,0}, &ml0};

/* Color background of the MSP430 */
u_int bgColor = COLOR_BLACK;
/* Variable to keep track of when to re-draw the layers */
int redrawScreen = 1;

/* Region of the Player Hitbox */
Region playerFence;
/* Region of the item the player needs to pick up */
Region itemFence;

void movLayerDraw(MovLayer *movLayers, Layer *layers) {
    int row, col;
    MovLayer *movLayer;
    and_sr(~8);
    for (movLayer = movLayers; movLayer; movLayer = movLayer->next) {
        Layer *l = movLayer->layer;
        l->posLast = l->pos;
        l->pos = l->posNext;
    }
    or_sr(8);
    for (movLayer = movLayers; movLayer; movLayer = movLayer->next) {
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
                     probeLayer = probeLayer->next) {
                    if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
                        color = probeLayer->color;
                        break; 
                    }
                     }
                     lcd_writeColor(color); 
            }
        }
    }
}	  

/* Moving layers to their next position to properly render them */
void mlAdvance(MovLayer *ml) {
    Vec2 newPos;
    /* While their exist a layer, continue. */
    while(ml) {
        vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
        ml->layer->posNext = newPos;
        ml = ml->next;// Move to the next moving layer
    }
}

/* Change the color of the player's layer if he goes out of the fieldLayer */
void mlAdvanceColorChange(Layer *l) {
  while(l) {//While there exist a layer, Continue
    if(l->posNext.axes[1] < 0){//If the player goes above the field region.
      l->color = COLOR_VIOLET;//Turn the player Violet
    }
    if(l->posNext.axes[1] > screenHeight) {//If the player goes below the field region.
      l->color = COLOR_YELLOW;//Turn the player Yellow
    }
    if(l->posNext.axes[0] > screenWidth) {//If the player goes right, out of the field region.
      l->color = COLOR_RED;//Turn the player Red
    }
    if(l->posNext.axes[0] < 0){//If the player goes left, out of the field region.
      l->color = COLOR_GREEN;//Turn the player Green
    }
    l = l->next;//Traverse to the next layer
  }
}

/* Check if the player and any of its layers is touching the Item */
char collisions(Region *player, Region *itemFence){
  //Check to see if any of the layers is colliding with the item
  return !(player->topLeft.axes[0] > itemFence->botRight.axes[0] ||
	   player->botRight.axes[0] < itemFence->topLeft.axes[0] ||
	   player->topLeft.axes[1] > itemFence->botRight.axes[1] ||
	   player->botRight.axes[1] < itemFence->topLeft.axes[1]);
}

/*
  
*/

void moveShapes() {
  switch(itemStates){
  case 1:
    shapeCount++;
    itemLayer.color = COLOR_RED;
    itemDesign.color = COLOR_RED;
    itemLayer.pos.axes[0] = 30;
    itemLayer.pos.axes[1] = 40;
    itemDesign.pos.axes[0] = 30;
    itemDesign.pos.axes[1] = 30;
    layerGetBounds(&itemLayer, &itemFence);
    if(redrawShape){
      layerDraw(&itemLayer);
      redrawShape = 0;
    }
    break;    
  case 2:
    shapeCount++;
    itemLayer.color = COLOR_YELLOW;
    itemDesign.color = COLOR_YELLOW;
    itemLayer.pos.axes[0] = 60;
    itemLayer.pos.axes[1] = 30;
    itemDesign.pos.axes[0] = 60;
    itemDesign.pos.axes[1] = 20;
    layerGetBounds(&itemLayer, &itemFence);
    if(redrawShape){
      layerDraw(&itemLayer);
    drawString5x7(0, screenHeight-16, "                     ",COLOR_WHITE,COLOR_BLACK);
    drawString5x7(0, screenHeight-8, "OBTAINED RED SHAPE",COLOR_WHITE,COLOR_BLACK);
      redrawShape = 0;
    }
    break;
  case 3:
    shapeCount++;
    itemLayer.color = COLOR_GREEN;
    itemDesign.color = COLOR_GREEN;
    itemLayer.pos.axes[0] = 80;
    itemLayer.pos.axes[1] = 20;
    itemDesign.pos.axes[0] = 80;
    itemDesign.pos.axes[1] = 10;
    layerGetBounds(&itemLayer, &itemFence);
    if(redrawShape){
      layerDraw(&itemLayer);
      redrawShape = 0;
        drawString5x7(0, screenHeight-16, "                     ",COLOR_WHITE,COLOR_BLACK);
    drawString5x7(0, screenHeight-8, "OBTAINED YELLOW SHAPE",COLOR_WHITE,COLOR_BLACK);
    }
    break;
  case 4:
    shapeCount++;
    itemLayer.color = COLOR_VIOLET;
    itemDesign.color = COLOR_VIOLET;
    itemLayer.pos.axes[0] = 30;
    itemLayer.pos.axes[1] = 130;
    itemDesign.pos.axes[0] = 30;
    itemDesign.pos.axes[1] = 120;
    layerGetBounds(&itemLayer, &itemFence);
    if(redrawShape){
      layerDraw(&itemLayer);
      redrawShape = 0;
        drawString5x7(0, screenHeight-16, "                     ",COLOR_WHITE,COLOR_BLACK);
    drawString5x7(0, screenHeight-8, "OBTAINED GREEN SHAPE",COLOR_WHITE,COLOR_BLACK);
    }
    break;
  case 5:
    shapeCount++;
    itemLayer.color = COLOR_GREEN;
    itemDesign.color = COLOR_GREEN;
    itemLayer.pos.axes[0] = 50;
    itemLayer.pos.axes[1] = 140;
    itemDesign.pos.axes[0] = 50;
    itemDesign.pos.axes[1] = 130;
    layerGetBounds(&itemLayer, &itemFence);
    if(redrawShape){
      layerDraw(&itemLayer);
        drawString5x7(0, screenHeight-16, "                     ",COLOR_WHITE,COLOR_BLACK);
    drawString5x7(0, screenHeight-8, "OBTAINED VIOLET SHAPE",COLOR_WHITE,COLOR_BLACK);
      redrawShape = 0;
    }
    break;
  }
}

void collisionAftermath(Region *playerFence, Region *itemFence){
    static int controls = 1;
    char collide = collisions(playerFence, itemFence);
    if(collide && layer5.color == itemLayer.color){
      itemStates++;   
      redrawShape++;
      moveShapes();
      collide = 0;
    }
    if(controls == 1){
      drawString5x7(0, screenHeight-16, "CONTROLS:",COLOR_WHITE,COLOR_BLACK);
      drawString5x7(0, screenHeight-8, "<      ^      v     >",COLOR_WHITE,COLOR_BLACK);   
      controls--;
    }
    if(collide && layer5.color != itemLayer.color){
        drawString5x7(0, screenHeight-16, "                     ",COLOR_WHITE,COLOR_BLACK);
        drawString5x7(0, screenHeight-8, "THINK OUTSIDE THE BOX",COLOR_WHITE,COLOR_BLACK);
        playerHelp = 1;
        death++;
    }
    if(itemStates > 6) {
        drawString5x7(0, screenHeight/2, "CONGRATS ON MATCHIN",COLOR_WHITE,COLOR_BLACK);
        drawString5x7(0, screenHeight/2+7, "ALL COLORS!",COLOR_WHITE,COLOR_BLACK);
        itemStates = 0;
        moveShapes();
    }
        if(death > 6) {
        drawString5x7(0, screenHeight/2, "TOO MANY MISTAKES",COLOR_WHITE,COLOR_BLACK);
        drawString5x7(0, screenHeight/2+7, "TRY AGAIN!",COLOR_WHITE,COLOR_BLACK);
        itemStates = 0;
        death = 0;
        moveShapes();
    }
}

  /* Move the player depending on the switches being pressed.*/
void movePlayer(){

  char select1 = !(p2sw_read() & (1<<0));//If SW1 is pressed
  char select2 = !(p2sw_read() & (1<<1));//If SW2 is pressed
  char select3 = !(p2sw_read() & (1<<2));//If SW3 is pressed 
  char select4 = !(p2sw_read() & (1<<3));//If SW4 is pressed
  //Check which button was pressed, and move the player accordingly
  x = 0;//Reset the player's X Velocity
  y = 0;//Reset the player's Y Velocity
  if(select1){
    x -= 4;
  }  
  if(select2){
    y -= 4;
  }  
  if(select3){
   y += 4;
  }  
  if(select4){
   x += 4;
  }
  Vec2 newVelocity = {x,y};//Create a new Vector, with the modified velocity
  (&ml5)->velocity = newVelocity;//Apply new Vector to each moving layer.
  (&ml4)->velocity = newVelocity;//Left Leg
  (&ml3)->velocity = newVelocity;
  (&ml2)->velocity = newVelocity;//Left Arm
  (&ml1)->velocity = newVelocity;
  (&ml0)->velocity = newVelocity;
  (&mlh)->velocity = newVelocity;
  layerGetBounds(&hitbox, &playerFence);
}

void wdt_c_handler() {
    static short count = 0;
    static short musicCount = 0;
    static short plrhelpCount = 0;
    if (count++ == 10) {
      movePlayer();//Move Player
      mlAdvance(&mlh);
      mlAdvanceColorChange(&layer0);//Check the color of the player accordingly
      collisionAftermath(&playerFence, &itemFence);//Collision detection with the player and itemFence
      redrawScreen = 1;
      count = 0;//Reset count
    } 
    if(plrhelpCount++ == 1000 && playerHelp){
        playerHelp = 0;
        plrhelpCount = 0;
    }
    if(musicCount++ == 100) {
      playMusic();//Play buzzer sounds
      musicCount = 0;
    }
}

void main() {
  configureClocks();
  buzzer_init();
  lcd_init();
  p2sw_init(15);
  layerInit(&hitbox);
  layerDraw(&hitbox);
  enableWDTInterrupts();
  redrawShape = 1;
  moveShapes();
  or_sr(0x8);
  while(1) {
    while (!redrawScreen) {
      or_sr(0x10);
    }
    redrawScreen = 0;
    movLayerDraw(&mlh, &hitbox);
  }
}
