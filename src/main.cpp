#include "main.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);
int menu = 0;
int cnt = 0;

const char *ssid = "NGO LUOC 6G";
const char *password = "ngonuong";

char command[20];

void setup()
{
  pinMode(btn_down, INPUT);
  pinMode(btn_slc, INPUT);
  Serial.begin(9600);
  lcd.init();
  WiFi.begin(ssid, password);
  lcd.backlight();
  byte printedDots = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (printedDots++ > 15)
    {
      for (; printedDots >= 1; printedDots--)
      {
        lcd.setCursor(printedDots, 1);
        lcd.print(' ');
      }
      lcd.setCursor(1, 1);
      printedDots = 1;
    }
    else
      lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println("");
  lcd.print(F("Connecting wifi"));
  lcd.setCursor(0, 1);
  // lcd.clear();
  lcd.print("Ready");

  String macAddress = WiFi.macAddress();
  Serial.println("MAC Address: " + macAddress);

  beep(2);
}

void loop()
{
  switch (menu)
  {
  case 0:
    menu_tong();
    for (int i = 0; i < 100; i++)
    {
      if (check_btn1())
      {
        Serial.println("Button 1 Pressed");
        cnt++;
        break;
      }
      if (check_btn2())
      {
        Serial.println("Button 2 Pressed");
        if (cnt % 3 == 0)
        {
          menu = 1;
          cnt = 0;
        }
        else if (cnt % 3 == 1)
        {
          menu = 2;
          cnt = 0;
        }
        else if (cnt % 3 == 2)
        {
          menu = 3;
          cnt = 0;
        }
        break;
      }
      delay(10);
    }
    break;
  case 1:
    menu_diemdanh();
    for (int i = 0; i < 100; i++)
    {
      if (check_btn1())
      {
        cnt++;
        break;
      }
      if (check_btn2() || menu == 0)
      {
        menu = 0;
        cnt = 0;
        break;
      }
      else
      {
        strcpy(command, "DiemDanh");
        send_receive();
      }
      delay(10);
    }

    break;
  case 2:
    menu_them();
    for (int i = 0; i < 100; i++)
    {
      if (check_btn1())
      {
        cnt++;
        break;
      }
      if (check_btn2() || menu == 0)
      {
        menu = 0;
        cnt = 0;
        break;
      }
      else
      {
        strcpy(command, "Them");
        send_receive();
      }
      delay(10);
    }
    break;
  case 3:
    menu_xoa();
    for (int i = 0; i < 100; i++)
    {
      if (check_btn1())
      {
        cnt++;
        break;
      }
      if (check_btn2() || menu == 0)
      {
        menu = 0;
        cnt = 0;
        break;
      }
      else
      {
        strcpy(command, "Xoa");
        send_receive();
      }
      delay(10);
    }
    break;
  default:
    break;
  }
}
