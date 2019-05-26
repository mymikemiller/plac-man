#define LAPTOP_MODE

#ifndef LAPTOP_MODE
#define MICRO_MODE
#endif

#ifdef LAPTOP_MODE
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_gfx/SDL2_gfxPrimitives.h>
#include <chrono>
#include <sys/timeb.h>
SDL_Window *_window;
SDL_Renderer *renderer;
#endif

#ifdef MICRO_MODE
#include <Adafruit_NeoPixel.h>
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:d
#define LED_PIN 11
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 50
// The analog pins to control the rainbow center
#define DIAL_PIN_X 1
#define DIAL_PIN_Y 2
#define DIAL_PIN_SPEED 0
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

#include <math.h>

const byte actual_leds[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36};

// Indexes into the neighbor arrays for the left, straight and right next lines on the grid
const byte LEFT PROGMEM = 0;
const byte STRAIGHT PROGMEM = 1;
const byte RIGHT PROGMEM = 2;

// too: change to 256-based bytes to free up program memory
const int RED_HUE PROGMEM = 170;
const int GREEN_HUE PROGMEM = 300;
const int BLUE_HUE PROGMEM = 359;

// The direction the user is trying to move the snake. Updated when the user is holding a button during a tick.
byte direction = STRAIGHT;

// Whether we're showing a rainbow or plaing Snake
bool rainbow = true;

bool isRainbowMode()
{
#ifdef LAPTOP_MODE
  return rainbow;
#endif
#ifdef MICRO_MODE
  return digitalRead(6);
#endif
}

// If > 0, we are performing a loss animation. Upon reaching 0, we reset.
byte lossAnimation = 0;

void setup();
void loop();
void draw();
void updateStrip();
void h2rgb(float H, int &R, int &G, int &B);

int get_max(int a, int b)
{
  return a > b ? a : b;
}

int get_min(int a, int b)
{
  return a < b ? a : b;
}

class Line
{
  byte _id;
  word _hue;
  byte _startX;
  byte _startY;
  byte _endX;
  byte _endY;

public:
  Line *rightNeighbors[3] = {NULL, NULL, NULL};
  Line *leftNeighbors[3] = {NULL, NULL, NULL};
  Line() {}
  Line(byte id, byte startX, byte startY, byte endX, byte endY) : _id(id),
                                                                  _startX(startX),
                                                                  _startY(startY),
                                                                  _endX(endX),
                                                                  _endY(endY)
  {
    _hue = -1;
  }
  byte id() { return _id; }
  int hue() { return _hue; } // 0 - 360
  void setHue(int hue) { _hue = hue; }
  byte startX() { return _startX; }
  byte startY() { return _startY; }
  byte endX() { return _endX; }
  byte endY() { return _endY; }
  byte leftX() { return get_min(_startX, _endX); }
  byte rightX() { return get_max(_startX, _endX); }
  byte topY() { return get_min(_startY, _endY); }
  byte bottomY() { return get_max(_startY, _endY); }
  float centerX() { return _startX + (_endX - _startX) / 2.0; }
  float centerY() { return _startY + (_endY - _startY) / 2.0; }
  int R()
  {
    if (_hue == -1)
      return 20;
    int r = 0, g = 0, b = 0;
    h2rgb(_hue / 360.0, r, g, b);
    return r;
  }
  int G()
  {
    if (_hue == -1)
      return 20;
    int r = 0, g = 0, b = 0;
    h2rgb(_hue / 360.0, r, g, b);
    return g;
  }
  int B()
  {
    if (_hue == -1)
      return 20;
    int r = 0, g = 0, b = 0;
    h2rgb(_hue / 360.0, r, g, b);
    return b;
  }
};

// Returns 0-360 (0 is pointing to the right, 90 is up)
float getAngle(float fromX, float fromY, float toX, float toY)
{
  float angle = atan2f(toY - fromY, toX - fromX);
  float degrees = (angle * 180 / M_PI + 360);
  degrees = fmod(degrees, 360);
  return degrees;
}

class Queue
{
  static const byte MAXSIZE = 32;
  byte _length = 0;

public:
  Line *queue[MAXSIZE] = {};
  byte getLength() { return _length; }
  bool isempty()
  {
    return _length == 0;
  }
  bool isfull()
  {
    return _length == MAXSIZE;
  }
  void clear()
  {
    while (_length > 0)
    {
      popTail();
    }
  }

  Line *head()
  {
    return queue[_length - 1];
  }

