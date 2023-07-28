#include <Wire.h>
#include <M5Stack.h>

#define MTOF171000c0_ADDRESS 0x52

// #define I2C_PIN_SDA 21
// #define I2C_PIN_SCL 22

void setup()
{
    Serial.begin(115200);
    M5.begin();
    M5.Power.begin();
    M5.Lcd.setBrightness(200);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextFont(7);

    Wire.begin();
    while (!Serial)
    {
        delay(1);
    }
    xTaskCreatePinnedToCore(tofTask, "tofTask", 4096, NULL, 1, NULL, 1);
}

void loop()
{
    M5.update();
    delay(10);
}

void tofTask(void *)
{
    while (true)
    {
        Serial.println(readDistance());
        delay(1000);
    }
}

int readDistance()
{
    uint16_t distance;
    uint16_t distance_tmp;
    uint8_t data_cnt;
    Wire.beginTransmission(MTOF171000c0_ADDRESS);
    Wire.write(0xD3);
    Wire.endTransmission(false);
    Wire.requestFrom(MTOF171000c0_ADDRESS, 2);
    data_cnt = 0;
    distance = 0;
    distance_tmp = 0;
    while (Wire.available())
    {
        distance_tmp = Wire.read();
        distance = (distance << (data_cnt * 8)) | distance_tmp;
        data_cnt++;
    }
    return distance;
}
