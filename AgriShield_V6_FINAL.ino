/*
================================================
                 AGRISHIELD V6.0
   Intelligent Agricultural Worker Safety
              Monitoring System
================================================

Platform : ESP32
Mode     : Software Sensor Simulation

AWARE MODULES:
1. Environmental Monitoring
2. Physiological Monitoring
3. Cumulative Exposure Analysis
4. Predictive Risk Analysis
5. Adaptive Sampling Control

================================================
*/

// ================================================
// SECTION 1: SYSTEM CONFIGURATION
// ================================================

// Serial communication
#define SERIAL_BAUD_RATE 115200

// Sensor simulation
#define NORMAL_TEMP 29.5
#define NORMAL_HUMIDITY 60.0
#define NORMAL_AIR 90
#define NORMAL_HEART_RATE 78

// Risk thresholds (experimental simulation values)
#define HEAT_HIGH_TEMP 35.0
#define HEAT_ELEVATED_TEMP 32.0

#define HEAT_HIGH_HUMIDITY 70.0
#define HEAT_ELEVATED_HUMIDITY 70.0

#define AIR_HIGH_THRESHOLD 300
#define AIR_ELEVATED_THRESHOLD 150

// Personal baseline
#define CALIBRATION_SAMPLES 5

// Prediction configuration
#define TREND_WINDOW 5

#define DAMPING_5_MIN 0.8
#define DAMPING_15_MIN 0.4

// Adaptive sampling intervals
#define NORMAL_INTERVAL 5000
#define ELEVATED_INTERVAL 3000
#define HIGH_INTERVAL 1000

// ================================================
// END OF SECTION 1
// ================================================
// ================================================
// SECTION 2: GLOBAL VARIABLES
// ================================================

// --------------------------------
// 2.1 SENSOR VARIABLES
// --------------------------------

float temperature = NORMAL_TEMP;
float humidity = NORMAL_HUMIDITY;

int airQuality = NORMAL_AIR;
int heartRate = NORMAL_HEART_RATE;

String activity = "LOW";

// --------------------------------
// 2.2 CURRENT RISK STATUS
// --------------------------------

String heatRisk = "NORMAL";
String airQualityStatus = "NORMAL";
String physiologicalStatus = "CALIBRATING";

String systemAlert = "NORMAL";

// --------------------------------
// 2.3 PERSONAL BASELINE
// --------------------------------

float baselineHeartRate = 0.0;
float baselineSum = 0.0;

float heartRateDeviation = 0.0;

int baselineReadings = 0;

bool baselineReady = false;

// --------------------------------
// 2.4 CUMULATIVE EXPOSURE
// --------------------------------

float heatExposureBurden = 0.0;
float airExposureBurden = 0.0;

float heatIntensity = 0.0;
float airIntensity = 0.0;

float exposureMinutes = 0.0;

// --------------------------------
// 2.5 TIME MANAGEMENT
// --------------------------------

unsigned long previousSamplingTime = 0;

unsigned long samplingInterval = NORMAL_INTERVAL;

float simulatedTimeStep = 0.0;

// --------------------------------
// 2.6 SENSOR SMOOTHING
// --------------------------------

float smoothTemperature = 0.0;
float smoothHumidity = 0.0;

float smoothAirQuality = 0.0;
float smoothHeartRate = 0.0;

float alphaTemperature = 0.2;
float alphaHumidity = 0.2;

float alphaAirQuality = 0.2;
float alphaHeartRate = 0.2;

bool smoothingInitialized = false;

// --------------------------------
// 2.7 SENSOR HISTORY
// --------------------------------

float temperatureHistory[TREND_WINDOW];
float humidityHistory[TREND_WINDOW];

float airQualityHistory[TREND_WINDOW];
float heartRateHistory[TREND_WINDOW];

float timeHistory[TREND_WINDOW];

int historyIndex = 0;
int historyCount = 0;

// --------------------------------
// 2.8 PREDICTED VALUES
// --------------------------------

// Five-minute predictions

float predictedTemp5 = 0.0;
float predictedHumidity5 = 0.0;