  Line *peekTail()
  {
    return queue[0];
  }

  Line *popTail()
  {
    if (!isempty())
    {
      Line *tail = queue[0];

      // Shift every line back one into the newly freed up space
      for (byte i = 0; i < _length; i++)
      {
        queue[i] = queue[i + 1];
      }
      _length = _length - 1;
      return tail;
    }
    else
    {
      return NULL;
      // printf("Could not retrieve data, Queue is empty.\n");
    }
  }

  void push(Line *data)
  {
    if (!isfull())
    {
      queue[_length] = data;
      _length = _length + 1;
    }
    else
    {
      // printf("Could not insert data, Queue is full.\n");
    }
  }

  bool contains(Line *line)
  {
    for (Line *l : queue)
    {
      if (line == l)
      {
        return true;
      }
    }
    return false;
  }
};

Line lines[32] = {
    // Forward slash
    Line(0, 1, 5, 2, 4), // Bottom left corner
    Line(1, 2, 4, 3, 3),
    Line(2, 3, 3, 4, 2),
    Line(3, 4, 2, 5, 1),
    Line(4, 5, 1, 6, 2),
    Line(5, 6, 2, 5, 3),
    Line(6, 5, 3, 4, 4),
    Line(7, 4, 4, 3, 5),
    Line(8, 3, 5, 2, 6),
    Line(9, 2, 6, 1, 5),
    Line(10, 1, 5, 0, 4),
    Line(11, 0, 4, 1, 3),
    Line(12, 1, 3, 2, 2),
    Line(13, 2, 2, 3, 1),
    Line(14, 3, 1, 4, 0),
    Line(15, 4, 0, 5, 1),

    // Backslash
    Line(16, 1, 1, 2, 2), // Top left corner
    Line(17, 2, 2, 3, 3),
    Line(18, 3, 3, 4, 4),
    Line(19, 4, 4, 5, 5),
    Line(20, 5, 5, 4, 6),
    Line(21, 4, 6, 3, 5),
    Line(22, 3, 5, 2, 4),
    Line(23, 2, 4, 1, 3),
    Line(24, 1, 3, 0, 2),
    Line(25, 0, 2, 1, 1),
    Line(26, 1, 1, 2, 0),
    Line(27, 2, 0, 3, 1),
    Line(28, 3, 1, 4, 2),
    Line(29, 4, 2, 5, 3),
    Line(30, 5, 3, 6, 4),
    Line(31, 6, 4, 5, 5),
};

Line *cherry = NULL;

// Snake
class Snake
{
public:
  Queue body;
  byte length = 1;
  Line *head()
  {
    return body.head();
  }
  // Which direction the snake is facing (i.e. where his head node is compared to his neck)
  bool facingRight = true;
  bool facingUp = true;

  void grow()
  {
    length++;
  }
  void reset()
  {
    body.clear();
    body.push(&lines[0]);
    length = 1;
  }

  bool contains(Line *line)
  {
    return body.contains(line);
  }

  void move(int direction)
  {
    // Add a segment in the direction we're facing
    Line *newHead = NULL;
    if (facingRight)
    {
      if (facingUp)
      {
        newHead = head()->rightNeighbors[direction];
      }
      else
      {
        newHead = head()->rightNeighbors[direction == LEFT ? RIGHT : (direction == RIGHT ? LEFT : STRAIGHT)];
      }
    }
    else
    {
      if (facingUp)
      {
        newHead = head()->leftNeighbors[direction];
      }
      else
      {
        newHead = head()->leftNeighbors[direction == LEFT ? RIGHT : (direction == RIGHT ? LEFT : STRAIGHT)];
      }
    }

    // Set the new direction we're facing
    // The neck is the segment that directly follows the head
    Line *neck = body.queue[body.getLength() - 1];
    if (facingRight)
    {
      // If we were previously facing right, we continue facing right if the left-most nodes don't match
      facingRight = newHead->leftX() != neck->leftX();
    }
    else
    {
      // If we were previously facing left, we only switch to right if the left-most nodes match (we turned 90 degrees)
      facingRight = newHead->leftX() == neck->leftX();
    }
    if (facingUp)
    {
      // If we were previously facing up, we continue facing up if the top-most nodes don't match
      facingUp = newHead->topY() != neck->topY();
    }
    else
    {
      // If we were previously facing down, we only switch to up if the top-most nodes match (we turned 90 degrees)
      facingUp = newHead->topY() == neck->topY();
    }

    // Remove the last segment of the tail
    if (body.getLength() >= length)
    {
      body.popTail();
    }

    // Check for the lose condition
    if (body.contains(newHead))
    {
      // Reset the loss animation. It will play over the next x ticks.
      lossAnimation = 10;
    }
    else
    {
      // Add the new head
      body.push(newHead);
    }
  }
};

