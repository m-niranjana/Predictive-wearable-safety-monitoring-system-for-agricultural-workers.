/*
========================================
             AGRISHIELD
    Intelligent Agricultural Worker
       Safety Monitoring System

    Version: 5.0
    Platform: ESP32
    Mode: Software Sensor Simulation
========================================
*/

// GLOBAL SENSOR VARIABLES
float temperature;
float humidity;
int airQuality;
int heartRate;
int exposureMinutes = 0;
const float MIN_TEMP = 0.0;
const float MAX_TEMP = 60.0;

const float MIN_HUMIDITY = 0.0;
const float MAX_HUMIDITY = 100.0;

const float MIN_AIR_QUALITY = 0.0;
const float MAX_AIR_QUALITY = 1000.0;

const float MIN_HEART_RATE = 30.0;
const float MAX_HEART_RATE = 220.0;
String activity;
String heatRisk;
String airQualityStatus;
String physiologicalStatus;
String systemAlert;
// FUTURE RISK ASSESSMENT
String futureHeatRisk = "NORMAL";
String futureAirRisk = "NORMAL";
String futurePhysiologicalRisk = "NORMAL";
String futureSystemAlert = "NORMAL";
int readingCount = 0;
// AWARE MODULE 04: ADAPTIVE SAMPLING

unsigned long samplingInterval = 5000;

const unsigned long NORMAL_INTERVAL = 5000;
const unsigned long ELEVATED_INTERVAL = 3000;
const unsigned long HIGH_INTERVAL = 1000;
unsigned long previousSamplingTime = 0;
// 15-MINUTE FUTURE RISK ASSESSMENT
String futureHeatRisk15 = "NORMAL";
String futureAirRisk15 = "NORMAL";
String futurePhysiologicalRisk15 = "NORMAL";
String futureSystemAlert15 = "NORMAL";
// PERSONAL BASELINE ANALYSIS
float baselineHeartRate = 0;
float baselineSum = 0;
float heartRateDeviation = 0;
int baselineReadings = 0;

const int CALIBRATION_SAMPLES = 5;
bool baselineReady = false;

// CUMULATIVE EXPOSURE ANALYSIS
float heatExposureBurden = 0.0;
float airExposureBurden = 0.0;
float heatIntensity = 0.0;
float airIntensity = 0.0;
float simulatedTimeStep = 1.0;

// AWARE MODULE 03: PREDICTIVE RISK ANALYSIS
const int TREND_WINDOW = 5;
// TREND DAMPING FACTORS
const float DAMPING_5_MIN = 0.8;
const float DAMPING_15_MIN = 0.4;
float temperatureHistory[TREND_WINDOW];
float humidityHistory[TREND_WINDOW];
float airQualityHistory[TREND_WINDOW];
float heartRateHistory[TREND_WINDOW];

int historyIndex = 0;
int historyCount = 0;
// PREDICTION CONFIDENCE
String predictionConfidence = "LOW";
// ==========================================
// ADAPTIVE EMA SENSOR SMOOTHING
// ==========================================

float smoothTemperature = 0;
float smoothHumidity = 0;
float smoothAirQuality = 0;
float smoothHeartRate = 0;

float alphaTemperature = 0.2;
float alphaHumidity = 0.2;
float alphaAirQuality = 0.2;
float alphaHeartRate = 0.2;

bool smoothingInitialized = false;
// PREDICTED SENSOR VALUES
float predictedTemp5 = 0;
float predictedTemp15 = 0;

float predictedHumidity5 = 0;
float predictedHumidity15 = 0;

float predictedAir5 = 0;
float predictedAir15 = 0;

float predictedHeart5 = 0;
float predictedHeart15 = 0;
// ==========================================
// AWARE MODULE 03: SENSOR HISTORY
// ==========================================
void storeSensorHistory() {
  temperatureHistory[historyIndex] = temperature;
  humidityHistory[historyIndex] = humidity;
  airQualityHistory[historyIndex] = airQuality;
  heartRateHistory[historyIndex] = heartRate;

  historyIndex++;

  if (historyIndex >= TREND_WINDOW) {
    historyIndex = 0;
  }

  if (historyCount < TREND_WINDOW) {
    historyCount++;
  }
}
// ==========================================
// ADAPTIVE EMA SMOOTHING FUNCTION
// ==========================================