float predictedAir5 = 0.0;
float predictedHeart5 = 0.0;

// Fifteen-minute predictions

float predictedTemp15 = 0.0;
float predictedHumidity15 = 0.0;

float predictedAir15 = 0.0;
float predictedHeart15 = 0.0;

// --------------------------------
// 2.9 FUTURE RISK STATUS
// --------------------------------

// Five-minute future risk

String futureHeatRisk = "NORMAL";
String futureAirRisk = "NORMAL";

String futurePhysiologicalRisk = "NORMAL";
String futureSystemAlert = "NORMAL";

// Fifteen-minute future risk

String futureHeatRisk15 = "NORMAL";
String futureAirRisk15 = "NORMAL";

String futurePhysiologicalRisk15 = "NORMAL";
String futureSystemAlert15 = "NORMAL";

// --------------------------------
// 2.10 PREDICTION CONTROL
// --------------------------------

String predictionConfidence = "LOW";

const float MAX_TEMP_TREND = 0.5;
const float MAX_HUMIDITY_TREND = 2.0;

const float MAX_AIR_TREND = 30.0;
const float MAX_HEART_TREND = 5.0;

const float TEMP_PREDICTION_RANGE = 5.0;
const float HUMIDITY_PREDICTION_RANGE = 15.0;

const float AIR_PREDICTION_RANGE = 40.0;
const float HEART_PREDICTION_RANGE = 25.0;

// --------------------------------
// 2.11 SIMULATION CONTROL
// --------------------------------

int readingCount = 0;

// ================================================
// END OF SECTION 2
// ================================================
// ================================================
// SECTION 3: SENSOR SIMULATION ENGINE
// ================================================

void simulateSensors() {

  int scenario = (readingCount / 5) % 4;

  // SCENARIO 1: NORMAL
  if (scenario == 0) {

    temperature = 29.5;
    humidity = 60.0;
    airQuality = 90;
    heartRate = 78;
    activity = "LOW";
  }

  // SCENARIO 2: ELEVATED
  else if (scenario == 1) {

    temperature = 33.0;
    humidity = 65.0;
    airQuality = 180;
    heartRate = 90;
    activity = "MODERATE";
  }

  // SCENARIO 3: HIGH
  else if (scenario == 2) {

    temperature = 37.0;
    humidity = 82.0;
    airQuality = 320;
    heartRate = 125;
    activity = "HIGH";
  }

  // SCENARIO 4: RECOVERY
  else {

    temperature = 29.5;
    humidity = 60.0;
    airQuality = 90;
    heartRate = 78;
    activity = "LOW";
  }

  readingCount++;
}

// ================================================
// END OF SECTION 3
// ================================================
// ================================================
// SECTION 4: ADAPTIVE EMA SENSOR SMOOTHING
// ================================================

float adaptiveEMA(float currentValue,
                  float previousValue,
                  float &alpha) {

  float change = fabs(currentValue - previousValue);

  if (change > 10.0) {
    alpha = 0.7;
  }
  else if (change > 5.0) {
    alpha = 0.5;
  }
  else {
    alpha = 0.2;
  }

  return (alpha * currentValue) +
         ((1.0 - alpha) * previousValue);
}


// SENSOR SMOOTHING MANAGER

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


// ================================================
// SECTION 5: ENVIRONMENTAL RISK ASSESSMENT
// ================================================

// HEAT RISK

void assessHeatRisk() {

  if (temperature >= HEAT_HIGH_TEMP &&
      humidity >= HEAT_HIGH_HUMIDITY) {

    heatRisk = "HIGH";
  }

  else if (temperature >= HEAT_ELEVATED_TEMP ||
           humidity >= HEAT_ELEVATED_HUMIDITY) {

    heatRisk = "ELEVATED";
  }

  else {
    heatRisk = "NORMAL";
  }
}


// AIR QUALITY RISK

