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
    // 文字
    M5.begin();
    M5.Power.begin();
    M5.Lcd.setBrightness(200);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextFont(7);
    M5.Lcd.fillScreen(0x867d);
}

void loop()
{
    M5.update();
    M5.Lcd.setTextColor(WHITE, 0x867d); 

    int left = 5; // 何割残っているか
    showLeftDrink(left);
    
    if (M5.BtnA.wasPressed())
    {
        Serial.println("BtnA");
        sendLocation();
    }
    delay(500);
}

void showLeftDrink(int left)
{   
    M5.Lcd.fillRect(10, 60, 110, 30, BLUE); // 1つ目の矩形を表示

    // left の値に応じて追加の矩形を表示
    if (left >= 2) {
        M5.Lcd.fillRect(10, 95, 110, 30, BLUE);
    }
    if (left >= 3) {
        M5.Lcd.fillRect(10, 130, 110, 30, BLUE);
    }
    if (left >= 4) {
        M5.Lcd.fillRect(10, 165, 110, 30, BLUE);
    }
    if (left >= 5) {
        M5.Lcd.fillRect(10, 200, 110, 30, BLUE);
    }
}

// さわるな
void tofTask(void *)
{
    while (true)
    {
        Serial.println(readDistance());
        delay(1000);
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
