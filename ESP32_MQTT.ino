#if defined(ESP8266) //deklarasi library WiFi karena programnya dibuat universal untuk board ESP8266 dan board ESP32 maka menggunakan if untuk dicek terlebih dahulu jenis board yang digunakan
#include <ESP8266WiFi.h> //apabila menggunakan board ESP8266 menggunakan library ESP8266WiFi.h
#elif defined(ESP32)
#include <WiFi.h>
#endifs
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#define TFT_DC 2
#define TFT_CS 15

#include <ThingsBoard.h>
#include "DHT.h"
#define DHTPIN 12
#define DHTTYPE DHT22
#define LDR 34
const float GAMMA= 0.7;
const float RL10 = 50;
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4); //alamat i2c 0x27
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

DHT dht (DHTPIN, DHTTYPE);

const char *ssid = "Wokwi-GUEST";  // bisa #define WIFI_SSID "Wokwi-GUEST"
const char *password = "";  // bisa #define WIFI_PASSWORD ""
#define TOKEN "DimasFikri" //access token
const char* mqtt_server = "thingsboard.cloud"; //bisa #define mqtt_server "thingsboard.cloud"

WiFiClient espClient;
PubSubClient client(espClient); // Deklarasi untuk mengubah perintah Thingsboard menjadi "client" dan memasukkan perintah WiFiClient yang sudah diubah menjadi espClient ke dalam library Thingsboard
int status = WL_IDLE_STATUS;

void setup()
{
  Serial.begin(9600);
  dht.begin(); //inisialisasi sensor dht
  delay(10);
  tft.begin(); //set tft ILI91341
  tft.setCursor(24, 80); //set tampilan awal
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println("MONITORING");
  tft.setCursor(30, 45);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println("Real Time");
  lcd.begin (20,4);  // Set lcd 
  lcd.init();
  lcd.backlight();
  pinMode(LDR, INPUT);
Serial.print("connect to: ");
Serial.println(ssid);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED)
{
  delay(500);
  Serial.print("...");
}
Serial.print("\n");
Serial.print("IP Address: ");
Serial.print(WiFi.localIP());
Serial.print("\n");
Serial.print("connect to: ");
Serial.println(ssid);
client.setServer( mqtt_server, 1883); // port default mqtt 1883

}

void loop() {
  if (!client.connected())
{
  reconnect();
}
getData();
delay(2000);

}

void getData()
{
  Serial.println("mengambil data sensor...");
  int analogDHT = analogRead(DHTPIN);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
lcd.setCursor(0, 0);
    lcd.print("Suhu : ");
    lcd.print(temperature);
lcd.setCursor(0, 1);
    lcd.print("kelembaban: ");
    lcd.print(humidity);
    lcd.print("  ");
tft.setCursor(20, 120);
    tft.setTextColor(ILI9341_BLUE);
    tft.setTextSize(3);
    tft.print("Suhu: ");
    tft.println(temperature);
tft.setCursor(20, 160);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.print("Kelembaban: ");
    tft.println(humidity);
    tft.print(" ");

  Serial.println(temperature);
  Serial.println(humidity);
  String payload1 = "{";
  payload1 +="\"Suhu\":";payload1+=temperature;
  payload1 +="}";

  char attribute[1000];
  payload1.toCharArray (attribute, 1000);
  client.publish("v1/devices/me/telemetry", attribute); //perintah kirim data temperature ke thingsboard telemetry
  client.publish("v1/devices/me/attributes", attribute);
  Serial.print("Suhu : ");
  Serial.print(temperature);
  Serial.print(" Kelembaban : ");
  Serial.println(humidity);

  String payload2 = "{";
  payload2 +="\"Kelembaban\":";payload2+=humidity;
  payload2 +="}";
  char Attribute[1000];
  payload2.toCharArray (Attribute, 1000);
  client.publish("v1/devices/me/telemetry", Attribute); //perintah kirim data temperature ke thingsboard telemetry 
  client.publish("v1/devices/me/attributes", Attribute);

  int analogValue = analogRead(LDR);
  float voltage = (analogValue / 4095.)* 5;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow (RL10 * 1e3 * pow(10, GAMMA) / resistance,(1 / GAMMA)); //lux ldr intensitas cahaya

lcd.setCursor(0, 2);
    lcd.print("LuxCahaya: ");
    lcd.print(lux);
lcd.setCursor(0, 3);
    lcd.print("-Dimasfikriar-");
  Serial.println(lux);
tft.setCursor(24, 200);
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    tft.print("LuxLDR: ");
    tft.println(lux);
  String payload = "{";
  payload +="\"Intensitas Cahaya\":";payload+=lux;
  payload +="}";
  
  char attributes[1000]; //1000 karakter
  payload.toCharArray (attributes, 1000);
  client.publish("v1/devices/me/telemetry", attributes);
  client.publish("v1/devices/me/attributes", attributes);
  Serial.println(attributes);
  
}

void reconnect() {
  while (!client.connected()){
    status = WiFi.status();
    if (status !=WL_CONNECTED){
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connect to AP");
  }
  Serial.print("Connecting to Thingsboard node...");
  if(client.connect("1ba0fed0-2c6c-11ee-abc9-9bc9f8cd5049", TOKEN, "")){  //Device ID dan Access TOKEN Thingsboard
    Serial.println("[DONE]");
  }
  else{
    Serial.print("[Failed][rc = ");
    Serial.println(": Retrying in 2 seconds]" );
    delay(2000); // pengambilan data sensor setiap 2 detik 
  }
 }
}