float adaptiveEMA(float currentValue,
                  float previousValue,
                  float &alpha) {

  float change = abs(currentValue - previousValue);

  if (change > 10) {
    alpha = 0.7;
  }
  else if (change > 5) {
    alpha = 0.5;
  }
  else {
    alpha = 0.2;
  }

  return (alpha * currentValue) +
         ((1.0 - alpha) * previousValue);
}
// ==========================================
// SENSOR SMOOTHING MANAGER
// ==========================================

void updateSmoothedSensors() {

  if (!smoothingInitialized) {

    smoothTemperature = temperature;
    smoothHumidity = humidity;
    smoothAirQuality = airQuality;
    smoothHeartRate = heartRate;

    smoothingInitialized = true;
    return;
  }

  smoothTemperature = adaptiveEMA(
    temperature, smoothTemperature, alphaTemperature);

  smoothHumidity = adaptiveEMA(
    humidity, smoothHumidity, alphaHumidity);

  smoothAirQuality = adaptiveEMA(
    airQuality, smoothAirQuality, alphaAirQuality);

  smoothHeartRate = adaptiveEMA(
    heartRate, smoothHeartRate, alphaHeartRate);
}
// ==========================================
// AWARE MODULE 03: TREND CALCULATION
// Returns average change per simulated reading.
// ==========================================
// ==========================================
// AWARE MODULE 03: LINEAR REGRESSION
// ==========================================

float calculateTrend(float data[]) {

  if (historyCount < 2) {
    return 0.0;
  }

  int oldestIndex;

  if (historyCount < TREND_WINDOW) {
    oldestIndex = 0;
  }
  else {
    oldestIndex = historyIndex;
  }

  float sumX = 0;
  float sumY = 0;
  float sumXY = 0;
  float sumX2 = 0;

  for (int i = 0; i < historyCount; i++) {

    int index = (oldestIndex + i) % TREND_WINDOW;

    float x = i;
    float y = data[index];

    sumX += x;
    sumY += y;
    sumXY += x * y;
    sumX2 += x * x;
  }

  float n = historyCount;

  float denominator = (n * sumX2) - (sumX * sumX);

  if (denominator == 0) {
    return 0.0;
  }

  float slope = ((n * sumXY) - (sumX * sumY)) / denominator;

  return slope;
}
// PREDICTION CONFIDENCE CALCULATION

void updatePredictionConfidence() {

  if (historyCount <= 2) {
    predictionConfidence = "LOW";
  }

  else if (historyCount < TREND_WINDOW) {
    predictionConfidence = "MODERATE";
  }

  else {
    predictionConfidence = "HIGH";
  }
}
// ==========================================
// AWARE MODULE 03: FUTURE PREDICTION ENGINE
// ==========================================

