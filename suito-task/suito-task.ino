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

#define WATER_FULL 0
#define WATER_Q3 1
#define WATER_HALF 2
#define WATER_Q1 3
#define WATER_EMPTY 4

uint8_t waterLevel = WATER_EMPTY;

// さわるな
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

// さわるな
void setup()
{
    M5.begin();
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

bool isUpdate = true;
int oldState = 0;
void loop()
{
    M5.update();
    if (M5.BtnA.wasPressed())
    {
        Serial.println("BtnA");
        sendLocation();
    }
    if (M5.BtnB.isPressed())
    {
        isUpdate = false;
    }
    if (M5.BtnB.isReleased())
    {
        isUpdate = true;
    }
    if (isUpdate)
    {
        auto state = waterLevel;
        if (state != oldState)
        {
            switch (state)
            {
            case WATER_EMPTY:
                Serial.println("WATER_EMPTY");
                break;
            case WATER_FULL:
                Serial.println("WATER_FULL");
                break;
            case WATER_HALF:
                Serial.println("WATER_HALF");
                break;
            case WATER_Q1:
                Serial.println("WATER_Q1");
                break;
            case WATER_Q3:
                Serial.println("WATER_Q3");
                break;
            }
        }
        oldState = state;
    }
    delay(1);
}

void tofTask(void *)
{
    while (true)
    {
        int distance = 0;
        for (size_t i = 0; i < 5; i++)
        {
            delay(10);
            auto read = readDistance();
            if (read == -1)
            {
                i--;
                continue;
            }
            distance += read;
        }
        distance /= 5;
        // M5.Lcd.setCursor(0, 0);
        // M5.Lcd.printf("%03d", distance);
        if (distance > 230)
        {
            continue;
        }
        if (distance > 190)
        {
            waterLevel = WATER_EMPTY;
            continue;
        }

        if (distance > 150)
        {
            waterLevel = WATER_Q1;
            continue;
        }

        if (distance > 130)
        {
            waterLevel = WATER_HALF;
            continue;
        }

        if (distance > 100)
        {
            waterLevel = WATER_Q3;
            continue;
        }
        waterLevel = WATER_FULL;
    }
}

// TOFセンサーから距離を読み込む
// さわるな
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
    if (!Wire.available())
    {
        return -1;
    }
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
