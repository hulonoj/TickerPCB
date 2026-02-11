#include "neoPixels.h"
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 14
#define NUM_PIXELS 10
#define DEFAULT_BRIGHTNESS 50

Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
NeoPixelManager neopixels;

// Static variables for fire effect
static byte fireHeat[NUM_PIXELS];

void NeoPixelManager::init() {
  strip.begin();
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  strip.show();
  
  currentPattern = PATTERN_NONE;
  patternColor = COLOR_WHITE;
  updateInterval = 50;  // Default 50ms between updates
  lastUpdate = 0;
  patternStep = 0;
}

void NeoPixelManager::setPattern(NeoPixelPattern pattern, uint32_t color) {
  currentPattern = pattern;
  if (color != 0) {
    patternColor = color;
  }
  patternStep = 0;  // Reset animation
  
  // Initialize pattern-specific variables
  if (pattern == PATTERN_FIRE) {
    memset(fireHeat, 0, sizeof(fireHeat));
  }
}

void NeoPixelManager::setSpeed(uint16_t interval_ms) {
  updateInterval = interval_ms;
}

void NeoPixelManager::clear() {
  currentPattern = PATTERN_NONE;
  strip.clear();
  strip.show();
}

void NeoPixelManager::setBrightness(uint8_t brightness) {
  strip.setBrightness(brightness);
  strip.show();
}

void NeoPixelManager::setAll(uint32_t color, uint8_t brightness = DEFAULT_BRIGHTNESS){
    strip.setBrightness(brightness);

    for (int i = 0; i < NUM_PIXELS; i++) {
        strip.setPixelColor(i,color);
    }
    strip.show();
}

// MAIN UPDATE FUNCTION - CALL THIS IN loop()
void NeoPixelManager::update() {
  unsigned long currentTime = millis();
  
  // Check if it's time to update
  if (currentTime - lastUpdate < updateInterval) {
    return;  // Not time yet, exit immediately
  }
  
  lastUpdate = currentTime;
  
  // Update current pattern
  switch (currentPattern) {
    case PATTERN_RAINBOW:
      updateRainbow();
      break;
    case PATTERN_THEATER_CHASE:
      updateTheaterChase();
      break;
    case PATTERN_SCANNER:
      updateScanner();
      break;
    case PATTERN_PULSE:
      updatePulse();
      break;
    case PATTERN_SPARKLE:
      updateSparkle();
      break;
    case PATTERN_FIRE:
      updateFire();
      break;
    case PATTERN_COLOR_WIPE:
      updateColorWipe();
      break;
    case PATTERN_RUNNING_LIGHTS:
      updateRunningLights();
      break;
    case PATTERN_NONE:
    default:
      break;
  }
}

// Rainbow pattern - one step per update
void NeoPixelManager::updateRainbow() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    int pixelHue = patternStep + (i * 65536L / NUM_PIXELS);
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  strip.show();
  
  patternStep += 256;  // Increment hue
  if (patternStep >= 65536) {
    patternStep = 0;  // Loop back
  }
}

// Theater chase - one step per update
void NeoPixelManager::updateTheaterChase() {
  strip.clear();
  for (int c = patternStep % 3; c < NUM_PIXELS; c += 3) {
    strip.setPixelColor(c, patternColor);
  }
  strip.show();
  
  patternStep++;
  if (patternStep >= 30) {  // 10 complete cycles
    patternStep = 0;
  }
}

// Scanner/KITT effect - one step per update
void NeoPixelManager::updateScanner() {
  strip.clear();
  
  int totalSteps = (NUM_PIXELS - 1) * 2;
  int pos = patternStep % totalSteps;
  
  // Forward or backward
  int pixelPos = (pos < NUM_PIXELS) ? pos : (totalSteps - pos);
  
  // Main pixel
  strip.setPixelColor(pixelPos, patternColor);
  
  // Trailing pixels (dimmer)
  uint8_t r = (patternColor >> 16) & 0xFF;
  uint8_t g = (patternColor >> 8) & 0xFF;
  uint8_t b = patternColor & 0xFF;
  
  if (pixelPos > 0) {
    strip.setPixelColor(pixelPos - 1, strip.Color(r/4, g/4, b/4));
  }
  if (pixelPos < NUM_PIXELS - 1) {
    strip.setPixelColor(pixelPos + 1, strip.Color(r/4, g/4, b/4));
  }
  
  strip.show();
  patternStep++;
}

