// modified lora library by Sandeep Mistry for TTGO ESP32 Lora
// lora receiverCallBack 915Mhz with OLED


#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#define SCK     5    // GPIO5  -- lora SCK
#define MISO    19   // GPIO19 -- lora MISO
#define MOSI    27   // GPIO27 -- lora MOSI
#define SS      18   // GPIO18 -- lora CS
#define RST     14   // GPIO14/12 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    915E6

String outgoing, dstatus="Waiting Mesagge...";              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xAA;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

//replace default pin  OLED_SDA=4, OLED_SCL=15 with  OLED_SDA=21, OLED_SCL=22
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

#define LED_BUILTIN 2



// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=4, OLED_SCL=15

int RxDataRSSI = 0;

void setup() {
  // START aktivas Oled
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST , OUTPUT);
  digitalWrite(OLED_RST , LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST , HIGH); // while OLED is running, must set GPIO16 in high、

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  // clear the display
  display.clear();
  // aktivasi Oled END

  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Duplex with callback");
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);

  Serial.println("LoRa Receiver Callback");

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // register the receive callback
  LoRa.onReceive(onReceive);

  // put the radio into receive mode
  LoRa.receive();
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "HwThinker");
  display.display();
  delay(1000);
  display.clear();
}

void loop() {
//    if (millis() - lastSendTime > interval) {
//    String message = "HeLoRa!";   // send a message
//    sendMessage(message);
//    Serial.println("Sending " + message);
//    lastSendTime = millis();            // timestamp the message
//    interval = random(2000) + 1000;     // 2-3 seconds
//    LoRa.receive();                     // go back into receive mode
//  }
  printLCD(dstatus,0);
}

void printLCD(String message, int line){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, line*15, message);
  display.display();
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

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  dstatus = incoming + String(incomingMsgId);
  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