void predictFutureValues() {

  // Wait until five readings are available

  if (historyCount < TREND_WINDOW) {

    Serial.println("Collecting sensor history...");

    return;
  }


  // Calculate sensor trends

  float tempTrend =
      calculateTrend(temperatureHistory);

  float humidityTrend =
      calculateTrend(humidityHistory);

  float airTrend =
      calculateTrend(airQualityHistory);

  float heartTrend =
      calculateTrend(heartRateHistory);


// 5-MINUTE DAMPED PREDICTION

predictedTemp5 = smoothTemperature + (tempTrend * 5 * DAMPING_5_MIN);
predictedHumidity5 = smoothHumidity + (humidityTrend * 5 * DAMPING_5_MIN);
predictedAir5 = smoothAirQuality + (airTrend * 5 * DAMPING_5_MIN);
predictedHeart5 = smoothHeartRate + (heartTrend * 5 * DAMPING_5_MIN);

// 15-MINUTE DAMPED PREDICTION

predictedTemp15 = smoothTemperature + (tempTrend * 15 * DAMPING_15_MIN);
predictedHumidity15 = smoothHumidity + (humidityTrend * 15 * DAMPING_15_MIN);
predictedAir15 = smoothAirQuality + (airTrend * 15 * DAMPING_15_MIN);
predictedHeart15 = smoothHeartRate + (heartTrend * 15 * DAMPING_15_MIN);


// BOUNDED PREDICTION

// Temperature bounds
predictedTemp5 = constrain(predictedTemp5, MIN_TEMP, MAX_TEMP);
predictedTemp15 = constrain(predictedTemp15, MIN_TEMP, MAX_TEMP);

// Humidity bounds
predictedHumidity5 = constrain(predictedHumidity5, MIN_HUMIDITY, MAX_HUMIDITY);
predictedHumidity15 = constrain(predictedHumidity15, MIN_HUMIDITY, MAX_HUMIDITY);

// Air quality bounds
predictedAir5 = constrain(predictedAir5, MIN_AIR_QUALITY, MAX_AIR_QUALITY);
predictedAir15 = constrain(predictedAir15, MIN_AIR_QUALITY, MAX_AIR_QUALITY);

// Heart rate bounds
predictedHeart5 = constrain(predictedHeart5, MIN_HEART_RATE, MAX_HEART_RATE);
predictedHeart15 = constrain(predictedHeart15, MIN_HEART_RATE, MAX_HEART_RATE);
}
// ==========================================
// AWARE MODULE 03: FUTURE RISK ASSESSMENT
// ==========================================

void assessFutureRisk() {

  // FUTURE HEAT RISK
  if (predictedTemp5 >= 35 && predictedHumidity5 >= 70) {
    futureHeatRisk = "HIGH";
  }
  else if (predictedTemp5 >= 32 || predictedHumidity5 >= 70) {
    futureHeatRisk = "ELEVATED";
  }
  else {
    futureHeatRisk = "NORMAL";
  }

  // FUTURE AIR QUALITY RISK
  if (predictedAir5 >= 300) {
    futureAirRisk = "HIGH";
  }
  else if (predictedAir5 >= 150) {
    futureAirRisk = "ELEVATED";
  }
  else {
    futureAirRisk = "NORMAL";
  }

  // FUTURE PHYSIOLOGICAL RISK
  if (baselineReady) {

    float futureHeartDeviation =
      ((predictedHeart5 - baselineHeartRate) / baselineHeartRate) * 100.0;

    if (futureHeartDeviation > 20) {
      futurePhysiologicalRisk = "HIGH";
    }
    else if (futureHeartDeviation > 10) {
      futurePhysiologicalRisk = "ELEVATED";
    }
    else {
      futurePhysiologicalRisk = "NORMAL";
    }
  }
  else {
    futurePhysiologicalRisk = "CALIBRATING";
  }

  // FUTURE SYSTEM ALERT
  if (futureHeatRisk == "HIGH" ||
      futureAirRisk == "HIGH" ||
      futurePhysiologicalRisk == "HIGH") {

    futureSystemAlert = "FUTURE URGENT WARNING";
  }
  else if (futureHeatRisk == "ELEVATED" ||
           futureAirRisk == "ELEVATED" ||
           futurePhysiologicalRisk == "ELEVATED") {

    futureSystemAlert = "FUTURE CAUTION";
  }
  else {
    futureSystemAlert = "NORMAL";
  }
}
// ==========================================
// 15-MINUTE FUTURE RISK ASSESSMENT
// ==========================================

