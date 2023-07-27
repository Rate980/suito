#include <M5Stack.h>
void setup()
{
    M5.begin();
    M5.Power.begin();
    M5.Lcd.setBrightness(200);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextFont(7);
}

void loop()
{
    for (size_t i = 1; i <= 100; i++)
    {
        M5.Lcd.setCursor(0, 0);
        if (i < 100)
        {
            M5.Lcd.print("0");
        }
        if (i < 10)
        {
            M5.Lcd.print("0");
        }
        M5.Lcd.print(i);
        delay(400);
    }
}