// Pulse/breathing - one step per update
void NeoPixelManager::updatePulse() {
  uint8_t r = (patternColor >> 16) & 0xFF;
  uint8_t g = (patternColor >> 8) & 0xFF;
  uint8_t b = patternColor & 0xFF;
  
  // Use sine wave for smooth pulsing
  float brightness = (sin(patternStep * 0.05) + 1.0) / 2.0;  // 0.0 to 1.0
  
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(
      r * brightness,
      g * brightness,
      b * brightness
    ));
  }
  strip.show();
  
  patternStep++;
  if (patternStep >= 126) {  // One complete sine cycle
    patternStep = 0;
  }
}

// Sparkle - random pixels flash
void NeoPixelManager::updateSparkle() {
  // Fade all pixels
  for (int i = 0; i < NUM_PIXELS; i++) {
    uint32_t currentColor = strip.getPixelColor(i);
    uint8_t r = ((currentColor >> 16) & 0xFF) * 0.8;
    uint8_t g = ((currentColor >> 8) & 0xFF) * 0.8;
    uint8_t b = (currentColor & 0xFF) * 0.8;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  
  // Add new sparkle
  if (random(10) < 3) {  // 30% chance
    int pixel = random(NUM_PIXELS);
    strip.setPixelColor(pixel, strip.Color(255, 255, 255));
  }
  
  strip.show();
  patternStep++;
}

// Fire effect - one step per update
void NeoPixelManager::updateFire() {
  uint8_t cooling = 55;
  uint8_t sparking = 120;
  
  // Cool down every pixel
  for (int i = 0; i < NUM_PIXELS; i++) {
    fireHeat[i] = fireHeat[i] > cooling ? fireHeat[i] - cooling : 0;
  }
  
  // Heat from neighbors
  for (int k = NUM_PIXELS - 1; k >= 2; k--) {
    fireHeat[k] = (fireHeat[k - 1] + fireHeat[k - 2] + fireHeat[k - 2]) / 3;
  }
  
  // Random sparks
  if (random(255) < sparking) {
    int y = random(7);
    fireHeat[y] = fireHeat[y] + random(160, 255);
  }
  
  // Convert heat to colors
  for (int j = 0; j < NUM_PIXELS; j++) {
    byte temperature = fireHeat[j];
    byte t192 = round((temperature / 255.0) * 191);
    byte heatramp = t192 & 0x3F;
    heatramp <<= 2;
    
    if (t192 > 0x80) {
      strip.setPixelColor(j, strip.Color(255, 255, heatramp));
    } else if (t192 > 0x40) {
      strip.setPixelColor(j, strip.Color(255, heatramp, 0));
    } else {
      strip.setPixelColor(j, strip.Color(heatramp, 0, 0));
    }
  }
  strip.show();
  patternStep++;
}

// Color wipe - fill one by one
void NeoPixelManager::updateColorWipe() {
  if (patternStep < NUM_PIXELS) {
    strip.setPixelColor(patternStep, patternColor);
    strip.show();
    patternStep++;
  } else {
    patternStep = 0;  // Loop
    strip.clear();
  }
}

// Running lights - smooth wave
void NeoPixelManager::updateRunningLights() {
  uint8_t r = (patternColor >> 16) & 0xFF;
  uint8_t g = (patternColor >> 8) & 0xFF;
  uint8_t b = patternColor & 0xFF;
  
  for (int i = 0; i < NUM_PIXELS; i++) {
    int brightness = (int)(sin(i + patternStep * 0.1) * 127 + 128);
    strip.setPixelColor(i, strip.Color(
      (r * brightness) / 255,
      (g * brightness) / 255,
      (b * brightness) / 255
    ));
  }
  strip.show();
  
  patternStep++;
  if (patternStep >= 628) {  // ~2*PI*100 for smooth loop
    patternStep = 0;
  }
}