void assessAirQuality() {

  if (airQuality >= AIR_HIGH_THRESHOLD) {
    airQualityStatus = "HIGH";
  }

  else if (airQuality >= AIR_ELEVATED_THRESHOLD) {
    airQualityStatus = "ELEVATED";
  }

  else {
    airQualityStatus = "NORMAL";
  }
}

// ================================================
// END OF SECTIONS 4 & 5
// ================================================
// ================================================
// SECTION 6: PERSONAL HEART RATE BASELINE
// ================================================

void updatePersonalBaseline() {

  if (!baselineReady) {

    baselineSum += heartRate;
    baselineReadings++;

    if (baselineReadings >= CALIBRATION_SAMPLES) {

      baselineHeartRate =
        baselineSum / CALIBRATION_SAMPLES;

      baselineReady = true;

      Serial.println("\nBASELINE CALIBRATION COMPLETE");
      Serial.print("Personal Baseline: ");
      Serial.print(baselineHeartRate);
      Serial.println(" BPM");
    }

    return;
  }

  if (heatRisk == "NORMAL" &&
      airQualityStatus == "NORMAL" &&
      activity == "LOW" &&
      fabs(heartRate - baselineHeartRate)
          <= baselineHeartRate * 0.10) {

    baselineHeartRate =
      (0.98 * baselineHeartRate) +
      (0.02 * heartRate);
  }
}


// ================================================
// SECTION 7: PHYSIOLOGICAL RISK ASSESSMENT
// ================================================

void assessPhysiologicalStatus() {

  if (!baselineReady) {
    physiologicalStatus = "CALIBRATING";
    return;
  }

  heartRateDeviation =
    ((heartRate - baselineHeartRate) /
      baselineHeartRate) * 100.0;

  if (heartRateDeviation > 20.0) {
    physiologicalStatus = "HIGH";
  }

  else if (heartRateDeviation > 10.0) {
    physiologicalStatus = "ELEVATED";
  }

  else {
    physiologicalStatus = "NORMAL";
  }
}


// ================================================
// SECTION 8: CUMULATIVE EXPOSURE ANALYSIS
// ================================================

void calculateExposure() {

  heatIntensity = 0.0;
  airIntensity = 0.0;

  if (temperature > 30.0) {
    heatIntensity += (temperature - 30.0) / 10.0;
  }

  if (humidity > 60.0) {
    heatIntensity += (humidity - 60.0) / 40.0;
  }

  if (airQuality > 100) {
    airIntensity = (airQuality - 100) / 200.0;
  }

  heatExposureBurden +=
    heatIntensity * simulatedTimeStep;

  airExposureBurden +=
    airIntensity * simulatedTimeStep;
}

// ================================================
// END OF SECTIONS 6, 7 & 8
// ================================================
// ================================================
// SECTION 9: PREDICTIVE RISK ENGINE
// ================================================

// 9.1 STORE SENSOR HISTORY

void storeSensorHistory() {

  temperatureHistory[historyIndex] = smoothTemperature;
  humidityHistory[historyIndex] = smoothHumidity;
  airQualityHistory[historyIndex] = smoothAirQuality;
  heartRateHistory[historyIndex] = smoothHeartRate;

  timeHistory[historyIndex] = exposureMinutes;

  historyIndex = (historyIndex + 1) % TREND_WINDOW;

  if (historyCount < TREND_WINDOW) {
    historyCount++;
  }
}


// 9.2 TIME-AWARE LINEAR REGRESSION

float calculateTrend(float data[]) {

  if (historyCount < 2) {
    return 0.0;
  }

  int oldestIndex =
    (historyCount < TREND_WINDOW) ? 0 : historyIndex;

  float sumX = 0.0;
  float sumY = 0.0;
  float sumXY = 0.0;
  float sumX2 = 0.0;

  for (int i = 0; i < historyCount; i++) {

    int index = (oldestIndex + i) % TREND_WINDOW;

    float x = timeHistory[index] -
              timeHistory[oldestIndex];

    float y = data[index];

    sumX += x;
    sumY += y;
    sumXY += x * y;
    sumX2 += x * x;
  }

  float n = historyCount;

  float denominator =
    (n * sumX2) - (sumX * sumX);

  if (fabs(denominator) < 0.000001) {
    return 0.0;
  }

  return ((n * sumXY) - (sumX * sumY))
         / denominator;
}


