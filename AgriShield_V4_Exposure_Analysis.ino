/*
========================================
             AGRISHIELD
    Intelligent Agricultural Worker
       Safety Monitoring System

    Version: 2.0
    Platform: ESP32
    Mode: Software Sensor Simulation
========================================
*/

float temperature;
float humidity;

int airQuality;
int heartRate;

int exposureMinutes = 0;

String activity;
String heatRisk;
String airQualityStatus;
String physiologicalStatus;
String systemAlert;

int readingCount = 0;
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

void updatePersonalBaseline() {

  // Initial calibration

  if (!baselineReady) {

    baselineSum += heartRate;

    baselineReadings++;

    if (baselineReadings >= CALIBRATION_SAMPLES) {

      baselineHeartRate =
          baselineSum / CALIBRATION_SAMPLES;

      baselineReady = true;

      Serial.println();
      Serial.println("BASELINE CALIBRATION COMPLETE!");

      Serial.print("Personal Heart Rate Baseline: ");

      Serial.print(baselineHeartRate);

      Serial.println(" BPM");
    }

    return;
  }

  // Adaptive baseline updating

  if (heatRisk == "NORMAL" &&
      airQualityStatus == "NORMAL" &&
      activity == "LOW" &&
      abs(heartRate - baselineHeartRate) <=
          baselineHeartRate * 0.10) {

    baselineHeartRate =
        (0.98 * baselineHeartRate) +
        (0.02 * heartRate);
  }
}
// ======================================
// AWARE MODULE 02
// CUMULATIVE EXPOSURE ANALYSIS
// ======================================

void calculateExposure() {

  // HEAT EXPOSURE INTENSITY

  heatIntensity = 0.0;

  if (temperature > 30.0) {

    heatIntensity +=
        (temperature - 30.0) / 10.0;

  }

  if (humidity > 60.0) {

    heatIntensity +=
        (humidity - 60.0) / 40.0;

  }


  // AIR QUALITY EXPOSURE INTENSITY

  airIntensity = 0.0;

  if (airQuality > 100) {

    airIntensity =
        (airQuality - 100) / 200.0;

  }


  // CUMULATIVE EXPOSURE

  heatExposureBurden +=
      heatIntensity * simulatedTimeStep;

  airExposureBurden +=
      airIntensity * simulatedTimeStep;

}
// ======================================
// SETUP
// ======================================

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


// ======================================
// SENSOR SIMULATION
// ======================================

void simulateSensors() {

  int scenario = (readingCount / 5) % 3;

  if (scenario == 0) {

    // NORMAL CONDITIONS

    temperature = 29.5;
    humidity = 60.0;

    airQuality = 90;
    heartRate = 78;

    activity = "LOW";

  }

  else if (scenario == 1) {

    // ELEVATED CONDITIONS

    temperature = 34.5;
    humidity = 72.0;

    airQuality = 210;
    heartRate = 105;

    activity = "MODERATE";

  }

  else {

    // HIGH-RISK TEST SCENARIO

    temperature = 37.0;
    humidity = 82.0;

    airQuality = 320;
    heartRate = 125;

    activity = "HIGH";

  }

  readingCount++;

  exposureMinutes++;
}


// ======================================
// HEAT RISK ASSESSMENT
// ======================================

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


// ======================================
// AIR QUALITY ASSESSMENT
// ======================================

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


// ======================================
// PHYSIOLOGICAL ASSESSMENT
// ======================================

void assessPhysiologicalStatus() {

  if (!baselineReady) {

    physiologicalStatus = "CALIBRATING";

    return;
  }

  heartRateDeviation =
      ((heartRate - baselineHeartRate) /
       baselineHeartRate) * 100.0;

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

// ======================================
// ALERT GENERATION
// ======================================

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


// ======================================
// DISPLAY RESULTS
// ======================================

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
  // ======================================
// EXPOSURE DISPLAY
// ======================================

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

// ======================================
// MAIN LOOP
// ======================================

void loop() {

  simulateSensors();

  assessHeatRisk();

  assessAirQuality();

  updatePersonalBaseline();

  assessPhysiologicalStatus();

  calculateExposure();

  generateAlert();

  displayResults();

  displayExposure();

  delay(3000);

}
