 #include <DFRobot_SHT3x.h>
 #include <SD.h>
DFRobot_SHT3x sht3x(&Wire,/*address=*/0x44,/*RST=*/4);

typedef struct{
  float tempC; 
  float tempF;
  float humidity; 
} data_point; //1 sample

/**
data_point data[100];
*/
int CS_PIN = 5; // SD card CS pin 
int SAMPLE_SIZE = 100; // set before use
float dsatempC = 0.2; //SEN0385 datasheet accuracy temperature C
float dsahumidity = 2.0/100; //SEN0385 datasheet accuracy humidity






int n = 0; // samples retrieved
data_point mean;
data_point stdev;

File dataFile;

unsigned long startTime;

void setup(){
  Serial.begin(9600);

  /**
   * CAN POSSIBLY ADD PIEZZO BUZZER FOR ERRORS
   */
  while (sht3x.begin() != 0) {
    Serial.println("Failed to Initialize the chip, please confirm the wire connection");
    delay(1000);
  }
  if(!sht3x.softReset()){
    Serial.println("Failed to Initialize the chip....");
  }

  if(!SD.begin(CS_PIN)){
    Serial.println("SD card initialization failed!");
    while(1);
  }
  
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening data.csv");
    while(1);
  }

  // Write headers in file
  dataFile.println("Time (ms), Temp (C), stdev (C), Temp (F), stdev (F), Humditity (%RH), stdev (%RH) ");
  dataFile.close();
  startTime = millis();
}

void loop(){
    float tempC = sht3x.getTemperatureC();
    float tempF = sht3x.getTemperatureF();
    float humidity = (sht3x.getHumidityRH()/100);
  if(n<20 || (abs(tempC - mean.tempC/n)<=2*dsatempC) && (abs(humidity - mean.humidity/n)<=2*dsahumidity)){
            
          
    if(n<SAMPLE_SIZE){
      
      
      
      mean.tempC += tempC;
      mean.tempF += tempF;
      mean.humidity += humidity;
      n++;
    }
    else{ // process data 
      //calculate mean
      mean.tempC /= n;
      mean.tempF /= n;
      mean.humidity /= n;
      mean.humidity*=100; 
  
      //population standard deviation (might have overflow??)
      stdev.tempC = sqrt((mean.tempC/n *mean.tempC) - (mean.tempC*mean.tempC)); 
      stdev.tempF = sqrt((mean.tempF/n *mean.tempF) - (mean.tempF*mean.tempF)); 
      stdev.humidity = sqrt((mean.humidity/n *mean.humidity) - (mean.humidity*mean.humidity));
  
      unsigned long elapsedTime = millis() - startTime; 
      
      // store on SD
      dataFile = SD.open("data.csv", FILE_WRITE);
      
      dataFile.print(elapsedTime); //time
      dataFile.print(",");
      dataFile.print(mean.tempC);
      dataFile.print(",");
      dataFile.print(stdev.tempC);
      dataFile.print(",");
      dataFile.print(mean.tempF);
      dataFile.print(",");
      dataFile.print(stdev.tempF);
      dataFile.print(",");
      dataFile.print(mean.humidity);
      dataFile.print(",");
      dataFile.println(stdev.humidity);
  
      // close file
      dataFile.close();
      
      mean.tempC = 0;
      mean.tempF = 0;
      mean.humidity = 0;
  
      stdev.tempC = 0;
      stdev.tempF = 0;
      stdev.humidity = 0;
      
      n = 0;
      
    }
  }
  
  delay(1000);
}
