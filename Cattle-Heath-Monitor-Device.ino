#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DFRobot_MLX90614.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
//----------------------------
Adafruit_MPU6050 mpu;
WiFiClient client;
DFRobot_MLX90614_I2C mlx;  // instantiate an object to drive our sensor
//---------------------------
const char *server = "api.thingspeak.com";
const char *ssid = "";  // replace with your WiFi SSID and WPA2 key
const char *pass = "";
const char *apiKey = "27JKXX1XXAF7JULC";  // Enter your Write API key from ThingSpeak
unsigned long channelID = 2199351;        // Replace with your ThingSpeak channel ID
const char *host = "192.168.0.101";
int port = 5000;
// -------------------------
float ambientTemp_Kalman;
float avgTemp_Kalman;
float ambientTemp;
float avgTemperature;
float objectTemp;
float x, y, z;
unsigned long MPUTime, MLXTime, thingSpeakTime;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("Connected!!!");
  // initialize the sensor
  while (NO_ERR != mlx.begin()) {
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
  }
  mlx.enterSleepMode();
  if (!mpu.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Begin ok!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  ThingSpeak.begin(client);  // Initialize ThingSpeak library
  MPUTime = MLXTime = thingSpeakTime = millis();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  if ((unsigned long)millis() - MPUTime > 100) {
    Wire.beginTransmission(0x68);
    x = a.acceleration.x;
    y = a.acceleration.y;
    z = a.acceleration.z;
    Wire.endTransmission();
    MPUTime = millis();
  }
  if ((unsigned long)millis() - MLXTime > 1000) {
    mlx.enterSleepMode(false);
    Wire.beginTransmission(0x5A);
    ambientTemp = mlx.getAmbientTempCelsius();
    objectTemp = mlx.getObjectTempCelsius();
    Wire.endTransmission();
    mlx.enterSleepMode();
    MLXTime = millis();
  }

  if ((unsigned long)millis() - thingSpeakTime > 15000) {
    // Data send to thingspeak
    ThingSpeak.setField(1, objectTemp);
    ThingSpeak.setField(2, ambientTemp);
    ThingSpeak.setField(3, x);
    ThingSpeak.setField(4, y);
    ThingSpeak.setField(5, z);
    int httpCode = ThingSpeak.writeFields(channelID, apiKey);


    if (httpCode == 200) {
      Serial.println("Data sent to ThingSpeak successfully.");
    } else {
      Serial.println("Error sending data to ThingSpeak. HTTP error code: " + String(httpCode));
    }
    thingSpeakTime = millis();
  }

  Serial.print(ambientTemp);
  Serial.print(" ");
  Serial.print(objectTemp);
  Serial.print(" ");
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.print(z);
  Serial.println();
  Serial.println(millis());

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }

  client.print(ambientTemp);
  client.print("    ");
  client.print(objectTemp);
  client.print("    ");
  client.print(x);
  client.print("    ");
  client.print(y);
  client.print("    ");
  client.println(z);
}
