#define LAPTOP_MODE

#ifndef LAPTOP_MODE
#define MICRO_MODE
#endif


#ifdef LAPTOP_MODE
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_gfx/SDL2_gfxPrimitives.h>
SDL_Window *_window;
SDL_Renderer *renderer;
#endif

#ifdef MICRO_MODE
#include <Adafruit_NeoPixel.h>
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    11
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 50
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

// Indexes into the neighbor arrays for the left, straight and right next lines on the grid
const int LEFT = 0;
const int STRAIGHT = 1;
const int RIGHT = 2;

// The direction the user is trying to move the snake. Updated when the user is holding a button during a tick.
int direction = STRAIGHT;

void setup();
void loop();
void draw();
void updateStrip();
void h2rgb(float H, int& R, int& G, int& B);

int max(int a, int b){
  return a > b ? a : b;
}

int min(int a, int b) {
  return a < b ? a : b;
}

class Line {
  int _id;
  int _hue;
  int _startX;
  int _startY;
  int _endX;
  int _endY;
  public:
    Line* rightNeighbors[3] = { NULL, NULL, NULL };
    Line* leftNeighbors[3] = { NULL, NULL, NULL };
    Line(){}
    Line(int id, int startX, int startY, int endX, int endY) :
      _id(id),
      _startX(startX),
      _startY(startY),
      _endX(endX),
      _endY(endY) {

      _hue = -1;
    }
    int id() { return _id; }
    int hue() { return _hue; } // 0 - 360
    void setHue(int hue) { _hue = hue; }
    int startX() { return _startX; }
    int startY() { return _startY; }
    int endX() { return _endX; }
    int endY() { return _endY; }
    int leftX() { return min(_startX, _endX); }
    int rightX() { return max(_startX, _endX); }
    int R() {
      if (_hue == -1) return 80;
      int r=0, g=0, b=0;
      h2rgb(_hue / 360.0,r,g,b);
      return r;
    }
    int G() {
      if (_hue == -1) return 80;
      int r=0, g=0, b=0;
      h2rgb(_hue / 360.0,r,g,b);
      return g;
    }
    int B() {
      if (_hue == -1) return 80;
      int r=0, g=0, b=0;
      h2rgb(_hue / 360.0,r,g,b);
      return b;
    }
};

class Queue {
  static const int MAXSIZE = 32;  
  int _length = 0; 
  public:  
    Line* queue[MAXSIZE] = {};  
    int getLength() { return _length; }       
    bool isempty() {
      return _length == 0;
    }
    bool isfull() {
      return _length == MAXSIZE;
    }

    Line* head() {
      return queue[_length - 1];
    }

    Line* peekTail() {
      return queue[0];
    }

    Line* popTail() {    
      if(!isempty()) {
          Line* tail = queue[0];

          // Shift every line back one into the newly freed up space
          for (int i = 0; i < _length - 1; i++) {
            queue[i] = queue[i + 1];
          }
          _length = _length - 1;
          return tail;
      } else {
        return NULL;
          // printf("Could not retrieve data, Queue is empty.\n");
      }
    }

    void push(Line* data) {
      if(!isfull()) {   
          queue[_length] = data;
          _length = _length + 1;
      } else {
          // printf("Could not insert data, Queue is full.\n");
      }
    }
};

// Snake
class Snake {
  public:
    Queue body;
    int length = 1;
    Line* head() {
      return body.head();
    }
    // Which direction the snake is facing (i.e. where his head node is compared to his neck)
    bool facingRight = true;

    void move(int direction) {
      Line* tail = body.peekTail();

      if (facingRight) {
        body.push(head()->rightNeighbors[direction]);
      } else {
        body.push(head()->leftNeighbors[direction]);
      }

      // The neck is the segment directly following the head 
      Line* neck = body.queue[body.getLength() - 2];
      // if (neck != NULL) {
      if (facingRight) {
        // If we were previously facing right, we continue facing right if the left-most nodes don't match
        facingRight = head()->leftX() != neck->leftX();
      } else {
        // If we were previously facing left, we only switch to right if the left-most nodes match (we turned 90 degrees)
        facingRight = head()->leftX() == neck->leftX();
      }
      
      // Remove the last segment of the tail
      if (body.getLength() > length) {
         body.popTail();
      }
    }
};

