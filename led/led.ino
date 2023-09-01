
#include <Adafruit_NeoPixel.h>

#define PIN 2
#define NUMPIXELS 10

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    pixels.begin();
    pixels.clear();
    for (size_t i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0xff, 0xff, 0xff));
    }
    pixels.show();
}

void loop()
{
}
