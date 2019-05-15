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

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device
byte destination = 0xAA;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup() {
  Serial.begin(9600);
  while (!Serial)
  
  Serial.println("LoRa Sender with callback");
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  //LoRa.onReceive(onReceive);
  //LoRa.receive();
  Serial.println("LoRa init succeeded.");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    String message = "HeLoRa World!";   // send a message
    sendMessage(message);
    Serial.println("Sending " + message + " " + msgCount);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;     // 2-3 seconds
    //LoRa.receive();                     // go back into receive mode
  }
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

//void onReceive(int packetSize) {
//  if (packetSize == 0) return;          // if there's no packet, return
//
//  // read packet header bytes:
//  int recipient = LoRa.read();          // recipient address
//  byte sender = LoRa.read();            // sender address
//  byte incomingMsgId = LoRa.read();     // incoming msg ID
//  byte incomingLength = LoRa.read();    // incoming msg length
//
//  String incoming = "";                 // payload of packet
//
//  while (LoRa.available()) {            // can't use readString() in callback, so
//    incoming += (char)LoRa.read();      // add bytes one by one
//  }
//
//  if (incomingLength != incoming.length()) {   // check length for error
//    Serial.println("error: message length does not match length");
//    return;                             // skip rest of function
//  }
//
//  // if the recipient isn't this device or broadcast,
//  if (recipient != localAddress && recipient != 0xFF) {
//    Serial.println("This message is not for me.");
//    return;                             // skip rest of function
//  }
//
//  // if message is for this device, or broadcast, print details:
//  Serial.println("Received from: 0x" + String(sender, HEX));
//  Serial.println("Sent to: 0x" + String(recipient, HEX));
//  Serial.println("Message ID: " + String(incomingMsgId));
//  Serial.println("Message length: " + String(incomingLength));
//  Serial.println("Message: " + incoming);
//  Serial.println("RSSI: " + String(LoRa.packetRssi()));
//  Serial.println("Snr: " + String(LoRa.packetSnr()));
//  Serial.println();
//
//  // forward incoming message to destination recipient
//  sendMessage(incoming);
//  Serial.println("Sending " + incoming) + " to " + String(destination,HEX);
//  LoRa.receive();                     // go back into receive mode
//}
