#define BLYNK_TEMPLATE_ID " "
#define BLYNK_DEVICE_NAME "Compost Monitoring System"
#define BLYNK_AUTH_TOKEN " "

#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <NewPing.h>
#define BLYNK_PRINT Serial  // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = BLYNK_AUTH_TOKEN; // Auth token
char ssid[] = "Kokojimz";       // WIFI name
char pass[] = " ";  // WIFI password

BlynkTimer timer;

//Define component pins
#define MOISTURE_SENSOR_PIN A0
#define DHTPIN D3             // Pin untuk DHT11
#define DHTTYPE DHT11         // Tipe sensor DHT
#define TRIGGER_PIN D5  // ESP8266 Pin D5 sebagai pin Trigger
#define ECHO_PIN    D6  // ESP8266 Pin D6 sebagai pin Echo
#define MAX_DISTANCE 400 // Maksimum jarak yang ingin kita ukur (dalam cm)

DHT dht(DHTPIN, DHTTYPE);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait until connected – only necessary on some boards
  
  lcd.init();
  lcd.backlight();
  dht.begin();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.setCursor(1, 0);
  lcd.print("System Loading");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, sendUltrasonicDistance);
}

void sendSensorData() {
  // Reading moisture sensor
  int moistureValue = analogRead(MOISTURE_SENSOR_PIN);
  moistureValue = map(moistureValue, 0, 1024, 0, 100);
  moistureValue = 100 - moistureValue;
  Blynk.virtualWrite(V0, moistureValue);

  // Reading temperature and humidity
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Checking if any reading failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Sending to Blynk
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);

  // Printing to LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.setCursor(8, 1);
  lcd.print("Moist: ");
  lcd.print(moistureValue);
}

void sendUltrasonicDistance() {
  // Mengukur jarak
  unsigned int uS = sonar.ping();
  float distance = uS / US_ROUNDTRIP_CM; // Konversi waktu ke jarak

  // Kirim jarak ke pin virtual V1 di Blynk
  Blynk.virtualWrite(V1, distance);
  
  // Opsi: tampilkan jarak di serial monitor untuk debugging
  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println("cm");
}

void loop() {
  Blynk.run();
  timer.run();
}
