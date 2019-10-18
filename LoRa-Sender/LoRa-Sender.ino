 #include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SleepyDog.h>
#include <Telekelud.h>
#include <SparkFunMAX31855k.h>
#include <Wire.h>
#include <I2C_Anything.h>

#define MAXCS   1

const byte MY_ADDRESS = 42;
const byte localAddress = 0xCC;
const byte destination = 0xAA;
//long interval = 256000;
long interval = 16000;
//int dummy = 10;

SparkFunMAX31855k probe(MAXCS);
Telekelud kld(localAddress, destination, interval);
volatile datpac packet;
datpac truePac;
volatile boolean dataUpdated = false;

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);
  delay(200);
  kld.start();
  kld.configure();
  
  digitalWrite(13, HIGH);
  delay(8000); // Let IC stabilize or first readings will be garbage
  
  digitalWrite(13, LOW);
  updatePacket();
  kld.setPacket(truePac);
  kld.sendMessage();
  //delay(10000); // Let IC stabilize or first readings will be garbage
}

void loop() {
  delay(10000); // dummy sleep
  //kld.sleep(); // real sleep
  digitalWrite(13, HIGH);
  if (!dataUpdated) return;
  delay(8000); // waiting data from sensor concentrator
  digitalWrite(13, LOW);
  updatePacket();
  kld.setPacket(truePac);
  kld.sendMessage();
  dataUpdated = false;
}

void updatePacket(){
  truePac.gas = packet.gas;
  truePac.tm1 = packet.tm1;
  truePac.tm2 = packet.tm2;
  truePac.ph = packet.ph;
  truePac.tds = packet.tds;
  truePac.dis = packet.dis;
}


// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
{
  if (howMany >= 24)
  {
    Serial.println("Hit");
    I2C_readAnything (packet.gas);
    I2C_readAnything (packet.tm1);
    I2C_readAnything (packet.tm2);
    I2C_readAnything (packet.ph);
    I2C_readAnything (packet.tds);
    I2C_readAnything (packet.dis);
    dataUpdated = true;
  }  // end if have enough data
}  // end of receiveEvent

int readTemp() {
  // Make sure SPI for LoRa set to HIGH (inactive)
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  int temp = (int)(probe.readTempC() * 100); //23.25 => 2325

  // Make sure SPI for MAX set to HIGH (inactive)
  pinMode(MAXCS, OUTPUT);
  digitalWrite(MAXCS, HIGH);
  if (isnan(temp)) return 9999;
  return temp;
}

int readCJT() {
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  int temp = (int)(probe.readCJT() * 100); //23.25 => 2325
  DEBUG_PRINTLN(temp);
  // Make sure SPI for MAX set to HIGH (inactive)
  pinMode(MAXCS, OUTPUT);
  digitalWrite(MAXCS, HIGH);
  if (isnan(temp)) return 9999;
  return temp;
}