// 9.3 LIMIT SENSOR TRENDS

float limitTrend(float trend, float maximum) {

  return constrain(trend, -maximum, maximum);
}


// 9.4 LIMIT PREDICTION DEVIATION

float limitPrediction(float prediction,
                      float currentValue,
                      float allowedRange) {

  return constrain(
    prediction,
    currentValue - allowedRange,
    currentValue + allowedRange
  );
}


// 9.5 PREDICTION CONFIDENCE LABEL

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


// 9.6 FUTURE VALUE PREDICTION

void predictFutureValues() {

  if (historyCount < TREND_WINDOW) {
    return;
  }

  float tempTrend =
    calculateTrend(temperatureHistory);

  float humidityTrend =
    calculateTrend(humidityHistory);

  float airTrend =
    calculateTrend(airQualityHistory);

  float heartTrend =
    calculateTrend(heartRateHistory);


  // Limit trends

  tempTrend = limitTrend(tempTrend, MAX_TEMP_TREND);

  humidityTrend =
    limitTrend(humidityTrend, MAX_HUMIDITY_TREND);

  airTrend = limitTrend(airTrend, MAX_AIR_TREND);

  heartTrend = limitTrend(heartTrend, MAX_HEART_TREND);


  // 5-MINUTE PREDICTIONS

  predictedTemp5 =
    smoothTemperature + tempTrend * 5 * DAMPING_5_MIN;

  predictedHumidity5 =
    smoothHumidity + humidityTrend * 5 * DAMPING_5_MIN;

  predictedAir5 =
    smoothAirQuality + airTrend * 5 * DAMPING_5_MIN;

  predictedHeart5 =
    smoothHeartRate + heartTrend * 5 * DAMPING_5_MIN;


  // 15-MINUTE PREDICTIONS

  predictedTemp15 =
    smoothTemperature + tempTrend * 15 * DAMPING_15_MIN;

  predictedHumidity15 =
    smoothHumidity + humidityTrend * 15 * DAMPING_15_MIN;

  predictedAir15 =
    smoothAirQuality + airTrend * 15 * DAMPING_15_MIN;

  predictedHeart15 =
    smoothHeartRate + heartTrend * 15 * DAMPING_15_MIN;


  // LIMIT PREDICTION DEVIATION

  predictedTemp5 =
    limitPrediction(predictedTemp5, smoothTemperature,
                    TEMP_PREDICTION_RANGE);

  predictedTemp15 =
    limitPrediction(predictedTemp15, smoothTemperature,
                    TEMP_PREDICTION_RANGE);

  predictedHumidity5 =
    limitPrediction(predictedHumidity5, smoothHumidity,
                    HUMIDITY_PREDICTION_RANGE);

  predictedHumidity15 =
    limitPrediction(predictedHumidity15, smoothHumidity,
                    HUMIDITY_PREDICTION_RANGE);

  predictedAir5 =
    limitPrediction(predictedAir5, smoothAirQuality,
                    AIR_PREDICTION_RANGE);

  predictedAir15 =
    limitPrediction(predictedAir15, smoothAirQuality,
                    AIR_PREDICTION_RANGE);

  predictedHeart5 =
    limitPrediction(predictedHeart5, smoothHeartRate,
                    HEART_PREDICTION_RANGE);

  predictedHeart15 =
    limitPrediction(predictedHeart15, smoothHeartRate,
                    HEART_PREDICTION_RANGE);

// ================================================
// RECOVERY PREDICTION STABILIZATION
// ================================================

// HEART RATE RECOVERY

if (heartTrend < 0 && heartRate <= baselineHeartRate) {

  predictedHeart5 = max(
    predictedHeart5,
    baselineHeartRate
  );

  predictedHeart15 = max(
    predictedHeart15,
    baselineHeartRate
  );
}


// TEMPERATURE RECOVERY

if (tempTrend < 0 && temperature < smoothTemperature) {

  predictedTemp5 = max(
    predictedTemp5,
    temperature
  );

  predictedTemp15 = max(
    predictedTemp15,
    temperature
  );
}


// HUMIDITY RECOVERY

if (humidityTrend < 0 && humidity < smoothHumidity) {

  predictedHumidity5 = max(
    predictedHumidity5,
    humidity
  );

  predictedHumidity15 = max(
    predictedHumidity15,
    humidity
  );
}


// AIR QUALITY RECOVERY

if (airTrend < 0 && airQuality < smoothAirQuality) {

  predictedAir5 = max(
    predictedAir5,
    (float)airQuality
  );

  predictedAir15 = max(
    predictedAir15,
    (float)airQuality
  );
}
  // ABSOLUTE SAFETY BOUNDS

  predictedTemp5 = constrain(predictedTemp5, 0.0, 60.0);
  predictedTemp15 = constrain(predictedTemp15, 0.0, 60.0);

  predictedHumidity5 =
    constrain(predictedHumidity5, 0.0, 100.0);

  predictedHumidity15 =
    constrain(predictedHumidity15, 0.0, 100.0);

  predictedAir5 = constrain(predictedAir5, 0.0, 1000.0);
  predictedAir15 = constrain(predictedAir15, 0.0, 1000.0);

  predictedHeart5 =
    constrain(predictedHeart5, 30.0, 220.0);

  predictedHeart15 =
    constrain(predictedHeart15, 30.0, 220.0);
}

