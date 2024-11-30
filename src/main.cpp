// BLYNK
#define BLYNK_TEMPLATE_ID "TMPL6_45WajaT"
#define BLYNK_TEMPLATE_NAME "Project"
#define BLYNK_AUTH_TOKEN "qnQBhPFZ_9isQv_uPwe-Im3U--A2mEOp"

// FIREBASE
#define FIREBASE_API_KEY "AIzaSyDlku4rxvpDzrvbXsaa_PK__VbLUtF4GKY"
#define FIREBASE_DATABASE_URL "https://embreddedproject-default-rtdb.asia-southeast1.firebasedatabase.app/"

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
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <LiquidCrystal_I2C.h>

// LCD display setup
LiquidCrystal_I2C lcd(0x27, 8, 1);
String feeling = "happy";

// Wi-Fi credentials
const char *ssid = "punchpnp";
const char *password = "0955967996";

bool FB_signupOK = false;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

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

// Light Sensor
#define LIGHT_SENSOR_PIN 34 // ESP32 pin GIOP36 (ADC0)
int lightSensorValue = 0;

// Function enable/disable flags
bool ultrasonicEnabled = true;
bool humidtempEnabled = true;
bool lightSensorEnabled = true;

void displayFeeling(String mood)
{
  lcd.clear();

  if (mood == "happy")
  {
    lcd.setCursor(3, 0);
    lcd.print("( O w O )");
    delay(1000);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("( - w - )");
  }
  else if (mood == "good")
  {
    lcd.setCursor(3, 0);
    lcd.print("( - _ - )");
    delay(1000);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("( - O - )");
  }
  else if (mood == "sad")
  {
    lcd.setCursor(3, 0);
    lcd.print("( T _ T )");
    delay(1000);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("( T - T )");
  }
}

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
  analogSetAttenuation(ADC_11db);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  displayFeeling(feeling);

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

  // Initialize Firebase
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase sign up OK!");
    FB_signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
    Serial.println("Firebase sign up failed!");
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void handleFirebaseStoreData(String _path, String _data)
{
  if (Firebase.ready() && FB_signupOK)
  {
    if (Firebase.RTDB.setString(&fbdo, _path, _data))
    {
      Serial.println();
      Serial.print(_data);
      Serial.print(" - Successfully saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ")");
    }
    else
    {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }
}

void humidtemp()
{
  // Periodically measure humidity and temperature
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (humidtempEnabled)
    {
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();

      if (isnan(humidity) || isnan(temperature))
      {
        Serial.println("Failed to read from DHT sensor!");
      }
      else
      {
        handleFirebaseStoreData("Server/Huminity", String(humidity));
        handleFirebaseStoreData("Server/Temperature", String(temperature));

        Blynk.virtualWrite(V1, humidity);
        Blynk.virtualWrite(V2, temperature);

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
  if (data.indexOf("water") != 1)
  {
    Serial.println("Soil Moisture does not meet the condition.");
  }
  else
  {
    client.println("openPump");
    Serial.println("Data(Water pump on) sent back to the client.");
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
  if (humidtempEnabled)
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

void handleLightSensorClient(WiFiClient &client)
{
  if (lightSensorEnabled)
  {
    int lightSensorValue = analogRead(LIGHT_SENSOR_PIN); // Read light sensor value

    Serial.print("Analog Value = ");
    Serial.print(lightSensorValue);
    Serial.print("\t");

    handleFirebaseStoreData("Server/LightSensor", String(lightSensorValue));

    String lightStatus = "Unknown";
    if (lightSensorValue < 40)
    {
      lightStatus = "Dark";
    }
    else if (lightSensorValue < 800)
    {
      lightStatus = "Dim";
    }
    else if (lightSensorValue < 2000)
    {
      lightStatus = "Light";
    }
    else if (lightSensorValue < 3200)
    {
      lightStatus = "Bright";
    }
    else
    {
      lightStatus = "Very bright";
    }

    client.print("Light Status: ");
    client.println(lightStatus);
    Serial.print("Light Sensor: ");
    Serial.println(lightStatus);
  }
}

void feelingLoop()
{
  if (feeling == "happy")
  {
    displayFeeling("happy");
    delay(1000);
  }
  else if (feeling == "good")
  {
    displayFeeling("good");
    delay(1000);
  }
  else if (feeling == "sad")
  {
    displayFeeling("sad");
    delay(1000);
  }
}

void loop()
{
  Blynk.run();

  feelingLoop();

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
        if (humidtempEnabled)
          humidtemp();
        Serial.println("-----------------------------");
        handleHumidityTemperatureClient(client);
        Serial.println("-----------------------------");
        handleSoilMoistureClient(client, data);
        Serial.println("-----------------------------");
        handleLightSensorClient(client);
        Serial.println("-----------------------------");
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
  humidtempEnabled = (pinValue == 1);
  if (humidtempEnabled)
    Serial.println("Humidity and Temperate function enabled.");
  else
    Serial.println("Humidity and Temperate function disabled.");
}
