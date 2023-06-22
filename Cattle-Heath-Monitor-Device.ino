#include <Adafruit_MLX90614.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
//#include <SimpleKalmanFilter.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>  // Added ThingSpeak library

WiFiClient client;
Adafruit_MPU6050 mpu;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float ambientTemp_Kalman;
float avgTemp_Kalman;
float ambientTemp;
float avgTemperature;

const char *apiKey = "TQYYF39XU1YINRKE";  // Enter your Write API key from ThingSpeak
unsigned long channelID = 2196082;        // Replace with your ThingSpeak channel ID

const char *ssid = "ASUS";  // replace with your WiFi SSID and WPA2 key
const char *pass = "123123123";
const char *server = "api.thingspeak.com";

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to " + String(ssid));
  unsigned long time = millis();
  int i = 0;

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  if (!mlx.begin(0x5A)) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1){
      delay(10);
    }
  }

  if (!mpu.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  Wire.begin();
  delay(100);

  ThingSpeak.begin(client);  // Initialize ThingSpeak library
}

void loop() {
  Wire.beginTransmission(0x5A);
  float avgTemp = 0;
  for (i;(unsigned long)(millis() - time)>10000; i++) {
    float temp = mlx.readObjectTempC();
    avgTemp += temp;
    time = millis();
  }
  avgTemp = avgTemp/i;

  // // Serial.print("Ambient temperature = ");
  // // ambientTemp = mlx.readAmbientTempC();
  // // // ambientTemp_Kalman = filter.updateEstimate(ambientTemp);
  // // Serial.print(ambientTemp);
  // // Serial.print("°C   ");
  // // Serial.print("Object temperature = ");
  // // avgTemperature = avgTemp / 100;
  float objectTemp = mlx.readObjectTempC();
  // Serial.print(objectTemp);
  // Serial.print(" ");
  // Wire.endTransmission(0x5A);
  // delay(500);


  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  // Wire.beginTransmission(0x68);
  // //Serial.print("Acceleration X: ");
  // Serial.print(a.acceleration.x);
  // delay(1);
  // Serial.print(" ");
  // Serial.print(a.acceleration.y);
  // delay(1);
  // Serial.print(" ");
  // Serial.println(a.acceleration.z);
  // delay(1);
  // //Serial.println(" m/s^2");
  // Wire.endTransmission(0x68);

  // Serial.print("Rotation X: ");
  // Serial.print(g.gyro.x);
  // Serial.print(", Y: ");
  // Serial.print(g.gyro.y);
  // Serial.print(", Z: ");
  // Serial.print(g.gyro.z);
  // Serial.println(" rad/s");

  // Serial.print("Temperature: ");
  // Serial.print(temp.temperature);
  // Serial.println(" degC");
  // delay(250);

  // lcd.setCursor(11, 0);
  // lcd.print(avgTemperature);
  // lcd.setCursor(9, 1);
  // lcd.print(ambientTemp_Kalman);

  // Send data to ThingSpeak
  ThingSpeak.setField(1, objectTemp);
  ThingSpeak.setField(2, ambientTemp);
  ThingSpeak.setField(3, g.gyro.x);
  ThingSpeak.setField(4, g.gyro.y);
  ThingSpeak.setField(5, g.gyro.z);

  int httpCode = ThingSpeak.writeFields(channelID, apiKey);
  // if (httpCode == 200) {
  //   Serial.println("Data sent to ThingSpeak successfully.");
  // } else {
  //   Serial.println("Error sending data to ThingSpeak. HTTP error code: " + String(httpCode));
  // }
  if (!client.connect("192.168.2.77", 50000)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }
  //client.println("hello from ESP8266");

  // read back one line from server
  Wire.beginTransmission(0x5A);
  client.print(avgTemp);
  client.print(" ");
  Wire.endTransmission(0x5A);
  delay(20);
  Wire.beginTransmission(0x68);
  //Serial.print("Acceleration X: ");
  client.print(a.acceleration.x);
  client.print(" ");
  client.print(a.acceleration.y);
  client.print(" ");
  client.println(a.acceleration.z);
  //Serial.println(" m/s^2");
  Wire.endTransmission(0x68);

  // Serial.println("closing connection");
  // client.stop();

  //delay(500); // Send data every 10 seconds
}
