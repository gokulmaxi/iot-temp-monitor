#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SS_PIN D4
#define RST_PIN D3
int response;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
const int trigPin = D0;           //D4
const int echoPin = D8;           //D3
long duration;
int distance;
bool distanceState;
int led = 10;
double temp_amb;
double temp_obj;
float Fahrenheit;
int a = 1;
String tag = "";
const char *ssid = "IOT LAB"; //ENTER YOUR WIFI SETTINGS
const char *password = "IoT$2020";
const char *host = "10.10.110.2"; //https://circuits4you.com website or IP address of server
void setup()
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {

    delay(1000);
    Serial.print("Connecting..");
  }
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  Serial.println("RFID reading UID");
  mlx.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  Serial.println("IOT LAB-BIT");
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("IOT LAB");
  display.setCursor(25, 35);
  display.setTextSize(2);
  display.print("BIT");
  display.display();
  delay(1000);
}

int post_http(String data1, String data2)
{

  HTTPClient http; //Declare object of class HTTPClient
  String postData;
  //Post Data
  postData = "station=" + data1 + "&status=" + data2;
  http.begin("http://10.10.110.2/c4yforum/postdemo.php");              //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  int httpCode = http.POST(postData);                                  //Send the request
  String payload = http.getString();                                   //Get the response payload
  Serial.println(httpCode);                                            //Print HTTP return code
  Serial.println(payload);                                             //Print request response payload
  http.end();                                                          //Close connection
  return httpCode;
}

bool distance_measure()
{
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  if (distance < 10)
  {
    return true;
  }
  else
  {
    return false;
  }
}
void loop()
{
  display.clearDisplay();
  display.setCursor(25, 15);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("IOT LAB");
  display.setCursor(25, 35);
  display.setTextSize(2);
  display.print("BIT");
  display.display();
   if (mfrc522.PICC_IsNewCardPresent())
  {
    if (mfrc522.PICC_ReadCardSerial())
    {
      Serial.print("RFID NO:");
      Serial.print(a);
      Serial.print(" Tag UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        tag.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        tag.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      tag.toUpperCase();
      Serial.print(tag);
      while (!distance_measure())
      {
        Serial.print("come closer");
        Serial.print("come closer");
        display.clearDisplay();
        display.setCursor(35, 10);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.println("Come    closer");
        digitalWrite(led, LOW);
        display.display();
        delay(1000);
      }
      if (distance_measure())
      {
        Serial.print("Hold on");
        temp_amb = mlx.readAmbientTempF();
        temp_obj = mlx.readObjectTempF();
        display.clearDisplay();
        display.setCursor(25, 10);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.println("Temperature");
        display.setCursor(25, 30);
        display.setTextSize(2);
        display.print(temp_obj);
        display.print((char)247);
        display.print("F");
        display.display();
        delay(1000);
        int response = post_http(tag, String(temp_obj));
        tag = "";
        display.clearDisplay();
        display.setCursor(25, 15);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.println("OKAY");
        display.setCursor(25, 35);
        display.setTextSize(2);
        display.print("DONE");
        display.display();
        digitalWrite(led, HIGH);
        delay(500);
      }
    }
  }

  mfrc522.PICC_HaltA();
  a = a + 1;
}
