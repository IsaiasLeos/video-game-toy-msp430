#ifndef buzzer_included
#define buzzer_included 

typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

void movLayerDraw(MovLayer *movLayers, Layer *layers);
void mlAdvance(MovLayer *ml);

#endif
