#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

#define D15 15
#define LED_NUMBER 8

struct HSL
{
    int H;
    int S = 100;
    int L = 50;
};

struct RGB
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
};

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_NUMBER, D15, NEO_GRB + NEO_KHZ800);

void setup()
{
    // Start the Serial Monitor
    Serial.begin(115200);
    // Start the DS18B20 sensor
    sensors.begin();

    for (int i = 0; i < LED_NUMBER; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 100, 255));
        pixels.show();
    }
}

float HueToRGB(float v1, float v2, float vH)
{
    if (vH < 0)
        vH += 1;

    if (vH > 1)
        vH -= 1;

    if ((6 * vH) < 1)
        return (v1 + (v2 - v1) * 6 * vH);

    if ((2 * vH) < 1)
        return v2;

    if ((3 * vH) < 2)
        return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

    return v1;
}

RGB hslTORGB(HSL hsl)
{
    struct RGB rgb;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    if (hsl.S == 0)
    {
        r = g = b = (unsigned char)(hsl.L * 255);
    }
    else
    {
        float v1, v2;
        float hue = (float)hsl.H / 360;

        v2 = (hsl.L < 0.5) ? (hsl.L * (1 + hsl.S)) : ((hsl.L + hsl.S) - (hsl.L * hsl.S));
        v1 = 2 * hsl.L - v2;

        r = (unsigned char)(255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
        g = (unsigned char)(255 * HueToRGB(v1, v2, hue));
        b = (unsigned char)(255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
    }

    rgb.R = r;
    rgb.G = g;
    rgb.B = b;

    return rgb;
}

void loop()
{
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    float temperatureF = sensors.getTempFByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    Serial.print(temperatureF);
    Serial.println("ºF");

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    // for (int i = 0; i < 2; i++)
    // {
    //     for (int j = 0; j < 2; j++)
    //     {
    //         for (int k = 0; k < 2; k++)
    //         {
    // delay(200);                                                       // Delay for a period of time (in milliseconds).
    //         }
    //     }
    // }

    int minTemp = 23, maxTmep = 29;

    float temperatureScale = 1 - max(min((temperatureC - minTemp) / (maxTmep - minTemp), 1.0f), 0.0f);

    float hueVal = temperatureScale * 255;
    struct HSL hsl;
    hsl.H = (int)hueVal / 1;
    hsl.S = 100;
    hsl.L = 50;

    struct RGB rgb = hslTORGB(hsl);

    for (int i = 0; i < LED_NUMBER; i++)
    {
        pixels.setPixelColor(i, pixels.Color(rgb.R, rgb.G, rgb.B)); // Moderately bright green color.
    }
    pixels.show(); // This sends the updated pixel color to the hardware.

    Serial.printf("HSL %f %f %f\n", hsl.H, hsl.S, hsl.L);
    Serial.printf("RGB %d %d %d\n", rgb.R, rgb.G, rgb.B);
    Serial.printf("temperature scale %f\n", temperatureScale);


    delay(200);
}