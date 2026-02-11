#ifndef NEO_PIXELS_H
#define NEO_PIXELS_H


#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#define NEOPIXEL_PIN 14
#define NUM_PIXELS 10
#define DEFAULT_BRIGHTNESS 50

// Pattern types
enum NeoPixelPattern {
  PATTERN_ALL_ON,
  PATTERN_NONE,
  PATTERN_RAINBOW,
  PATTERN_THEATER_CHASE,
  PATTERN_SCANNER,
  PATTERN_PULSE,
  PATTERN_SPARKLE,
  PATTERN_FIRE,
  PATTERN_COLOR_WIPE,
  PATTERN_RUNNING_LIGHTS
};


// NeoPixel manager class
class NeoPixelManager {
public:
  void init();
  void update();  // Call this in loop() - non-blocking
  void setPattern(NeoPixelPattern pattern, uint32_t color = 0);
  void setSpeed(uint16_t interval_ms);
  void clear();
  void setBrightness(uint8_t brightness);
  void setAll(uint32_t color, uint8_t brightness);
  
private:
  NeoPixelPattern currentPattern;
  uint32_t patternColor;
  uint16_t updateInterval;
  unsigned long lastUpdate;
  uint16_t patternStep;
  
  void updateRainbow();
  void updateTheaterChase();
  void updateScanner();
  void updatePulse();
  void updateSparkle();
  void updateFire();
  void updateColorWipe();
  void updateRunningLights();
};

extern NeoPixelManager neopixels;
extern Adafruit_NeoPixel strip;

// Predefined colors
#define COLOR_RED     strip.Color(255, 0, 0)
#define COLOR_GREEN   strip.Color(0, 255, 0)
#define COLOR_BLUE    strip.Color(0, 0, 255)
#define COLOR_YELLOW  strip.Color(255, 255, 0)
#define COLOR_CYAN    strip.Color(0, 255, 255)
#define COLOR_MAGENTA strip.Color(255, 0, 255)
#define COLOR_WHITE   strip.Color(255, 255, 255)
#define COLOR_ORANGE  strip.Color(255, 165, 0)
#define COLOR_PURPLE  strip.Color(128, 0, 128)

#endif