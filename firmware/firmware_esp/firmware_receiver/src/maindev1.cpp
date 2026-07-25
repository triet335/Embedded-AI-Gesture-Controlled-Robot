#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define LED_PIN 2

typedef struct
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t temp;
  int16_t gx;
  int16_t gy;
  int16_t gz;
} MPU_DATA;

volatile MPU_DATA rxData;
unsigned long lastRecvTime = 0;
bool isConnected = false;
unsigned long prevBlinkTime = 0;
bool ledState = false;

// Ký tự đồng bộ (Header) để STM32 biết đâu là điểm bắt đầu của gói tin
const uint8_t header[2] = {0xAA, 0xBB};

// Hàm callback nhận dữ liệu ESP-NOW từ C3 Mini
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(MPU_DATA))
  {
    memcpy((void *)&rxData, incomingData, sizeof(MPU_DATA));
    lastRecvTime = millis();
    isConnected = true;

    // --- GỬI DỮ LIỆU SANG STM32 QUA UART ---
    Serial2.write(header, 2);                            // Gửi 2 byte header trước
    Serial2.write((uint8_t *)&rxData, sizeof(MPU_DATA)); // Gửi 14 byte dữ liệu

    // Vẫn in ra màn hình PC (Serial 0) để theo dõi
    Serial.printf("AX=%6d AY=%6d AZ=%6d | TEMP=%6d\n", rxData.ax, rxData.ay, rxData.az, rxData.temp);
  }
}

void setup()
{
  Serial.begin(115200); // Serial nối với máy tính

  // Khởi tạo Serial2 nối với STM32 (Baudrate 115200, RX2=16, TX2=17)
  Serial2.begin(115200);

  delay(1000);
  WiFi.mode(WIFI_STA);
  // Ép ESP32 DevKit V1 sử dụng Wi-Fi Channel 1 để đồng bộ với mạch C3
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  delay(100);

  uint8_t newMac[] = {0x14, 0x33, 0x5C, 0x04, 0x43, 0x90};
  esp_wifi_set_mac(WIFI_IF_STA, newMac);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (esp_now_init() != ESP_OK)
    return;
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  Serial.println("He thong UART TX da san sang!");
}

void loop()
{
  if (millis() - lastRecvTime > 1000)
    isConnected = false;

  if (isConnected)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    if (millis() - prevBlinkTime >= 200)
    {
      prevBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
  delay(5);
}