void initLine(byte index,
              byte leftNeighborLeft, byte leftNeighborStraight, byte leftNeighborRight,
              byte rightNeighborLeft, byte rightNeighborStraight, byte rightNeighborRight)
{

  lines[index].leftNeighbors[LEFT] = &lines[leftNeighborLeft];
  lines[index].leftNeighbors[STRAIGHT] = &lines[leftNeighborStraight];
  lines[index].leftNeighbors[RIGHT] = &lines[leftNeighborRight];
  lines[index].rightNeighbors[LEFT] = &lines[rightNeighborLeft];
  lines[index].rightNeighbors[STRAIGHT] = &lines[rightNeighborStraight];
  lines[index].rightNeighbors[RIGHT] = &lines[rightNeighborRight];
}

void initLines()
{
  initLine(0, 10, 9, 9, 23, 1, 22);
  initLine(1, 23, 0, 22, 17, 2, 18);
  initLine(2, 17, 1, 18, 28, 3, 29);
  initLine(3, 28, 2, 29, 15, 15, 4);
  initLine(4, 3, 15, 15, 5, 5, 5);
  initLine(5, 29, 6, 30, 4, 4, 4);
  initLine(6, 18, 7, 19, 29, 5, 30);
  initLine(7, 22, 8, 21, 18, 6, 19);
  initLine(8, 9, 9, 9, 22, 7, 21);
  initLine(9, 10, 10, 0, 8, 8, 8);
  initLine(10, 11, 11, 11, 9, 9, 0);
  initLine(11, 10, 10, 10, 24, 12, 23);
  initLine(12, 24, 11, 23, 16, 13, 17);
  initLine(13, 16, 12, 17, 27, 14, 28);
  initLine(14, 27, 13, 28, 15, 15, 15);
  initLine(15, 14, 14, 14, 3, 4, 4);
  initLine(16, 25, 25, 26, 12, 17, 13);
  initLine(17, 12, 16, 13, 1, 18, 2);
  initLine(18, 1, 17, 2, 7, 19, 6);
  initLine(19, 7, 18, 6, 20, 31, 31);
  initLine(20, 21, 21, 21, 19, 31, 31);
  initLine(21, 8, 22, 7, 20, 20, 20);
  initLine(22, 0, 23, 1, 8, 21, 7);
  initLine(23, 11, 24, 12, 0, 22, 1);
  initLine(24, 25, 25, 25, 11, 23, 12);
  initLine(25, 24, 24, 24, 26, 26, 16);
  initLine(26, 25, 25, 16, 27, 27, 27);
  initLine(27, 26, 26, 26, 13, 28, 14);
  initLine(28, 13, 27, 14, 2, 29, 3);
  initLine(29, 2, 28, 3, 6, 30, 5);
  initLine(30, 6, 29, 5, 31, 31, 31);
  initLine(31, 19, 20, 20, 30, 30, 30);
}

Snake snake;

byte lineCount()
{
  return sizeof(lines) / sizeof(Line);
}
int actualLedCount()
{
  return sizeof(actual_leds) / sizeof(byte);
}

int getRandomLineIndex()
{
#ifdef MICRO_MODE
  return random(0, lineCount() - 1);
#endif
  return rand() % (lineCount()) + 0;
}

int getRandomHue()
{
#ifdef MICRO_MODE
  return random(0, 359);
#endif
  return rand() % (359 + 1) + 0;
}

void randomizeCherry()
{
  cherry = &lines[getRandomLineIndex()];
  while (snake.contains(cherry))
  {
    cherry = &lines[getRandomLineIndex()];
  }
}

void randomizeColors()
{
  for (int i = 0; i < lineCount(); i++)
  {
    lines[i].setHue(getRandomHue());
  }
}

bool clockwiseRainbow = true;

