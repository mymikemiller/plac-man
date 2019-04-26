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

void loop();
void draw();
void h2rgb(float H, int& R, int& G, int& B);

class Line {
  int _id;
  int _hue;
  int _startX;
  int _startY;
  int _endX;
  int _endY;
  public:
    Line(int id, int startX, int startY, int endX, int endY) :
      _id(id),
      _startX(startX),
      _startY(startY),
      _endX(endX),
      _endY(endY) {

      _hue = 0;
    }
    int id() { return _id; }
    int hue() { return _hue; } // 0 - 360
    void setHue(int hue) { _hue = hue; }
    int startX() { return _startX; }
    int startY() { return _startY; }
    int endX() { return _endX; }
    int endY() { return _endY; }
    int R() {
      int r=0, g=0, b=0;
      h2rgb(_hue / 360.0,r,g,b);
      return r;
    }
    int G() {
      int r=0, g=0, b=0;
      h2rgb(_hue / 360.0,r,g,b);
      return g;
    }
    int B() {
      int r=0, g=0, b=0;
      h2rgb(_hue / 360.0,r,g,b);
      return b;
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

int lineCount() {
  return sizeof(lines) / sizeof(Line);
}

#ifdef LAPTOP_MODE
int main(int argc, const char * argv[]) { // Only called for LAPTOP_MODE
  SDL_Init(SDL_INIT_VIDEO);
  _window = SDL_CreateWindow("PLAC-MAN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 500, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Event e;
  bool quit = false;
  while (!quit){
    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT){
        quit = true;
      }
      if (e.type == SDL_KEYDOWN){
        quit = true;
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

void setup() { // Only called for MICRO_MODE
#ifdef MICRO_MODE
  Serial.begin(9600);           // set up Serial library at 9600 bps
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
#endif

  for (int i = 0; i < lineCount(); i++) {
    lines[i].setHue(i * (360 / lineCount()));
  }
}

void loop() {
  draw();
    
#ifdef MICRO_MODE  
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
  updateStrip();
#endif
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





#ifdef MICRO_MODE

void updateStrip() {
  for (int i = 0 ; i < lineCount(); i++) {
    if (i < strip.numPixels()) {
      strip.setPixelColor(i, strip.Color(lines[i].R(), lines[i].G(), lines[i].B()));
    }
  }
  strip.show();
}

// From Adafruit NeoPixel library example 'strandtest'
// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
#endif


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