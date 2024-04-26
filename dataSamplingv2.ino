#include <DFRobot_SHT3x.h>
#include <SD.h>

DFRobot_SHT3x sht3x(&Wire, /*address=*/0x44, /*RST=*/4);

struct DataPoint {
  float tempC;
  float humidity;
};

const int CS_PIN = 5; // SD card CS pin
const int SAMPLE_SIZE = 100; // set before use
const float DSATEMPC = 0.2; // SEN0385 datasheet accuracy temperature C
const float DSAHUMIDITY = 2.0 / 100; // SEN0385 datasheet accuracy humidity

int n = 0; // samples retrieved
DataPoint mean = {0}; // Initialize to 0
DataPoint stdev = {0}; // Initialize to 0
File dataFile;
unsigned long startTime;

void setup() {
  Serial.begin(9600);
  delay(1000); // Give some time to open the Serial Monitor
  
  // Initialize SHT3x sensor
  while (sht3x.begin() != 0) {
    Serial.println("Failed to Initialize the chip, please confirm the wire connection");
    delay(1000);
  }
  if (!sht3x.softReset()) {
    Serial.println("Failed to Initialize the chip....");
  }

  // Initialize SD card
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }

  // Open data file
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening data.csv");
    while (1);
  }

  // Write headers in file
  dataFile.println("Time (ms), Temp (C), stdev (C), Humidity (%RH), stdev (%RH) ");
  dataFile.close();

  startTime = millis();
}

void loop() {
  float tempC = sht3x.getTemperatureC();
  float humidity = (sht3x.getHumidityRH() / 100);

  if (n < 20 || (abs(tempC - mean.tempC / n) <= 2 * DSATEMPC) && (abs(humidity - mean.humidity / n) <= 2 * DSAHUMIDITY)) { // skip outliers
    if (n < SAMPLE_SIZE) {
      mean.tempC += tempC;
      mean.humidity += humidity;
      n++;
    } else {
      mean.tempC /= n;
      mean.humidity /= n;
      mean.humidity *= 100;

      stdev.tempC = sqrt((mean.tempC / n * mean.tempC) - (mean.tempC * mean.tempC));
      stdev.humidity = sqrt((mean.humidity / n * mean.humidity) - (mean.humidity * mean.humidity));

      unsigned long elapsedTime = millis() - startTime;
      // write to file
      dataFile = SD.open("data.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.print(elapsedTime); // Time
        dataFile.print(",");
        dataFile.print(mean.tempC);
        dataFile.print(",");
        dataFile.print(stdev.tempC);
        dataFile.print(",");
        dataFile.print(mean.humidity);
        dataFile.print(",");
        dataFile.println(stdev.humidity);
        dataFile.close();
      } else {
        Serial.println("Error opening data.csv");
      }

      mean = {0};
      stdev = {0};
      n = 0;
    }
  }

  delay(1000);
}