Line lines[32] = {
  // Forward slash
  Line(0, 1,5,2,4), // Bottom left corner
  Line(1, 2,4,3,3),
  Line(2, 3,3,4,2),
  Line(3, 4,2,5,1),
  Line(4, 5,1,6,2),
  Line(5, 6,2,5,3),
  Line(6, 5,3,4,4),
  Line(7, 4,4,3,5),
  Line(8, 3,5,2,6),
  Line(9, 2,6,1,5),
  Line(10, 1,5,0,4),
  Line(11, 0,4,1,3),
  Line(12, 1,3,2,2),
  Line(13, 2,2,3,1),
  Line(14, 3,1,4,0),
  Line(15, 4,0,5,1),

  // Backslash
  Line(16, 1,1,2,2), // Top left corner
  Line(17, 2,2,3,3),
  Line(18, 3,3,4,4),
  Line(19, 4,4,5,5),
  Line(20, 5,5,4,6),
  Line(21, 4,6,3,5),
  Line(22, 3,5,2,4),
  Line(23, 2,4,1,3),
  Line(24, 1,3,0,2),
  Line(25, 0,2,1,1),
  Line(26, 1,1,2,0),
  Line(27, 2,0,3,1),
  Line(28, 3,1,4,2),
  Line(29, 4,2,5,3),
  Line(30, 5,3,6,4),
  Line(31, 6,4,5,5),
};

void initLine(int index, 
  int leftNeighborLeft, int leftNeighborStraight, int leftNeighborRight,
  int rightNeighborLeft, int rightNeighborStraight, int rightNeighborRight) {

  lines[index].leftNeighbors[LEFT] = &lines[leftNeighborLeft];
  lines[index].leftNeighbors[STRAIGHT] = &lines[leftNeighborStraight];
  lines[index].leftNeighbors[RIGHT] = &lines[leftNeighborRight];
  lines[index].rightNeighbors[LEFT] = &lines[rightNeighborLeft];
  lines[index].rightNeighbors[STRAIGHT] = &lines[rightNeighborStraight];
  lines[index].rightNeighbors[RIGHT] = &lines[rightNeighborRight];
}

void initLines() {
  initLine(0, 10,9,9, 23,1,22);
  initLine(1, 23,0,22, 17,2,18);
  initLine(2, 17,1,18, 28,3,29);
  initLine(3, 28,2,29, 15,15,4);
  initLine(4, 3,15,15, 5,5,5);
  initLine(5, 29,6,30, 4,4,4);
  initLine(6, 18,7,19, 29,5,30);
  initLine(7, 22,8,21, 18,6,19);
  initLine(8, 9,9,9, 22,7,21);
  initLine(9, 10,10,0, 8,8,8);
  initLine(10, 11,11,11, 9,9,0);
  initLine(11, 10,10,10, 24,12,23);
  initLine(12, 24,11,23, 16,13,17);
  initLine(13, 16,12,17, 27,14,28);
  initLine(14, 27,13,28, 15,15,15);
  initLine(15, 14,14,14, 3,4,4);
  initLine(16, 25,25,26, 12,17,13);
  initLine(17, 12,16,13, 1,18,2);
  initLine(18, 1,17,2, 7,19,6);
  initLine(19, 7,18,6, 20,31,31);
  initLine(20, 21,21,21, 19,31,31);
  initLine(21, 8,22,7, 20,20,20);
  initLine(22, 0,23,1, 8,21,7);
  initLine(23, 11,24,12, 0,22,1);
  initLine(24, 25,25,25, 11,23,12);
  initLine(25, 24,24,24, 26,26,16);
  initLine(26, 25,25,16, 27,27,27);
  initLine(27, 26,26,26, 13,28,14);
  initLine(28, 13,27,14, 2,29,3);
  initLine(29, 2,28,3, 6,30,5);
  initLine(30, 6,29,5, 31,31,31);
  initLine(31, 19,20,20, 30,30,30);
}

Snake snake;

int lineCount() {
  return sizeof(lines) / sizeof(Line);
}

