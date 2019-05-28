#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SleepyDog.h>
#include <Telekelud.h>

byte localAddress = 0xAA;
byte destination = 0xBB;
long interval = 16000;
int dummy = 10;

Telekelud kld(localAddress,destination,interval);

void setup() {
  kld.start();
  kld.configure();
  kld.listenMode();
  delay(10000);
}

void loop() {
  kld.repeaterServicePS();
}
