#include <Wire.h>
#include <I2C_Anything.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_MLX90614.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <MedianFilterLib.h>
#include <VL53L1X.h>
#include "GravityTDS.h"
  
	  // Define DS18B20 data pin
	  #define dsPin 9

const byte SLAVE_ADDRESS = 42;
int nsample = 10;
MedianFilter<float> medianFilter(nsample);

OneWire oneWire(dsPin); // DS18B20 thermistor
DallasTemperature ds(&oneWire);
DeviceAddress tempSensor = {0x28, 0x2F, 0xDA, 0x46, 0x92, 0x15, 0x2, 0x4E};
Adafruit_ADS1115 ads; // 16-bit ADC for pH and TDS sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // Contactless infrared temperature sensor
unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; //Read the gas density command for CO2 gas sensor
VL53L1X lidar;
GravityTDS gravityTds; // TDS object

class Sensor {
public:
  virtual float getReading(){
    return 0.0;
  }

  float getMedian(){
    for (int i = 0; i < nsample-1; ++i){
    	medianFilter.AddValue(getReading());
      delay(5); //stability
    }
    float result = medianFilter.AddValue(getReading());
    return result;
  }
};

class Ds18b20 : public Sensor{
public:
  float getReading(){
    ds.requestTemperaturesByAddress(tempSensor);
    return ds.getTempC(tempSensor);
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

class Vl53l1x : public Sensor{
  public:
    float getReading(){
      float r = lidar.read();
      return r;
    }
};

class Tdsdfrobot : public Sensor{
public:
	float getReading(){
		gravityTds.setTemperature(mlx.readObjectTempC());
    float voltage = (float)ads.readADC_SingleEnded(2)/(float)ads.readADC_SingleEnded(1)*5;
    gravityTds.update(voltage);
		return gravityTds.getTdsValue();;
	}
};

void setup(void)
{
  Serial.begin(9600);
  //while(!Serial);
  Serial.println("Sensor Concentrator");
  Wire.begin ();
  Serial1.begin(9600); // C02 gas sensor interface
  //ads.setGain(GAIN_ONE);  
  ads.begin(); // pH and TDS ADC
  mlx.begin(); // Contactless IR thermometer
  ds.begin(); // DS18B20 thermistor
  ds.setResolution(tempSensor, 11);
    lidar.setTimeout(500);
    lidar.init();
    lidar.setDistanceMode(VL53L1X::Long);
    lidar.setMeasurementTimingBudget(50000);
    lidar.startContinuous(50);

  gravityTds.begin(); // gravity Tds

  Serial.println("All sensor set");
}

void loop(void)
{ 
  float Tgas, Ttm1, Ttm2, Tphr, Ttds, Tdis;
  Ds18b20 tm1; Mlx90614 tm2; Phdfrobot phr;
  Tdsdfrobot tds; Vl53l1x dis; Mhz16 gas;

  Tgas = gas.getReading();
  Ttm1 = tm1.getMedian();
  Ttm2 = tm2.getMedian();
  Tphr = phr.getMedian();
  Ttds = tds.getMedian();
  Tdis = dis.getMedian();

  Serial.print(Tgas); Serial.print(", ");
  Serial.print(Ttm1); Serial.print(", ");
  Serial.print(Ttm2); Serial.print(", ");
  Serial.print(Tphr); Serial.print(", ");
  Serial.print(Ttds); Serial.print(", ");
  Serial.println(Tdis);

  Wire.beginTransmission (SLAVE_ADDRESS);
  I2C_writeAnything (Tgas);
  I2C_writeAnything (Ttm1);
  I2C_writeAnything (Ttm2);
  I2C_writeAnything (Tphr);
  I2C_writeAnything (Ttds);
  I2C_writeAnything (Tdis);
  Wire.endTransmission ();
  
  delay(200);
}