void assessFutureRisk15() {

  // 15-MINUTE HEAT RISK
  if (predictedTemp15 >= 35 && predictedHumidity15 >= 70) {
    futureHeatRisk15 = "HIGH";
  }
  else if (predictedTemp15 >= 32 || predictedHumidity15 >= 70) {
    futureHeatRisk15 = "ELEVATED";
  }
  else {
    futureHeatRisk15 = "NORMAL";
  }

  // 15-MINUTE AIR QUALITY RISK
  if (predictedAir15 >= 300) {
    futureAirRisk15 = "HIGH";
  }
  else if (predictedAir15 >= 150) {
    futureAirRisk15 = "ELEVATED";
  }
  else {
    futureAirRisk15 = "NORMAL";
  }

  // 15-MINUTE PHYSIOLOGICAL RISK
  if (baselineReady) {

    float futureHeartDeviation15 =
      ((predictedHeart15 - baselineHeartRate) / baselineHeartRate) * 100.0;

    if (futureHeartDeviation15 > 20) {
      futurePhysiologicalRisk15 = "HIGH";
    }
    else if (futureHeartDeviation15 > 10) {
      futurePhysiologicalRisk15 = "ELEVATED";
    }
    else {
      futurePhysiologicalRisk15 = "NORMAL";
    }
  }
  else {
    futurePhysiologicalRisk15 = "CALIBRATING";
  }

  // 15-MINUTE SYSTEM ALERT
  if (futureHeatRisk15 == "HIGH" ||
      futureAirRisk15 == "HIGH" ||
      futurePhysiologicalRisk15 == "HIGH") {

    futureSystemAlert15 = "FUTURE URGENT WARNING";
  }
  else if (futureHeatRisk15 == "ELEVATED" ||
           futureAirRisk15 == "ELEVATED" ||
           futurePhysiologicalRisk15 == "ELEVATED") {

    futureSystemAlert15 = "FUTURE CAUTION";
  }
  else {
    futureSystemAlert15 = "NORMAL";
  }
}
// ==========================================
// PERSONAL BASELINE UPDATE
// ==========================================
void updatePersonalBaseline() {
  if (!baselineReady) {
    baselineSum += heartRate;
    baselineReadings++;

    if (baselineReadings >= CALIBRATION_SAMPLES) {
      baselineHeartRate = baselineSum / CALIBRATION_SAMPLES;
      baselineReady = true;

      Serial.println();
      Serial.println("BASELINE CALIBRATION COMPLETE!");
      Serial.print("Personal Heart Rate Baseline: ");
      Serial.print(baselineHeartRate);
      Serial.println(" BPM");
    }
    return;
  }

  if (heatRisk == "NORMAL" &&
      airQualityStatus == "NORMAL" &&
      activity == "LOW" &&
      abs(heartRate - baselineHeartRate) <= baselineHeartRate * 0.10) {
    baselineHeartRate = (0.98 * baselineHeartRate) + (0.02 * heartRate);
  }
}

// ==========================================
// AWARE MODULE 02: CUMULATIVE EXPOSURE
// ==========================================
void calculateExposure() {
  heatIntensity = 0.0;

  if (temperature > 30.0) {
    heatIntensity += (temperature - 30.0) / 10.0;
  }

  if (humidity > 60.0) {
    heatIntensity += (humidity - 60.0) / 40.0;
  }

  airIntensity = 0.0;

  if (airQuality > 100) {
    airIntensity = (airQuality - 100) / 200.0;
  }

  heatExposureBurden += heatIntensity * simulatedTimeStep;
  airExposureBurden += airIntensity * simulatedTimeStep;
}
// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("====================================");
  Serial.println("          AGRISHIELD SYSTEM");
  Serial.println("====================================");
  Serial.println("Initializing system...");

  delay(1500);

  Serial.println("System Ready!");
}

// ==========================================
// SENSOR SIMULATION
// ==========================================
void simulateSensors() {
  int scenario = (readingCount / 5) % 3;

  if (scenario == 0) {
    temperature = 29.5;
    humidity = 60.0;
    airQuality = 90;
    heartRate = 78;
    activity = "LOW";
  }
  else if (scenario == 1) {
    temperature = 34.5;
    humidity = 72.0;
    airQuality = 210;
    heartRate = 105;
    activity = "MODERATE";
  }
  else {
    temperature = 37.0;
    humidity = 82.0;
    airQuality = 320;
    heartRate = 125;
    activity = "HIGH";
  }

  readingCount++;
  exposureMinutes++;
}

// ==========================================
// HEAT RISK ASSESSMENT
// ==========================================
void assessHeatRisk() {
  if (temperature >= 35 && humidity >= 70) {
    heatRisk = "HIGH";
  }
  else if (temperature >= 32 || humidity >= 70) {
    heatRisk = "ELEVATED";
  }
  else {
    heatRisk = "NORMAL";
  }
}

