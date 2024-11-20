#include "data.h"
#include <WiFi.h> // Thư viện WiFi để lấy địa chỉ MAC

extern LiquidCrystal_I2C lcd;
MFRC522 rfid(SS_PIN, RST_PIN);
Servo servo;
extern char command[20];

// Cấu hình URL của API server
const char *serverApiUrl = "http://192.168.100.20:8088/api/v1";
bool flag = false;
String macAddress; // Biến lưu địa chỉ MAC của thiết bị

void setUp()
{
  servo.setPeriodHertz(50);
  servo.attach(DOOR_PIN);
  servo.write(120); // Đảm bảo cửa đóng khi khởi động
  SPI.begin();
  rfid.PCD_Init();

  // Lấy địa chỉ MAC của thiết bị
  macAddress = WiFi.macAddress();
}

// Hàm xử lý và hiển thị thông báo lỗi lên LCD
void handleError()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error");
}

// Hàm xử lý và hiển thị thông báo thành công lên LCD
// Hàm xử lý và hiển thị thông báo thành công lên LCD
void handleSuccess()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Success");

  // Chỉ mở cửa nếu lệnh là "DiemDanh"
  if (strcmp(command, "DiemDanh") == 0)
  {
    int currentAngle = servo.read();
    Serial.print("Current servo angle: ");
    Serial.println(currentAngle);
    Serial.println("Command DiemDanh received");
    beep(5);

    // Mở cửa từ góc hiện tại về 0 độ (dần dần)
    for (int angle = currentAngle; angle >= 0; angle -= 5)
    {
      servo.write(angle); // Điều chỉnh góc servo
      delay(50);          // Thời gian chờ để servo di chuyển
    }
    Serial.println("Door opened");

    // Giữ cửa mở trong 5 giây
    delay(5000);

    // Đóng cửa từ góc hiện tại về 120 độ (dần dần)
    for (int angle = 0; angle <= 120; angle += 5)
    {
      servo.write(angle); // Điều chỉnh góc servo
      delay(50);          // Thời gian chờ để servo di chuyển
    }
    Serial.println("Door closed");
  }
}

// Hàm gửi yêu cầu đến server và kiểm tra phản hồi
void sendRequestToServer(const char *url, const char *method)
{
  HTTPClient http;
  int httpResponseCode;

  Serial.println(url);
  http.begin(url);

  if (strcmp(method, "POST") == 0)
  {
    httpResponseCode = http.POST(""); // Gửi POST với payload rỗng
  }
  else if (strcmp(method, "DELETE") == 0)
  {
    httpResponseCode = http.sendRequest("DELETE");
  }
  else
  {
    httpResponseCode = http.GET();
  }

  if (httpResponseCode == 200 || httpResponseCode == 201) // Kiểm tra mã phản hồi HTTP cho thành công
  {
    handleSuccess();
  }
  else
  {
    handleError();
  }
  http.end();
}

// Hàm gửi yêu cầu điểm danh với tham số URL và phương thức POST
void sendAttendanceRequest()
{
  char url[150];
  sprintf(url, "%s/attendance?rfidCode=", serverApiUrl);

  // Tạo chuỗi tham số, bao gồm địa chỉ MAC
  for (size_t i = 0; i < rfid.uid.size; i++)
  {
    sprintf(url + strlen(url), "%s%X", (i == 0 ? "" : ""), rfid.uid.uidByte[i]); // Append each byte as hex
  }
  sprintf(url + strlen(url), "&codeDevice=%s", macAddress.c_str()); // Append MAC address

  sendRequestToServer(url, "POST"); // Gửi yêu cầu với phương thức POST
}

// Hàm gửi yêu cầu thêm thẻ với tham số URL và phương thức POST
void sendAddCardRequest()
{
  char url[150];
  sprintf(url, "%s/rfid?rfidCode=", serverApiUrl);

  // Tạo chuỗi tham số, bao gồm địa chỉ MAC
  for (size_t i = 0; i < rfid.uid.size; i++)
  {
    sprintf(url + strlen(url), "%s%X", (i == 0 ? "" : ""), rfid.uid.uidByte[i]); // Append each byte as hex
  }
  sprintf(url + strlen(url), "&deviceCode=%s", macAddress.c_str()); // Append MAC address

  sendRequestToServer(url, "POST"); // Gửi yêu cầu với phương thức POST
}

// Hàm gửi yêu cầu xóa thẻ với tham số URL
void sendDeleteCardRequest()
{
  char url[150];
  sprintf(url, "%s/rfid/", serverApiUrl);

  // Tạo chuỗi tham số, bao gồm địa chỉ MAC
  for (size_t i = 0; i < rfid.uid.size; i++)
  {
    sprintf(url + strlen(url), "%s%X", (i == 0 ? "" : ""), rfid.uid.uidByte[i]); // Append each byte as hex
  }
  sprintf(url + strlen(url), "?deviceCode=%s", macAddress.c_str()); // Append MAC address

  sendRequestToServer(url, "DELETE");
}

// Hàm xử lý lệnh và gọi API tương ứng
void sendDataToServer()
{
  if (strcmp(command, "DiemDanh") == 0)
  {
    sendAttendanceRequest();
  }
  else if (strcmp(command, "Them") == 0)
  {
    sendAddCardRequest();
  }
  else if (strcmp(command, "Xoa") == 0)
  {
    sendDeleteCardRequest();
  }
}

void send_receive()
{
  if (!flag)
  {
    setUp();
    flag = !flag;
  }
  if (!rfid.PICC_IsNewCardPresent())
  {
    return;
  }
  if (!rfid.PICC_ReadCardSerial())
  {
    return;
  }
  beep(1);
  lcd.backlight();

  for (size_t i = 0; i < rfid.uid.size; i++)
  {
    Serial.printf("%X", rfid.uid.uidByte[i]);
  }
  Serial.println(command);

  // Gọi hàm để gửi dữ liệu đến server
  sendDataToServer();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  beep(1);
}