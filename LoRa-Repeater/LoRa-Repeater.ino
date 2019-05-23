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

int outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xAA;     // address of this device
byte gateway = 0xBB;          // gateway to send to

void setup() {
  Serial.begin(9600);
  //while (!Serial)
  
  Serial.println("LoRa Repeater");
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
}

void loop() {

}

void sendMessage(int outgoing, byte destination, byte msgCount) {
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
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID

  int incoming;                         // payload of packet
  while (LoRa.available()) {            
    incoming <<= 8;
    incoming |= LoRa.read();            // add bytes one by one
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.print("Message: "); Serial.println(incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  // forward incoming message to destination recipient
  sendMessage(incoming, gateway, incomingMsgId);
  Serial.println("Sending " + String(incoming) + " to " + String(gateway,HEX) + String(incomingMsgId));
  LoRa.receive();                     // go back into receive mode
}
