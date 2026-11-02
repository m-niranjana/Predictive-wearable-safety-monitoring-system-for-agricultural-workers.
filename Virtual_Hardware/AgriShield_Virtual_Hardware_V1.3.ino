#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// ================= PIN CONFIGURATION =================

#define DHT_PIN 15
#define DHT_TYPE DHT22

#define GAS_PIN 34
#define BUZZER_PIN 25

#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ================= SENSOR OBJECTS =================

DHT dht(DHT_PIN, DHT_TYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

Adafruit_MPU6050 mpu;

// ================= VARIABLES =================

float temperature = 0.0;
float humidity = 0.0;

int gasValue = 0;

float acceleration = 0.0;
float activityLevel = 0.0;

String environmentalRisk = "NORMAL";
String systemAlert = "NORMAL";

bool oledReady = false;
bool mpuReady = false;
bool dhtReadingValid = false;

unsigned long previousTime = 0;

const unsigned long samplingInterval = 2000;

// ================= SETUP =================

void setup() {

  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GAS_PIN, INPUT);

  digitalWrite(BUZZER_PIN, LOW);

  analogReadResolution(12);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" AGRISHIELD VIRTUAL HARDWARE");
  Serial.println(" VERSION 1.3");
  Serial.println("==============================");

  // DHT22
  dht.begin();

  // I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // OLED
  oledReady = display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  if (oledReady) {

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("AGRISHIELD");
    display.println("Initializing...");
    display.display();

    Serial.println("OLED: READY");

  } else {

    Serial.println("OLED: FAILED");

  }

  // MPU6050
  mpuReady = mpu.begin();

  if (mpuReady) {

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("MPU6050: READY");

  } else {

    Serial.println("MPU6050: FAILED");

  }

  Serial.println("DHT22: INITIALIZED");
  Serial.println("MQ-2: INITIALIZED");

  Serial.println("==============================");
  Serial.println("INITIALIZATION COMPLETE");
  Serial.println("==============================");

  delay(2000);
}

// ================= READ SENSORS =================

void readSensors() {

  // DHT22
  float newTemperature = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  if (!isnan(newTemperature) && !isnan(newHumidity)) {

    temperature = newTemperature;
    humidity = newHumidity;

    dhtReadingValid = true;

  } else {

    dhtReadingValid = false;

    Serial.println("WARNING: DHT22 READING FAILED");

  }

  // MQ-2
  gasValue = analogRead(GAS_PIN);

  // MPU6050
  if (mpuReady) {

    sensors_event_t a, g, temp;

    mpu.getEvent(&a, &g, &temp);

    acceleration = sqrt(
      a.acceleration.x * a.acceleration.x +
      a.acceleration.y * a.acceleration.y +
      a.acceleration.z * a.acceleration.z
    );

    activityLevel = fabs(acceleration - 9.81);

  }

}

// ================= RISK ASSESSMENT =================

void assessRisk() {

  environmentalRisk = "NORMAL";

  if (dhtReadingValid &&
      temperature >= 35.0 &&
      humidity >= 70.0) {

    environmentalRisk = "HIGH";

  }

  else if (
    (dhtReadingValid &&
     (temperature >= 32.0 || humidity >= 70.0)) ||
    gasValue >= 1800
  ) {

    environmentalRisk = "ELEVATED";

  }

  systemAlert = environmentalRisk;

  // BUZZER
  if (systemAlert == "HIGH") {

    tone(BUZZER_PIN, 2000);

  }

  else if (systemAlert == "ELEVATED") {

    tone(BUZZER_PIN, 1000);

  }

  else {

    noTone(BUZZER_PIN);

  }

}

// ================= OLED DISPLAY =================

void updateDisplay() {

  if (!oledReady) {
    return;
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("AGRISHIELD V1.3");
  display.println("----------------");

  display.print("Temp: ");

  if (dhtReadingValid) {
    display.print(temperature, 1);
    display.println(" C");
  } else {
    display.println("ERROR");
  }

  display.print("Humidity: ");

  if (dhtReadingValid) {
    display.print(humidity, 1);
    display.println(" %");
  } else {
    display.println("ERROR");
  }

  display.print("Gas: ");
  display.println(gasValue);

  display.print("Activity: ");
  display.println(activityLevel, 2);

  display.print("Risk: ");
  display.println(environmentalRisk);

  display.display();

}

// ================= SERIAL DASHBOARD =================

void printDashboard() {

  Serial.println();
  Serial.println("========== AGRISHIELD ==========");

  Serial.print("Temperature: ");

  if (dhtReadingValid) {
    Serial.print(temperature, 1);
    Serial.println(" C");
  } else {
    Serial.println("INVALID");
  }

  Serial.print("Humidity: ");

  if (dhtReadingValid) {
    Serial.print(humidity, 1);
    Serial.println(" %");
  } else {
    Serial.println("INVALID");
  }

  Serial.print("MQ-2 Gas Response: ");
  Serial.println(gasValue);

  Serial.print("Acceleration: ");
  Serial.print(acceleration, 2);
  Serial.println(" m/s2");

  Serial.print("Activity: ");
  Serial.println(activityLevel, 2);

  Serial.print("Environmental Risk: ");
  Serial.println(environmentalRisk);

  Serial.print("System Alert: ");
  Serial.println(systemAlert);

  Serial.println("================================");

}

// ================= MAIN LOOP =================

void loop() {

  unsigned long currentTime = millis();

  if (currentTime - previousTime >= samplingInterval) {

    previousTime = currentTime;

    readSensors();

    assessRisk();

    updateDisplay();

    printDashboard();

  }

}

