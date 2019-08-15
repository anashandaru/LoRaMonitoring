#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_MLX90614.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <MedianFilterLib.h>
	
	// Define DS18B20 data pin
	#define dsPin 10
    // Define Trig and Echo pin:
    #define trigPin 14
    #define echoPin 15

OneWire oneWire(dsPin); // DS18B20 thermistor
DallasTemperature ds(&oneWire);
Adafruit_ADS1115 ads; // 16-bit ADC for pH and TDS sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Contactless infrared temperature sensor
unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; //Read the gas density command for CO2 gas sensor

class Sensor {
public:
  virtual float getReading(){
    return 0.0;
  }

  float getMedian(int nsample){
    return 0.0;
  }
};

class Ds18b20 : public Sensor{
public:
  float getReading(){
    ds.requestTemperatures();   
    return ds.getTempCByIndex(0);
  }
};

class Mlx90614 : public Sensor{
public:
	float getReading(){
		return mlx.readObjectTempC();
	}
};

class Phdfrobot : public Sensor{
public:
	float getReading(){
		return (float)ads.readADC_SingleEnded(0)/(float)ads.readADC_SingleEnded(1)*5*3.5;
	}
};

class Mhz16 : public Sensor{
public:
	float getReading(){
		long hi,lo,CO2;
		Serial1.write(hexdata,9);
		//delay(100);

		for(int i=0,j=0;i<9;i++){
			if (Serial1.available()>0){
     			int ch=Serial1.read();

    			if(i==2){hi=ch;}   //High concentration
    			if(i==3){lo=ch;}   //Low concentration
    			if(i==8){CO2=hi*256+lo;}  //CO2 concentration
    		}
		}
  		return (float) CO2;
	}
};

class Jsn04t : public Sensor{
public:
	float getReading(){
	long duration;
    float distance;

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

	return distance;
	}
};

class Tdsdfrobot : public Sensor{
public:
	float getReading(){
		return 0.0;
	}
};

void setup(void)
{
  // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600); // for debuging purposes
  while(!Serial);
  Serial1.begin(9600); // C02 gas sensor interface
  //ads.setGain(GAIN_ONE);  
  ads.begin(); // pH and TDS ADC
  mlx.begin(); // Contactless IR thermometer
  ds.begin(); // DS18B20 thermistor
}

void loop(void)
{ 
  Ds18b20 tm1; Mlx90614 tm2; Phdfrobot phr;
  Tdsdfrobot tds; Jsn04t dis; Mhz16 gas;
  Serial.print(gas.getReading()); Serial.print(",");
  Serial.print(tm1.getReading()); Serial.print(",");
  Serial.print(tm2.getReading()); Serial.print(",");
  Serial.print(phr.getReading()); Serial.print(",");
  Serial.print(tds.getReading()); Serial.print(",");
  Serial.print(dis.getReading()); Serial.println();
  
  delay(1000);
}