#ifdef LAPTOP_MODE
int main(int argc, const char * argv[]) { // Only called for LAPTOP_MODE
  SDL_Init(SDL_INIT_VIDEO);
  _window = SDL_CreateWindow("PLAC-MAN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 500, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Event e;
  setup();
  bool quit = false;
  while (!quit){
    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){
        quit = true;
      }
      if (e.type == SDL_KEYDOWN){
        if (e.key.keysym.sym == SDLK_LEFT) {
          direction = LEFT;
          std::cout << "LEFT" << std::endl;
        } else if (e.key.keysym.sym == SDLK_RIGHT) {
          direction = RIGHT;
          std::cout << "RIGHT" << std::endl;
        }
      }
    }

    loop();
  }

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  SDL_DestroyWindow(_window);
  SDL_Quit();

  return 0;
}
#endif

void assignColors() {
  // Assign each light a default hue
  for (int i = 0; i < lineCount(); i++) {
    lines[i].setHue(-1);//i * (360 / lineCount()));
  }

  for (Line* l : snake.body.queue) {
    if (l != NULL) {
      l->setHue(200);
    }
  }
}

// Get the direction the user is trying to move the snake
int getDirection() {
  return direction;
}

void tick() {
  snake.move(getDirection());
  direction = STRAIGHT;
}

void setup() { // Called for both modes
  initLines();
  snake.body.push(&lines[0]);

#ifdef MICRO_MODE
  Serial.begin(9600);           // set up Serial library at 9600 bps
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
#endif
}

void delay_ms(int ms) {
#ifdef LAPTOP_MODE
  SDL_Delay(ms);
#endif
#ifdef MICRO_MODE
  delay(ms);
#endif
}

void loop() {
  assignColors();

  draw();
  updateStrip();


  delay_ms(500);

  tick();
    
  // Fill along the length of the strip in various colors...
  // colorWipe(strip.Color(255,   0,   0), 50); // Red
  // colorWipe(strip.Color(  0, 255,   0), 50); // Green
  // colorWipe(strip.Color(  0,   0, 255), 50); // Blue

  // // Do a theater marquee effect in various colors...
  // theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
  // theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
  // theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness

  // rainbow(10);             // Flowing rainbow cycle along the whole strip
  // theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
}

void draw(){
#ifdef LAPTOP_MODE
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    int scale = 80;

    for (Line l : lines) {
      thickLineRGBA(renderer, 
        l.startX() * scale, l.startY() * scale, l.endX() * scale, l.endY() * scale,
        15, l.R(), l.G(), l.B(), 255);
    }
    
    SDL_RenderPresent(renderer);
#endif
}





void updateStrip() {
#ifdef MICRO_MODE
  for (int i = 0 ; i < lineCount(); i++) {
    if (i < strip.numPixels()) {
      strip.setPixelColor(i, strip.Color(lines[i].R(), lines[i].G(), lines[i].B()));
    }
  }
  strip.show();
#endif
}


void h2rgb(float H, int& R, int& G, int& B) {

 int var_i;
 float S=1, V=1, var_1, var_2, var_3, var_h, var_r, var_g, var_b;

 if ( S == 0 )                       //HSV values = 0 ÷ 1
 {
   R = V * 255;
   G = V * 255;
   B = V * 255;
 }
 else
 {
   var_h = H * 6;
   if ( var_h == 6 ) var_h = 0;      //H must be < 1
   var_i = int( var_h ) ;            //Or ... var_i = floor( var_h )
   var_1 = V * ( 1 - S );
   var_2 = V * ( 1 - S * ( var_h - var_i ) );
   var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

   if      ( var_i == 0 ) { 
     var_r = V     ; 
     var_g = var_3 ; 
     var_b = var_1 ;
   }
   else if ( var_i == 1 ) { 
     var_r = var_2 ; 
     var_g = V     ; 
     var_b = var_1 ;
   }
   else if ( var_i == 2 ) { 
     var_r = var_1 ; 
     var_g = V     ; 
     var_b = var_3 ;
   }
   else if ( var_i == 3 ) { 
     var_r = var_1 ; 
     var_g = var_2 ; 
     var_b = V     ;
   }
   else if ( var_i == 4 ) { 
     var_r = var_3 ; 
     var_g = var_1 ; 
     var_b = V     ;
   }
   else                   { 
     var_r = V     ; 
     var_g = var_1 ; 
     var_b = var_2 ;
   }

   R = (1-var_r) * 255;                  //RGB results = 0 ÷ 255
   G = (1-var_g) * 255;
   B = (1-var_b) * 255;
 }
}
