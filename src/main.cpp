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
const char *ssid = "Jorpor.foto";
const char *password = "88888888";

WiFiServer server(80); // Create a server that listens on port 80
WiFiClient client;

// Line Notify
const String LINE_TOKEN = "fAKzjptkcxRekINRtvdoeELGw9puKg32fMZDIt0i905"; // Your Line Notify Token

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

  // Start the server'
  server.begin();
  Serial.println("Server started");
}

bool ultrasonicEnabled = false; // Ultrasonic function

void loop()
{
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
        // test button
        char command = client.read();

        Serial.print("Received : ");
        Serial.println(data);
        // Serial.print(" cm");

        if (data == "water")
        {
          String message = "เปิดปั้มน้ำ";
          sendLineNotification(message);
          Serial.println("Notification sent to LINE: เปิดปั้มน้ำ");
          client.println(data);
          Serial.println("Data(Water pump on) sent back to the client.");
        }
        else
        {
          Serial.println("Soil Moisture does not meet the condition.");
        }

        // Convert the received data to a float
        float distance = data.toFloat();

        // Check the condition
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

        // Send the received data back to the client
        if (ultrasonicEnabled)
        {
          client.println(data);
          Serial.println("Data sent back to the client.");
        }
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