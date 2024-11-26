#define BLYNK_TEMPLATE_ID "TMPL6_45WajaT"
#define BLYNK_TEMPLATE_NAME "Project"
#define BLYNK_AUTH_TOKEN "qnQBhPFZ_9isQv_uPwe-Im3U--A2mEOp"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_I2CDevice.h>
#include "FS.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char *ssid = "punchpnp";
const char *password = "0955967996";

WiFiServer server(80); // Create a server that listens on port 80
WiFiClient client;

// Line Notify
const String LINE_TOKEN = "fAKzjptkcxRekINRtvdoeELGw9puKg32fMZDIt0i905"; // Your Line Notify Token

// Temperature and Humidity Sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float humidity = 0.0;
float temperature = 0.0;
unsigned long previousMillis = 0;
const unsigned long interval = 2000;

// Function enable/disable flags
bool ultrasonicEnabled = false;
bool humidtempEnable = false;

void sendLineNotification(const String &message)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("https://notify-api.line.me/api/notify");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Bearer " + LINE_TOKEN);

    String payload = "message=" + message;
    delay(5000);
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 200)
    {
      Serial.print("LINE Notify sent successfully. HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.println("\n");
    }
    else
    {
      Serial.print("Error sending LINE Notify. HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.println("\n");
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected. Cannot send LINE Notify.");
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("loading... ");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  if (Blynk.connected())
    Serial.println("Blynk connected!");
  else
    Serial.println("Blynk connection failed!");

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void humidtemp()
{
  // Periodically measure humidity and temperature
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (humidtempEnable)
    {
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();

      if (isnan(humidity) || isnan(temperature))
      {
        Serial.println("Failed to read from DHT sensor!");
      }
      else
      {
        Serial.print("Measured Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
        Serial.print("Measured Temperature: ");
        Serial.print(temperature);
        Serial.println(" *C");
      }
    }
  }
}

void handleSoilMoistureClient(WiFiClient &client, String data)
{
  if (data == "water")
  {
    String message = "เปิดปั้มน้ำ";
    sendLineNotification(message);
    Serial.println("Notification sent to LINE: เปิดปั้มน้ำ");
    client.println("openPump");
    Serial.println("Data(Water pump on) sent back to the client.");
  }
  else
  {
    Serial.println("Soil Moisture does not meet the condition.");
  }
}

void handleUltrasonicClient(WiFiClient &client, String data)
{
  if (ultrasonicEnabled)
  {
    float distance = data.toFloat();

    if (distance >= 7.50)
    {
      String message = "น้ำใกล้หมดถัง";
      sendLineNotification(message);
      Serial.println("Notification sent to LINE: น้ำหมดถัง");
    }
    else
    {
      Serial.println("Distance does not meet the condition.");
    }

    client.println(data);
    Serial.println("Data sent back to the client.");
  }
}

void handleHumidityTemperatureClient(WiFiClient &client)
{
  if (humidtempEnable)
  {
    // Read humidity and temperature sensor
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    client.print("Humidity: ");
    client.print(humidity);
    client.print(" %\t");
    client.print("Temperature: ");
    client.print(temperature);
    client.println(" *C");
    Serial.println("Sent humidity and temperature data to client.");
  }
}

void loop()
{
  Blynk.run();

  if (humidtempEnable)
    humidtemp();

  client = server.available();
  if (client)
  {
    Serial.println("Client connected");
    while (client.connected())
    {
      if (client.available())
      {
        // test ultrasonic sensor
        String data = client.readStringUntil('\n');
        data.trim();

        // Handle different sensor data
        handleUltrasonicClient(client, data);
        handleHumidityTemperatureClient(client);
        handleSoilMoistureClient(client, data);
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

BLYNK_WRITE(V3) // Button for enable/disable ultrasonic
{
  int pinValue = param.asInt();
  ultrasonicEnabled = (pinValue == 1);
  if (ultrasonicEnabled)
    Serial.println("Ultrasonic function enabled.");
  else
    Serial.println("Ultrasonic function disabled.");
}

BLYNK_WRITE(V5) // Button for enable/disable Humid and Temperate
{
  int pinValue = param.asInt();
  humidtempEnable = (pinValue == 1);
  if (humidtempEnable)
    Serial.println("Humidity and Temperate function enabled.");
  else
    Serial.println("Humidity and Temperate function disabled.");
}