// ================================================
// END OF SECTION 9
// ================================================
// ================================================
// SECTION 10: FUTURE RISK ASSESSMENT
// ================================================

// 10.1 FIVE-MINUTE FUTURE RISK

void assessFutureRisk() {

  // HEAT RISK
  if (predictedTemp5 >= HEAT_HIGH_TEMP &&
      predictedHumidity5 >= HEAT_HIGH_HUMIDITY) {

    futureHeatRisk = "HIGH";
  }
  else if (predictedTemp5 >= HEAT_ELEVATED_TEMP ||
           predictedHumidity5 >= HEAT_ELEVATED_HUMIDITY) {

    futureHeatRisk = "ELEVATED";
  }
  else {
    futureHeatRisk = "NORMAL";
  }

  // AIR QUALITY RISK
  if (predictedAir5 >= AIR_HIGH_THRESHOLD) {
    futureAirRisk = "HIGH";
  }
  else if (predictedAir5 >= AIR_ELEVATED_THRESHOLD) {
    futureAirRisk = "ELEVATED";
  }
  else {
    futureAirRisk = "NORMAL";
  }

  // PHYSIOLOGICAL RISK
  if (baselineReady && baselineHeartRate > 0) {

    float deviation =
      ((predictedHeart5 - baselineHeartRate) /
       baselineHeartRate) * 100.0;

    if (deviation > 20.0) {
      futurePhysiologicalRisk = "HIGH";
    }
    else if (deviation > 10.0) {
      futurePhysiologicalRisk = "ELEVATED";
    }
    else {
      futurePhysiologicalRisk = "NORMAL";
    }
  }
  else {
    futurePhysiologicalRisk = "CALIBRATING";
  }

  // FIVE-MINUTE SYSTEM ALERT
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


// ================================================
// 10.2 FIFTEEN-MINUTE FUTURE RISK
// ================================================

void assessFutureRisk15() {

  // HEAT RISK
  if (predictedTemp15 >= HEAT_HIGH_TEMP &&
      predictedHumidity15 >= HEAT_HIGH_HUMIDITY) {

    futureHeatRisk15 = "HIGH";
  }
  else if (predictedTemp15 >= HEAT_ELEVATED_TEMP ||
           predictedHumidity15 >= HEAT_ELEVATED_HUMIDITY) {

    futureHeatRisk15 = "ELEVATED";
  }
  else {
    futureHeatRisk15 = "NORMAL";
  }

  // AIR QUALITY RISK
  if (predictedAir15 >= AIR_HIGH_THRESHOLD) {
    futureAirRisk15 = "HIGH";
  }
  else if (predictedAir15 >= AIR_ELEVATED_THRESHOLD) {
    futureAirRisk15 = "ELEVATED";
  }
  else {
    futureAirRisk15 = "NORMAL";
  }

  // PHYSIOLOGICAL RISK
  if (baselineReady && baselineHeartRate > 0) {

    float deviation =
      ((predictedHeart15 - baselineHeartRate) /
       baselineHeartRate) * 100.0;

    if (deviation > 20.0) {
      futurePhysiologicalRisk15 = "HIGH";
    }
    else if (deviation > 10.0) {
      futurePhysiologicalRisk15 = "ELEVATED";
    }
    else {
      futurePhysiologicalRisk15 = "NORMAL";
    }
  }
  else {
    futurePhysiologicalRisk15 = "CALIBRATING";
  }

  // FIFTEEN-MINUTE SYSTEM ALERT
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


// ================================================
// SECTION 11: CURRENT SYSTEM ALERT
// ================================================

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

// ================================================
// END OF SECTIONS 10 & 11
// ================================================
// ================================================
// SECTION 12: ADAPTIVE SAMPLING CONTROL
// ================================================

void updateSamplingInterval() {

  // HIGH RISK: SAMPLE EVERY 1 SECOND

  if (heatRisk == "HIGH" ||
      airQualityStatus == "HIGH" ||
      physiologicalStatus == "HIGH" ||
      futureSystemAlert == "FUTURE URGENT WARNING" ||
      futureSystemAlert15 == "FUTURE URGENT WARNING") {

    samplingInterval = HIGH_INTERVAL;
  }

  // ELEVATED RISK: SAMPLE EVERY 3 SECONDS

  else if (heatRisk == "ELEVATED" ||
           airQualityStatus == "ELEVATED" ||
           physiologicalStatus == "ELEVATED" ||
           futureSystemAlert == "FUTURE CAUTION" ||
           futureSystemAlert15 == "FUTURE CAUTION") {

    samplingInterval = ELEVATED_INTERVAL;
  }

  // NORMAL: SAMPLE EVERY 5 SECONDS

  else {
    samplingInterval = NORMAL_INTERVAL;
  }
}


// ================================================
// SECTION 13: SERIAL MONITOR DASHBOARD
// ================================================

void displayResults() {

  Serial.println("\n========================================");
  Serial.println("       AGRISHIELD V6.0 MONITORING");
  Serial.println("========================================");

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

  Serial.print("Heart Rate: ");
  Serial.print(heartRate);
  Serial.println(" BPM");

  Serial.print("Activity: ");
  Serial.println(activity);

  Serial.print("Personal Baseline: ");
  Serial.print(baselineHeartRate);
  Serial.println(" BPM");

  Serial.print("Heart Rate Deviation: ");
  Serial.print(heartRateDeviation);
  Serial.println(" %");

  Serial.println("\nCURRENT RISK ASSESSMENT");

  Serial.print("Heat Risk: ");
  Serial.println(heatRisk);

  Serial.print("Air Quality Risk: ");
  Serial.println(airQualityStatus);

  Serial.print("Physiological Risk: ");
  Serial.println(physiologicalStatus);

  Serial.print("SYSTEM ALERT: ");
  Serial.println(systemAlert);

  Serial.println("\nCUMULATIVE EXPOSURE");

  Serial.print("Exposure Duration: ");
  Serial.print(exposureMinutes, 2);
  Serial.println(" simulated minutes");

  Serial.print("Heat Exposure Burden: ");
  Serial.println(heatExposureBurden, 3);

  Serial.print("Air Exposure Burden: ");
  Serial.println(airExposureBurden, 3);

  Serial.println("\nPREDICTION CONFIDENCE");
  Serial.println(predictionConfidence);

  Serial.println("========================================");
}


// ================================================
// 13.1: FUTURE PREDICTION DISPLAY
// ================================================

void displayPredictions() {

  if (historyCount < TREND_WINDOW) {
    Serial.println("Collecting prediction history...");
    return;
  }

  Serial.println("\n--- 5-MINUTE PREDICTION ---");

  Serial.print("Temperature: ");
  Serial.println(predictedTemp5);

  Serial.print("Humidity: ");
  Serial.println(predictedHumidity5);

  Serial.print("Air Quality: ");
  Serial.println(predictedAir5);

  Serial.print("Heart Rate: ");
  Serial.println(predictedHeart5);

  Serial.print("Future Alert: ");
  Serial.println(futureSystemAlert);


  Serial.println("\n--- 15-MINUTE PREDICTION ---");

  Serial.print("Temperature: ");
  Serial.println(predictedTemp15);

  Serial.print("Humidity: ");
  Serial.println(predictedHumidity15);

  Serial.print("Air Quality: ");
  Serial.println(predictedAir15);

  Serial.print("Heart Rate: ");
  Serial.println(predictedHeart15);

  Serial.print("Future Alert: ");
  Serial.println(futureSystemAlert15);
}


// ================================================
// 13.2: ADAPTIVE SAMPLING DISPLAY
// ================================================

void displaySamplingStatus() {

  Serial.println("\n--- ADAPTIVE SAMPLING ---");

  Serial.print("Sampling Interval: ");
  Serial.print(samplingInterval);
  Serial.println(" ms");

  if (samplingInterval == HIGH_INTERVAL) {
    Serial.println("Mode: HIGH RISK");
  }

  else if (samplingInterval == ELEVATED_INTERVAL) {
    Serial.println("Mode: ELEVATED RISK");
  }

  else {
    Serial.println("Mode: NORMAL");
  }

  Serial.println("========================================");
}


// ================================================
// SECTION 14: SYSTEM INITIALIZATION
// ================================================

void setup() {

  Serial.begin(SERIAL_BAUD_RATE);

  Serial.println("\n");
  Serial.println("========================================");
  Serial.println("          AGRISHIELD V6.0");
  Serial.println(" Intelligent Agricultural Worker Safety");
  Serial.println("========================================");

  Serial.println("Initializing system...");

  delay(1000);

  Serial.println("Sensor Simulation: ACTIVE");
  Serial.println("Predictive Engine: READY");
  Serial.println("Adaptive Sampling: READY");

  Serial.println("\nAGRISHIELD SYSTEM READY!");
}


// ================================================
// SECTION 15: MAIN SYSTEM LOOP
// ================================================

void loop() {

  unsigned long currentTime = millis();

  if (currentTime - previousSamplingTime < samplingInterval) {
    return;
  }

  // Calculate actual elapsed time

  unsigned long elapsedMillis;

  if (previousSamplingTime == 0) {
    elapsedMillis = samplingInterval;
  }
  else {
    elapsedMillis = currentTime - previousSamplingTime;
  }

  previousSamplingTime = currentTime;

  // 5000 ms = 1 simulated minute

  simulatedTimeStep = elapsedMillis / 5000.0;

  exposureMinutes += simulatedTimeStep;


  // SENSOR SIMULATION

  simulateSensors();


  // CURRENT RISK ASSESSMENT

  assessHeatRisk();
  assessAirQuality();

  updatePersonalBaseline();
  assessPhysiologicalStatus();


  // SENSOR SMOOTHING

  updateSmoothedSensors();


  // STORE SENSOR HISTORY

  storeSensorHistory();


  // PREDICTION CONFIDENCE

  updatePredictionConfidence();


  // FUTURE PREDICTION

  predictFutureValues();


  // FUTURE RISK ASSESSMENT

  if (historyCount >= TREND_WINDOW) {

    assessFutureRisk();
    assessFutureRisk15();
  }


  // CUMULATIVE EXPOSURE

  calculateExposure();


  // CURRENT ALERT

  generateAlert();


  // ADAPTIVE SAMPLING

  updateSamplingInterval();


  // DISPLAY SYSTEM OUTPUT

  displayResults();
  displayPredictions();
  displaySamplingStatus();
}

// ================================================
// END OF AGRISHIELD V6.0
// ================================================