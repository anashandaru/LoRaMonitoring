#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_MLX90614.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
    // Define Trig and Echo pin:
    #define trigPin 14
    #define echoPin 15
    // Define variables:
    long duration;
    int distance;

// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; //Read the gas density command /Don't change the order
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

void setup(void)
{
        // Define inputs and outputs
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
  
  Serial.begin(9600);
  Serial.println("Hello!");
  
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  Serial1.begin(9600);
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();
  mlx.begin();
  sensors.begin();  
}

void loop(void)
{
  int16_t results;
  
  /* Be sure to update this value based on the IC and the gain settings! */
  float   multiplier = 0.03F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  //float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  results = ads.readADC_Differential_0_1();  
    
  Serial.print(results * multiplier); Serial.print(",");
  Serial.print(mlx.readObjectTempC());Serial.print(",");

  sensors.requestTemperatures();
  Serial.print(sensors.getTempCByIndex(0));


      // Clear the trigPin by setting it LOW:
      digitalWrite(trigPin, LOW);
      
      delayMicroseconds(5);
     // Trigger the sensor by setting the trigPin high for 10 microseconds:
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      
      // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
      duration = pulseIn(echoPin, HIGH);
      
      // Calculate the distance:
      distance = duration*0.034/2;
      
      // Print the distance on the Serial Monitor (Ctrl+Shift+M):
      Serial.print(",");
      Serial.print(distance);

  
   Serial1.write(hexdata,9);
   delay(500);

 for(int i=0,j=0;i<9;i++)
 {
  if (Serial1.available()>0)
  {
     long hi,lo,CO2;
     int ch=Serial1.read();

    if(i==2){     hi=ch;   }   //High concentration
    if(i==3){     lo=ch;   }   //Low concentration
    if(i==8) {
               CO2=hi*256+lo;  //CO2 concentration
      Serial.print(",");
      Serial.println(CO2);
      //Serial.println(",");
               }
  }
 }
  delay(1000);
}
