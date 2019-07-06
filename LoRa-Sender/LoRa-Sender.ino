#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SleepyDog.h>
#include <Telekelud.h>
#include <SparkFunMAX31855k.h> 

#define MAXCS   1

byte localAddress = 0xCC;
byte destination = 0xAA;
long interval = 256000;
//long interval = 16000;
int dummy = 10;

SparkFunMAX31855k probe(MAXCS);
Telekelud kld(localAddress,destination,interval);

void setup() {
    kld.start();
    kld.configure();
    delay(4000); // Let IC stabilize or first readings will be garbage
    kld.setSenderBatt(analogRead(VBATPIN));
    kld.setTemp(readTemp());
    kld.sendMessage();
    delay(10000); // Let IC stabilize or first readings will be garbage

}

void loop() {
  kld.sleep();
  kld.setSenderBatt(analogRead(VBATPIN));
  delay(3000);
  kld.setTemp(readTemp());
  kld.sendMessage();
}

int readTemp(){
	// Make sure SPI for LoRa set to HIGH (inactive)
	pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);

    int temp = (int)(probe.readTempC() * 100); //23.25 => 2325
    if (isnan(temp)) return 9999;

    // Make sure SPI for MAX set to HIGH (inactive)
    pinMode(MAXCS, OUTPUT);
    digitalWrite(MAXCS, HIGH);
    return temp;
}
