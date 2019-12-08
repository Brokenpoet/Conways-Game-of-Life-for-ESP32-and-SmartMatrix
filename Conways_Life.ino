/*
 * Conway's "Life", adapted for ESP32
 */

#include <Adafruit_GFX.h>   // Core graphics library
//#include <SmartMatrix_GFX.h>
#include <SmartMatrix3.h>

//Use direct display wiring
#define GPIOPINOUT 0
 


#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

byte world[kMatrixWidth][kMatrixHeight][3];  // Set up 3 worlds, 0 is current, 1 is next generation summation, 2 is for stagnant endstate recognition
long density = 20; // Initial population density
int polltime; // Used to track generation time
int startTime;  //Used to track game length
const int defaultBrightness = (100*255)/100;        // full (100%) brightness
//const int defaultBrightness = (15*255)/100;       // dim: 15% brightness

const rgb24 defaultBackgroundColor = {0, 0, 0};
const rgb24 liveColor = {0,0xff,0xff};
const rgb24 deadColor = {0,0,0};
boolean same = false;  // Used to stack if the world is stagnant
int counter = 1;  // Tracks generations

void setup() {
      // initialize the digital pin as an output.
    matrix.addLayer(&backgroundLayer); // Game of Life exists in the background layer
    matrix.addLayer(&scrollingLayer);  // Used for Game Over
    matrix.addLayer(&indexedLayer);
    matrix.begin();
  
    matrix.setBrightness(defaultBrightness);
  
    scrollingLayer.setOffsetFromTop(5);
    scrollingLayer.setColor({0xff, 0, 0});
    scrollingLayer.setMode(wrapForward);
    scrollingLayer.setSpeed(40);
    scrollingLayer.setFont(font6x10);
        
    backgroundLayer.enableColorCorrection(true);
    backgroundLayer.fillScreen({0,0,0});

   randomSeed(analogRead(0));
   randomscreen();  // Set an initial random population on the screen
   world[1][1][2]=1; // Set a default value on world 2
   startTime = millis();
 }

void loop() {

 if (millis()-polltime >=50)  { // Run a generation every 50 ms
  displayoutput(); // Run the next generation
  backgroundLayer.swapBuffers(); // Update the screen
  polltime=millis();
    if (same || (millis()-startTime >= 120000)){  // If the pattern has stagnated or if 2 minutes have gone by
      scrollingLayer.start("Game Over", 1);
      sleep(4);
      backgroundLayer.fillScreen({0,0,0});
      backgroundLayer.swapBuffers();
      ESP.restart();
    }
  }
}

// This function calculates the sum of each cell's neighbors and puts it in the home cell's spot in the matrix
int neighbours(int x, int y) {
 return
         world[(x + 1)        % kMatrixWidth][  y                   ]        [0] +
         world[ x                          ][ (y + 1)        % kMatrixHeight] [0] +
         world[(x + kMatrixWidth - 1) % kMatrixWidth][  y                   ]       [0] +
         world[ x                          ][ (y + kMatrixHeight - 1) % kMatrixHeight] [0] +
         world[(x + 1)        % kMatrixWidth][ (y + 1)        % kMatrixHeight] [0] +
         world[(x + kMatrixWidth - 1) % kMatrixWidth][ (y + 1)        % kMatrixHeight] [0] +
         world[(x + kMatrixWidth - 1) % kMatrixWidth][ (y + kMatrixHeight - 1) % kMatrixHeight] [0] +
         world[(x + 1) % kMatrixWidth       ][ (y + kMatrixHeight - 1) % kMatrixHeight] [0];
}

// This function establishes a random starting position at the defined population density
void randomscreen() {
    for (int y = 0; y < kMatrixHeight; y++) {
    for (int x = 0; x < kMatrixWidth; x++) {
      if (random(100) < density) {world[x][y][0] = 1; }
      else                       {world[x][y][0] = 0;}
      world[x][x][1] = 0;
    }
  }
 }

// This function generates the next generation and checks for stagnation
void displayoutput(){
  for (int y = 0; y < kMatrixHeight; y++) {
    for (int x = 0; x < kMatrixWidth; x++) {
      // Default is for cell to stay the same
      world[x][y][1] = world[x][y][0];
      int count = neighbours(x, y);
      if (count == 3 && world[x][y][0] == 0)               { world[x][y][1] = 1;backgroundLayer.drawPixel(x, y, liveColor); } //new cell
      if ((count < 2 || count > 3) && world[x][y][0] == 1) { world[x][y][1] = 0;backgroundLayer.drawPixel(x, y, deadColor); } //cell dies
    }
  }
  // swap next generation into place
  if (counter % 10 == 0){same = true;}  // Check for stagnation every 10 generations, has to be an even number
  for (int y = 0; y < kMatrixHeight; y++) {
    for (int x = 0; x < kMatrixWidth; x++) {
      world[x][y][0] = world[x][y][1];  // Sets the currnt generation equal to the new generation
      // Stagnation check
      if (counter % 10 == 0){
        if (world[x][y][0] != world[x][y][2]){
          same = false;  // If any cells in the matrix are different then the game continues
            }
        world[x][y][2]=world[x][y][0]; // Set the stagnation generation equal to the current generation
          }  
        }
     }
   counter++;  // Increment generation counter
  }
