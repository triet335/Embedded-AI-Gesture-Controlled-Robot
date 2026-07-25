#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

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

MPU_DATA txData;

// Địa chỉ MAC của mạch nhận (ESP32 DevKit V1)
uint8_t receiverMac[] = {0x14, 0x33, 0x5C, 0x04, 0x43, 0x90};

// Hàm callback thông báo trạng thái ESP-NOW
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("Trang thai gui: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Thanh cong" : "That bai");
}

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Khoi dong ESP32-C3 (Tay cam) ---");

  // Khởi tạo I2C ở chân 4 (SDA) và 5 (SCL)
  Wire.begin(4, 5);

  Serial.println("Dang khoi dong MPU6050 qua thanh ghi...");
  // Gửi lệnh đánh thức MPU6050 (Ghi số 0 vào thanh ghi PWR_MGMT_1 - 0x6B)
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0)
  {
    Serial.println("LOI: Khong the giao tiep voi MPU6050!");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 OK!");
  WiFi.mode(WIFI_STA);
  // Ép ESP32-C3 sử dụng Wi-Fi Channel 1
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("LOI: Khong the khoi tao ESP-NOW!");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("ESP-NOW OK!");

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("LOI: Khong the them Peer (Mach nhan)!");
    return;
  }
  Serial.println("Them Peer thanh cong. Bat dau gui du lieu...");
}

void loop()
{
  // Bắt đầu đọc 14 bytes từ thanh ghi 0x3B (ACCEL_XOUT_H)
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 14, true);

  // MPU6050 trả về 14 bytes (mỗi giá trị 16-bit gồm High Byte và Low Byte)
  if (Wire.available() == 14)
  {
    txData.ax = Wire.read() << 8 | Wire.read();
    txData.ay = Wire.read() << 8 | Wire.read();
    txData.az = Wire.read() << 8 | Wire.read();
    txData.temp = Wire.read() << 8 | Wire.read();
    txData.gx = Wire.read() << 8 | Wire.read();
    txData.gy = Wire.read() << 8 | Wire.read();
    txData.gz = Wire.read() << 8 | Wire.read();

    // Gửi dữ liệu ESP-NOW
    esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&txData, sizeof(txData));

    if (result != ESP_OK)
    {
      Serial.println("LOI: Loi truyen du lieu ESP-NOW");
    }
  }

  // Chậm lại một chút để tránh nghẽn băng thông
  delay(5);
}