// centerX and centerY are 0-1. hueOffset is from 0-360
void colorWheel(float centerX, float centerY, float hueOffset)
{
  // Normalize center. Line center is 3,3, far corner is 6,6.
  centerX = centerX * 6;
  centerY = centerY * 6;
  for (byte i = 0; i < lineCount(); i++)
  {
    // angle will be between 0 and 360
    float angle = getAngle(centerX, centerY, lines[i].centerX(), lines[i].centerY());
    float hue = fmod(360 + angle - hueOffset, 360);
    lines[i].setHue(hue);
  }
}

// Direction is 1-4 (moving left, up, down or right), offset is 0-359
// void orthagonalRainbowSwipe(int direction, int offset) {
//   int spread = 360;
//   int max = 6; // max x or y of line
//   for(int i = 0; i <= max; i++) {

//     int hue =

//     for(int lineI = 0; lineI < lineCount(); lineI++) {
//       Line* line = &lines[linesI];
//       if (direction == 0) { // left
//         if (line->startX() == i) {
//           line->setHue();
//         }
//       } else if (direction == 1) {// up
//         if (line->startY() == i) {

//         }
//       }
//     }
//   }
// }

#ifdef LAPTOP_MODE
int main(int argc, const char *argv[])
{ // Only called for LAPTOP_MODE
  SDL_Init(SDL_INIT_VIDEO);
  _window = SDL_CreateWindow("PLAC-MAN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 500, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Event e;
  setup();

  bool quit = false;
  while (!quit)
  {
    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
      {
        quit = true;
      }
      if (e.type == SDL_KEYDOWN)
      {
        if (e.key.keysym.sym == SDLK_LEFT)
        {
          direction = LEFT;
        }
        else if (e.key.keysym.sym == SDLK_RIGHT)
        {
          direction = RIGHT;
        }
        else if (e.key.keysym.sym == SDLK_SPACE)
        {
          rainbow = !rainbow;
        }
      }
    }

    loop();
  }

  if (renderer)
  {
    SDL_DestroyRenderer(renderer);
  }
  SDL_DestroyWindow(_window);
  SDL_Quit();

  return 0;
}
#endif

void assignColors()
{
  // Handle the loss case and early out first
  if (lossAnimation > 0)
  {
    for (byte i = 0; i < lineCount(); i++)
    {
      lines[i].setHue(lossAnimation % 2 == 0 ? RED_HUE : -1);
    }
    return;
  }

  // Assign each light a default hue
  for (byte i = 0; i < lineCount(); i++)
  {
    lines[i].setHue(-1); //i * (360 / lineCount()));
  }

  if (cherry != NULL)
  {
    cherry->setHue(RED_HUE);
  }

  int diff = BLUE_HUE - GREEN_HUE;
  int increment = diff / snake.length;
  int bodyHue = BLUE_HUE;
  for (Line *l : snake.body.queue)
  {
    if (l != NULL)
    {
      l->setHue(bodyHue);
      bodyHue -= increment;
    }
  }

  snake.head()->setHue(GREEN_HUE);
}

// Get the direction the user is trying to move the snake
int getDirection()
{
#ifdef MICRO_MODE
  bool leftButton = digitalRead(4);
  bool rightButton = digitalRead(5);
  if (leftButton)
  {
    Serial.println("LEFT");
    return LEFT;
  }
  if (rightButton)
  {
    Serial.println("RIGHT");
    return RIGHT;
  }
  return STRAIGHT;
#endif

  return direction;
}

int startedVisualizationMs = 0;

int getMilliCount()
{
#ifdef LAPTOP_MODE
  timeb tb;
  ftime(&tb);
  int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
  return nCount;
#else
  return millis();
#endif
}

int countdownToPollSpeed = 20; // Prevent polling every frame.
int msPerVizualizationRotation = 5000;
float getPercentThroughVisualization()
{
  int milliCount = getMilliCount();
  countdownToPollSpeed--;
  if (countdownToPollSpeed < 0)
  {
    msPerVizualizationRotation = getMillisPerVisualizationRevolution();
    countdownToPollSpeed = 20;
  }
  int nSpan = milliCount - startedVisualizationMs;
  if (nSpan > msPerVizualizationRotation)
  {
    startedVisualizationMs = milliCount;
    nSpan = 0;
  }

  int ms = nSpan % msPerVizualizationRotation; // ms is between 0 and msPerVizualizationRotation
  float percent = ms / (float)msPerVizualizationRotation;
  return percent;
}

int getMillisPerVisualizationRevolution()
{
#ifdef LAPTOP_MODE
  return 5000;
#endif
  float speedPercent = 1 - (analogRead(DIAL_PIN_SPEED) / 1024.0);
  int slowestSpeed = 500;
  int fastestSpeed = 10000;
  return speedPercent * (fastestSpeed - slowestSpeed) + slowestSpeed;
}

