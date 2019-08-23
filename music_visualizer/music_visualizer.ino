#include <FastLED.h>

// Arduino Music Visualizer 0.3 - Shane Vincent, August 2019

// This music visualizer works off of analog input from a 3.5mm headphone jack
// Just touch jumper wire from A0 to tip of 3.5mm headphone jack

// The code is dynamic and can handle variable amounts of LEDs
// as long as you adjust NUM_LEDS according to the amount of LEDs you are using

// v0.3
// Adjusted for increased functionality
// Added visual style
// Added auto sensitivity checking


//DISABLE FOR LIVE PERFORMANCE
bool debug_mode = true;

// LED LIGHTING SETUP
#define LED_PIN     6
#define NUM_LEDS    60
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

// AUDIO INPUT SETUP
int audio_channel_1 = A0;
int audio_channel_2 = A1;

// STANDARD VISUALIZER VARIABLES
int loop_max = 0;
int k = 255; // COLOR WHEEL POSITION
int decay = 0; // HOW MANY MS BEFORE ONE LIGHT DECAY
int decay_check = 0;
long pre_react = 0; // NEW SPIKE CONVERSION
long react = 0; // NUMBER OF LEDs BEING LIT
long post_react = 0; // OLD SPIKE CONVERSION

// RAINBOW WAVE SETTINGS
int wheel_speed = 1; //default 3

// VINCE'S ADDED VARIABLES
long pre_brightness = 255;
long brightness = 255;
int brightness_decay_rate = 5;
int sensitivity = 10; //default 1
int palette_selector = 1;
long maximum_reading = 0;
int percent_effect = 0;
int rate_of_fade = 30;

// SET VISUAL STYLE HERE
// Visual styles:
// 0 = audio levels control number of LEDs displaying
// 1 = audio levels control brightness of LEDs displaying
int visual_style = 1;

// TODO:
// COLOUR PALETTES FOR DIFFERENT ARTISTS
int artist_selected = 0;
// Artists: 
// 0 - Vinnie
// 1 - Cody
// 2 - Ana

//Setup of arrays for Artist Palette Nodes

// palette_node data type
typedef struct palette_node
{
  CRGB palette;
  int node_position;
};

// initialization of vinnie's palette nodes
palette_node vinnie_palette[2];


/*
CRGB vinnie_palette[2];
vinnie_palette[0] = CRGB(3, 252, 207);
vinnie_palette[1] = CRGB(223, 3, 252);
int nodes_vinnie[2];*/

/*
byte[] vinnie_palette1 = [3, 252, 207];
byte[] vinnie_palette2 = [223, 3, 252];
byte[] cody_palette = [0,0,0];
byte[] ana_palette = [0,0,0];
*/


int palette_nodes = 0;
int node_displacement = 0;

void setup()
{
  // LED LIGHTING SETUP
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  // CLEAR LEDS
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB(0, 0, 0);
  FastLED.show();

  // Setting up artist's palette nodes
  vinnie_palette[0].palette = new CRGB(3, 252, 207);
  vinnie_palette[1].palette = new CRGB(223, 3, 252);

  //Generate a number of nodes somewhere between 2-5?
  palette_nodes = 2; //Will be set to 2 for testing
  
  //Randomly place those nodes across the strip
  // TODO: add some randomness (+- 40%?) to the displacement
  node_displacement = NUM_LEDS / palette_nodes+1; // Will set static displacement 

  if (artist_selected = 0)
  {
    //Vinnie selected

    for (int i = 0; i < palette_nodes; i++)
    {
      vinnie_palette[i].node_position = node_displacement*i;
    }
  }

  // SERIAL AND INPUT SETUP
  Serial.begin(115200);
  pinMode(audio_channel_1, INPUT);
  pinMode(audio_channel_2, INPUT);
  Serial.println("\nListening...");
}

// FUNCTION TO GENERATE COLOR BASED ON VIRTUAL WHEEL
// https://github.com/NeverPlayLegit/Rainbow-Fader-FastLED/blob/master/rainbow.ino

CRGB Scroll(int pos) {
  CRGB color (0,0,0);
  if(pos < 85) {
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = (255 - color.r);
  } else if(pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = (255 - color.g);
    color.b = 0;
  } else if(pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = (255 - color.b);
    color.r = 1;
  }
  return color;
}