// ==========================================
// AIR QUALITY ASSESSMENT
// ==========================================
void assessAirQuality() {
  if (airQuality >= 300) {
    airQualityStatus = "HIGH";
  }
  else if (airQuality >= 150) {
    airQualityStatus = "ELEVATED";
  }
  else {
    airQualityStatus = "NORMAL";
  }
}

// ==========================================
// PHYSIOLOGICAL ASSESSMENT
// ==========================================
void assessPhysiologicalStatus() {
  if (!baselineReady) {
    physiologicalStatus = "CALIBRATING";
    return;
  }

  heartRateDeviation =
      ((heartRate - baselineHeartRate) / baselineHeartRate) * 100.0;

  if (heartRateDeviation > 20) {
    physiologicalStatus = "HIGH";
  }
  else if (heartRateDeviation > 10) {
    physiologicalStatus = "ELEVATED";
  }
  else {
    physiologicalStatus = "NORMAL";
  }
}

// ==========================================
// ALERT GENERATION
// ==========================================
void generateAlert() {
  if (heatRisk == "HIGH" ||
      airQualityStatus == "HIGH" ||
      physiologicalStatus == "HIGH") {
    systemAlert = "URGENT WARNING";
  }
  else if (heatRisk == "ELEVATED" ||
           airQualityStatus == "ELEVATED" ||
           physiologicalStatus == "ELEVATED") {
    systemAlert = "CAUTION";
  }
  else {
    systemAlert = "NORMAL";
  }
}
// ==========================================
// AWARE MODULE 04: ADAPTIVE SAMPLING CONTROL
// ==========================================

void updateSamplingInterval() {

  // HIGH RISK
  if (heatRisk == "HIGH" ||
      airQualityStatus == "HIGH" ||
      physiologicalStatus == "HIGH") {

    samplingInterval = HIGH_INTERVAL;
  }

  // ELEVATED RISK
  else if (heatRisk == "ELEVATED" ||
           airQualityStatus == "ELEVATED" ||
           physiologicalStatus == "ELEVATED") {

    samplingInterval = ELEVATED_INTERVAL;
  }

  // NORMAL CONDITIONS
  else {

    samplingInterval = NORMAL_INTERVAL;
  }
}
// ==========================================
// DISPLAY MONITORING RESULTS
// ==========================================
void displayResults() {
  Serial.println();
  Serial.println("====================================");
  Serial.println("       AGRISHIELD MONITORING");
  Serial.println("====================================");

  Serial.println("\nENVIRONMENTAL MONITORING");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Air Quality Indicator: ");
  Serial.println(airQuality);

  Serial.println("\nPHYSIOLOGICAL MONITORING");

  if (baselineReady) {
    Serial.print("Personal Baseline: ");
    Serial.print(baselineHeartRate);
    Serial.println(" BPM");

    Serial.print("Heart Rate Deviation: ");
    Serial.print(heartRateDeviation);
    Serial.println(" %");
  }
  else {
    Serial.print("Baseline Calibration: ");
    Serial.print(baselineReadings);
    Serial.println("/5");
  }

  Serial.print("Heart Rate: ");
  Serial.print(heartRate);
  Serial.println(" BPM");

  Serial.print("Activity: ");
  Serial.println(activity);

  Serial.print("Exposure Duration: ");
  Serial.print(exposureMinutes);
  Serial.println(" simulated minutes");

  Serial.println("\nRISK ASSESSMENT");

  Serial.print("Heat Risk: ");
  Serial.println(heatRisk);

  Serial.print("Air Quality: ");
  Serial.println(airQualityStatus);

  Serial.print("Physiological Status: ");
  Serial.println(physiologicalStatus);

  Serial.println("\nSYSTEM STATUS");

  Serial.print("Alert: ");
  Serial.println(systemAlert);

  Serial.println("Monitoring: ACTIVE");
  Serial.println("====================================");
}

// ==========================================
// CUMULATIVE EXPOSURE DISPLAY
// ==========================================
void displayExposure() {
  Serial.println();
  Serial.println("CUMULATIVE EXPOSURE ANALYSIS");
  Serial.println("--------------------------------");

  Serial.print("Heat Intensity: ");
  Serial.println(heatIntensity, 3);

  Serial.print("Heat Exposure Burden: ");
  Serial.println(heatExposureBurden, 3);

  Serial.println();

  Serial.print("Air Quality Intensity: ");
  Serial.println(airIntensity, 3);

  Serial.print("Air Exposure Burden: ");
  Serial.println(airExposureBurden, 3);

  Serial.println("--------------------------------");
}
// ==========================================
// AWARE MODULE 03: PREDICTION DISPLAY
// ==========================================

