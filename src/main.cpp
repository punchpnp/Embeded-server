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

// Wi-Fi credentials
const char *ssid = "punchpnp";
const char *password = "0955967996";

#define DHTPIN 4      // GPIO D4
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

WiFiServer server(80); // Create a server that listens on port 80
WiFiClient client;

// BlynkTimer timer;

// void myTimer()
// {
//   client.write(humidity);
//   delay(500);
//   client.write(temperature);
//   delay(500);
// }

void setup()
{
  Serial.begin(115200);

  // Initialize DHT sensor
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

  // Start the server'
  server.begin();
  Serial.println("Server started");
}

void loop()
{
  // Check if a client has connected
  client = server.available();
  if (client)
  {
    Serial.println("Client connected");
    while (client.connected())
    {
      if (client.available())
      {
        Serial.println("Client available");
        char command = client.read();
        Serial.print("Received command: ");

        Serial.println(command);

        // Handle the command
        if (command == '1')
        {
          // Do something when command is 1
          Serial.println("Command 1 received");
        }
        else if (command == '0')
        {
          // Do something when command is 0
          Serial.println("Command 0 received");
        }
        else if (command == 'W')
        {
          Serial.println("Command W sended");
        }
        // Read DHT sensor data
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        // Check if any reads failed and exit early (to try again).
        if (isnan(humidity) || isnan(temperature))
        {
          Serial.println("Failed to read from DHT sensor!");
          client.println("Failed to read from DHT sensor!");
          return;
        }
        client.print("Humidity: ");
        client.print(humidity);
        client.print(" %\t");
        client.print("Temperature: ");
        client.print(temperature);
        client.println(" *C");

        // Print the results to the serial monitor
        // Serial.print("Humidity: ");
        // Serial.print(humidity);
        // Serial.print(" %\t");
        // Serial.print("Temperature: ");
        // Serial.print(temperature);
        // Serial.println(" *C");
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}