// FUNCTION TO GET AND SET COLOR
// THE ORIGINAL FUNCTION WENT BACKWARDS
// THE MODIFIED FUNCTION SENDS WAVES OUT FROM FIRST LED
// https://github.com/NeverPlayLegit/Rainbow-Fader-FastLED/blob/master/rainbow.ino
void rainbow(int brightness)
{
  for(int i = NUM_LEDS - 1; i >= 0; i--) {
    if (i < react)
      leds[i] = Scroll((i * 256 / 50 + k) % 256);
    //else
      //leds[i] = CRGB(0, 0, 0);      
  }
  FastLED.show(); 
}

void artistPalette(int brightness) {
  
  
  //Node centres will have a the exact colour from the palettes
  //Between nodes will have fade between the two colours
}

void loop()
{
  int audio_input_channel_1 = analogRead(audio_channel_1)*sensitivity; 
  int audio_input_channel_2 = analogRead(audio_channel_2)*sensitivity;  

  //TODO: Disable for live performance as this decreases frame rate
  if (debug_mode)
  {
    Serial.print("DEBUGGING: Visual style: ");
    Serial.print(visual_style);
    Serial.print(" | Serial read: ");
    Serial.print(" CHANNEL 1: ");
    Serial.print(audio_input_channel_1);
    Serial.print(" CHANNEL 2: ");
    Serial.print(audio_input_channel_2);
  
    if (maximum_reading < audio_input_channel_1) maximum_reading = audio_input_channel_1;
  
    Serial.print(" | Maximum read: ");
    Serial.print(maximum_reading);
  
    percent_effect = audio_input_channel_1 / (maximum_reading/100);
  
    Serial.print(" | % effect: ");
    Serial.print(percent_effect);  
  }


  if ((audio_input_channel_1 > 0) && (visual_style == 0))
  {
    // Visual style 0 will display a number of LEDs according to recorded audio input
    // Audio level (as a percentage of maximum recorded audio level) translated to LEDs
    pre_react = (percent_effect * NUM_LEDS)/100;

    // To decrease rapid stuttering, the only way the react value will reduce is due to decay
     if (pre_react > react) // ONLY ADJUST LEVEL OF LED IF LEVEL HIGHER THAN CURRENT LEVEL
      react = pre_react;

    //brightness is 255 by default and will stay that way unless adjusted

    if (debug_mode) 
    {
      Serial.print(" | react: ");
      Serial.print(react);
    }
  } else if ((audio_input_channel_1 > 0) && (visual_style == 1))
  {
    //Visual Style 1 will display ALL LEDs and adjust brightness according to audio input
    //100% of LEDs will be displaying
    react = NUM_LEDS;

    // Brightness level is the current audio input (as a percentage of maximum recorded input) * 255 (max brightness is 255)
    pre_brightness = (percent_effect * 255)/100;

    // To decrease rapid stuttering, the only way the brightness value will reduce is due to decay
    if (pre_brightness > brightness)
      brightness = pre_brightness;

    if (debug_mode)
    {
      Serial.print(" | brightness: ");
      Serial.print(brightness);
    }
  }

  //Cycle between "settings here"
  //TODO: Add artist palette cycle
  //TODO: Very slow bleed between cycles
  fadeToBlackBy(leds, NUM_LEDS, rate_of_fade);
  FastLED.setBrightness(brightness);
  rainbow(brightness); // APPLY COLOR

  //If debug printing, start a new line
  if (debug_mode) Serial.println();

  k = k - wheel_speed; // SPEED OF COLOR WHEEL
  if (k < 0) // RESET COLOR WHEEL
    k = 255;

  // REMOVE LEDs
  
  decay_check++;
  if (decay_check > decay)
  {
    decay_check = 0;
    if (react > 0 && visual_style == 0)
      //React should only decay if we are using visual style 0
      react--;
    else if (brightness > 0 && visual_style == 1)
      //Brightness will decay if using visual style 1
      //Brightness should decay at an increased rate
      brightness = brightness - brightness_decay_rate;
  }
  
  //delay(1);
}
