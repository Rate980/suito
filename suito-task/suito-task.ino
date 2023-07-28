#include <Wire.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#define MTOF171000c0_ADDRESS 0x52

// #define I2C_PIN_SDA 21
// #define I2C_PIN_SCL 22
WiFiMulti wifiMulti;
QueueHandle_t wifiQueue;

bool isWifiConnected()
{
    return wifiMulti.run() == WL_CONNECTED;
}

// line ボットに位置情報を送信する
// 今は適当な値を送っている
// さわるな
void sendLocation()
{
    uint8_t location = 2;
    auto res = xQueueSend(wifiQueue, &location, 0);
    if (res != pdTRUE)
    {
        Serial.println("Failed to send location");
    }
}

void setup()
{
    Serial.begin(115200);
    wifiMulti.addAP("maruyama", "marufuck");
    Wire.begin();
    wifiQueue = xQueueCreate(4, sizeof(uint8_t));
    while (!Serial)
    {
        delay(1);
    }
    xTaskCreatePinnedToCore(tofTask, "tofTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(wifiTask, "wifi", 8192, NULL, 1, NULL, 1);
}

void loop()
{
    M5.update();
    if (M5.BtnA.wasPressed())
    {
        Serial.println("BtnA");
        sendLocation();
    }
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

// さわるな
void wifiTask(void *)
{
    while (true)
    {
        uint8_t data;
        auto res = xQueueReceive(wifiQueue, &data, portMAX_DELAY);
        if (!isWifiConnected() || res != pdTRUE)
        {
            if (res == pdTRUE)
            {
                Serial.println("wifi is not connected");
            }
            delay(1);
            continue;
        }
        HTTPClient http;
        http.begin("https://suito.rate980.net/location");
        auto httpRes = http.POST(String(data));
        if (httpRes != 200)
        {
            Serial.println("http failed to send location");
        }
    }
}
