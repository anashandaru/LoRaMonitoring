#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SleepyDog.h>
#include <Telekelud.h>

byte localAddress = 0xCC;
byte destination = 0xAA;
long interval = 24000;
int dummy = 10;

Telekelud kld(localAddress,destination,interval);

void setup() {
    kld.start();
    kld.configure();
    kld.setTemp(dummy);
    kld.sendMessage();
    delay(10000);
}

void loop() {
  kld.senderServicePS();
}