void displayPredictions() {

  if (historyCount < TREND_WINDOW) {
    return;
  }

  Serial.println();
  Serial.println("FUTURE RISK PREDICTION");
  Serial.println("--------------------------------");
  Serial.print("Prediction Confidence: ");
  Serial.println(predictionConfidence);
  Serial.println("5-MINUTE PREDICTION");

  Serial.print("Temperature: ");
  Serial.println(predictedTemp5);

  Serial.print("Humidity: ");
  Serial.println(predictedHumidity5);

  Serial.print("Air Quality: ");
  Serial.println(predictedAir5);

  Serial.print("Heart Rate: ");
  Serial.println(predictedHeart5);

  Serial.println();

  Serial.println("15-MINUTE PREDICTION");

  Serial.print("Temperature: ");
  Serial.println(predictedTemp15);

  Serial.print("Humidity: ");
  Serial.println(predictedHumidity15);

  Serial.print("Air Quality: ");
  Serial.println(predictedAir15);

  Serial.print("Heart Rate: ");
  Serial.println(predictedHeart15);
// FUTURE RISK ASSESSMENT DISPLAY

Serial.println();
Serial.println("FUTURE RISK ASSESSMENT");
Serial.println("--------------------------------");

Serial.print("Predicted Heat Risk: ");
Serial.println(futureHeatRisk);

Serial.print("Predicted Air Quality Risk: ");
Serial.println(futureAirRisk);

Serial.print("Predicted Physiological Risk: ");
Serial.println(futurePhysiologicalRisk);

Serial.println("--------------------------------");

Serial.print("Future System Alert: ");
Serial.println(futureSystemAlert);
// 15-MINUTE FUTURE RISK DISPLAY

Serial.println();
Serial.println("15-MINUTE FUTURE RISK ASSESSMENT");
Serial.println("--------------------------------");

Serial.print("Predicted Heat Risk: ");
Serial.println(futureHeatRisk15);

Serial.print("Predicted Air Quality Risk: ");
Serial.println(futureAirRisk15);

Serial.print("Predicted Physiological Risk: ");
Serial.println(futurePhysiologicalRisk15);

Serial.println("--------------------------------");

Serial.print("15-Minute System Alert: ");
Serial.println(futureSystemAlert15);

Serial.println("--------------------------------");
}
// ==========================================
// MAIN LOOP
// ==========================================
void loop() {

  unsigned long currentTime = millis();

  if (currentTime - previousSamplingTime < samplingInterval) {
    return;
  }

  previousSamplingTime = currentTime;

  simulateSensors();
  assessHeatRisk();
  assessAirQuality();

  updatePersonalBaseline();
  assessPhysiologicalStatus();

  // Record each new sensor sample in the rolling history.
  // SENSOR SMOOTHING
updateSmoothedSensors();

// RECORD SMOOTHED SENSOR HISTORY
temperatureHistory[historyIndex] = smoothTemperature;
humidityHistory[historyIndex] = smoothHumidity;
airQualityHistory[historyIndex] = smoothAirQuality;
heartRateHistory[historyIndex] = smoothHeartRate;

historyIndex++;

if (historyIndex >= TREND_WINDOW) {
  historyIndex = 0;
}

if (historyCount < TREND_WINDOW) {
  historyCount++;
}
// UPDATE PREDICTION CONFIDENCE
updatePredictionConfidence();

// FUTURE PREDICTION
predictFutureValues();
// FUTURE RISK ASSESSMENT
if (historyCount >= TREND_WINDOW) {
  assessFutureRisk();
  assessFutureRisk15();
}
  calculateExposure();
generateAlert();
// UPDATE ADAPTIVE SAMPLING
updateSamplingInterval();
  displayResults();
  displayExposure();
displayPredictions();
}
