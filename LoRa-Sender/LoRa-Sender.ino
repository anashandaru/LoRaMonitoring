#include <SPI.h>
#include <LoRa.h>

  //LoR32u4II 868MHz or 915MHz (black board)
  #define SCK     15
  #define MISO    14
  #define MOSI    16
  #define SS      8
  #define RST     4
  #define DI0     7
  #define BAND    915E6

int data = 52;                   // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xCC;     // address of this device
byte destination = 0xAA;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 5000;          // interval between sends

void setup() {
  Serial.begin(9600);
  //while (!Serial)
  
  Serial.println("LoRa End Node");
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa init succeeded.");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    sendMessage(data);
    Serial.print("Sending ");
    Serial.print(data);
    Serial.print(" ");
    Serial.println(msgCount);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 20000;     // 20-22 seconds
  }
}

void sendMessage(int outgoing) {
  byte buff[2];
  buff[0] = (outgoing >> 8) & 0xFF;
  buff[1] = outgoing & 0xFF;

  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(buff[0]);                  // add payload
  LoRa.write(buff[1]);                  // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;data++;                    // increment message ID
}