void tick()
{
  if (isRainbowMode())
  {
    float centerX = 0.5;
    float centerY = 0.5;
#ifdef MICRO_MODE
    centerX = analogRead(DIAL_PIN_X) / 1024.0;
    centerY = analogRead(DIAL_PIN_Y) / 1024.0;
#endif

    if (startedVisualizationMs == 0)
    {
      startedVisualizationMs = getMilliCount();
    }

    float percentThroughColorWheel = getPercentThroughVisualization();
    colorWheel(centerX, centerY, percentThroughColorWheel * 360);
  }
  else // Snake mode
  {
    if (lossAnimation <= 0)
    {
      snake.move(getDirection());
      direction = STRAIGHT;
      if (cherry == snake.head())
      {
        snake.grow();
        randomizeCherry();
      }
    }
    else
    {
      lossAnimation--;
      if (lossAnimation == 0)
      {
        // We're on the last frame of the loss animation
        snake.reset();
        randomizeCherry();
      }
    }
  }
}

void setup()
{ // Called for both modes
  initLines();
  snake.reset();
  randomizeCherry();

#ifdef MICRO_MODE
  Serial.begin(9600);       // set up Serial library at 9600 bps
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
  Serial.println(analogRead(A4));
  randomSeed(analogRead(A4));

  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  // Initialize analog pins
  pinMode(DIAL_PIN_X, INPUT_PULLUP);
  pinMode(DIAL_PIN_Y, INPUT_PULLUP);
  pinMode(DIAL_PIN_SPEED, INPUT_PULLUP);
#endif
}

void delay_ms(int ms)
{
#ifdef LAPTOP_MODE
  SDL_Delay(ms);
#endif
#ifdef MICRO_MODE
  delay(ms);
#endif
}

void loop()
{
  if (!isRainbowMode())
  {
    assignColors();
  }

  draw();
  updateStrip();

  if (!isRainbowMode())
  {
    delay_ms(lossAnimation > 0 ? 200 : 500);
  }

  tick();
}

void draw()
{
#ifdef LAPTOP_MODE
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  int scale = 80;

  for (Line l : lines)
  {
    thickLineRGBA(renderer,
                  l.startX() * scale, l.startY() * scale, l.endX() * scale, l.endY() * scale,
                  15, l.R(), l.G(), l.B(), 255);
  }

  SDL_RenderPresent(renderer);
#endif
}

void updateStrip()
{
#ifdef MICRO_MODE
  for (byte i = 0; i < actualLedCount(); i++)
  {
    if (i < strip.numPixels())
    {
      strip.setPixelColor(actual_leds[i], strip.Color(lines[i].R(), lines[i].G(), lines[i].B()));
    }
  }
  strip.show();
#endif
}

void h2rgb(float H, int &R, int &G, int &B)
{

  int var_i;
  float S = 1, V = 1, var_1, var_2, var_3, var_h, var_r, var_g, var_b;

  if (S == 0) //HSV values = 0 ÷ 1
  {
    R = V * 255;
    G = V * 255;
    B = V * 255;
  }
  else
  {
    var_h = H * 6;
    if (var_h == 6)
      var_h = 0;        //H must be < 1
    var_i = int(var_h); //Or ... var_i = floor( var_h )
    var_1 = V * (1 - S);
    var_2 = V * (1 - S * (var_h - var_i));
    var_3 = V * (1 - S * (1 - (var_h - var_i)));

    if (var_i == 0)
    {
      var_r = V;
      var_g = var_3;
      var_b = var_1;
    }
    else if (var_i == 1)
    {
      var_r = var_2;
      var_g = V;
      var_b = var_1;
    }
    else if (var_i == 2)
    {
      var_r = var_1;
      var_g = V;
      var_b = var_3;
    }
    else if (var_i == 3)
    {
      var_r = var_1;
      var_g = var_2;
      var_b = V;
    }
    else if (var_i == 4)
    {
      var_r = var_3;
      var_g = var_1;
      var_b = V;
    }
    else
    {
      var_r = V;
      var_g = var_1;
      var_b = var_2;
    }

    R = (1 - var_r) * 255; //RGB results = 0 ÷ 255
    G = (1 - var_g) * 255;
    B = (1 - var_b) * 255;
  }
}
