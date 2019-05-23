// modified lora library by Sandeep Mistry for TTGO ESP32 Lora
// lora receiverCallBack 915Mhz with OLED

#include <ThingSpeak.h>
#include <WiFi.h>
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

int data;              // outgoing message
String dstatus="Waiting Message...";
byte localAddress = 0xBB;     // address of this device
byte destination = 0xAA;      // destination to send to
long lastSendTime = 0;        // 
int interval = 500;           // 

//replace default pin  OLED_SDA=4, OLED_SCL=15 with  OLED_SDA=21, OLED_SCL=22
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define LED_BUILTIN 2

char ssid[] = "anashandaru";   // your network SSID (name) 
char pass[] = "anashandaru";   // your network password
unsigned long myChannelNumber = 785978;  // replace 0000000 with your channel number
const char * myWriteAPIKey = "XHFH9125894EN04Y";   // replace XYZ with your channel write API Key

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=4, OLED_SCL=15

WiFiClient  client;

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
  //while (!Serial);
  Serial.println("LoRa 1ch-Gateway");
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // register the receive callback
  //LoRa.onReceive(LoRa.parsePacket());

  // put the radio into receive mode
  LoRa.receive();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "GEOFISIKA-UGM");
  display.display();
  delay(1000);
  display.clear();

  // Initialize ThingSpeak
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);
  connectWifi();
}

void loop() {
    onReceive(LoRa.parsePacket());
}

void printLCD(String message, int line){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, line*15, message);
  display.display();
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID

  int incoming = 0;                 // payload of packet

  while (LoRa.available()) {
    incoming <<= 8;                   // can't use readString() in callback, so
    incoming |= LoRa.read();      // add bytes one by one
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

  connectWifi();
  writeThingSpeak(incoming);
}

void connectWifi(){
    if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println("anashandaru");
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
}

void writeThingSpeak(int number){
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeField(myChannelNumber, 1, number, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}
