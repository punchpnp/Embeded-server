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
const String LINE_TOKEN = "wHpInKQTtQ9OiQW9CgDwEIYXkzOSS1fH5QnUMCpOPYh"; // Your Line Notify Token

void sendLineNotification(const String &message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://notify-api.line.me/api/notify");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Bearer " + LINE_TOKEN);

    String payload = "message=" + message;
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 200) {
      Serial.print("LINE Notify sent successfully. HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.println("\n");
    } else {
      Serial.print("Error sending LINE Notify. HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.println("\n");
    }
    http.end();
  } else {
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
        Serial.print(" cm");

        // Convert the received data to a float
        float distance = data.toFloat();

        // Check the condition
        if (distance >= 9.40) {
          String message = "น้ำหมดถัง"; // Custom message for specific condition
          sendLineNotification(message);
          Serial.println("Notification sent to LINE: น้ำหมดถัง");
        } else {
          Serial.println("Distance does not meet the condition.");
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}