// modified lora library by Sandeep Mistry for TTGO ESP32 Lora
// lora receiverCallBack 915Mhz with OLED

#include <ThingSpeak.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

  #define SCK     5
  #define MISO    19
  #define MOSI    27
  #define SS      18
  #define RST     14
  #define DI0     26
  #define BATPIN  35
  #define BAND    915E6
  #define SF      12
  #define TXP     20

  #define DEBUG

// set to DEBUG to activate Serial
#ifdef DEBUG
  #define DEBUG_WAIT           !Serial
  #define DEBUG_PRINT(x)       Serial.print (x)
  #define DEBUG_PRINT2(x,y)    Serial.print (x,y)
  #define DEBUG_PRINTDEC(x)    Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)     Serial.println (x)
  #define DEBUG_PRINTLN2(x,y)  Serial.println (x,y)
  #define DEBUG_START(x)       Serial.begin(x)
  #define DEBUG_ATTACH()       USBDevice.attach();
#else
  #define DEBUG_WAIT           false
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT2(x,y)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTLN2(x,y)
  #define DEBUG_START(x)
  #define DEBUG_ATTACH()
#endif

struct datpac
{
  float gas;
  float tm1;
  float tm2;
  float ph;
  float tds;
  float dis;
};

struct batpac
{
  float sender;
  float repeater;
};

  datpac _packet;
  batpac _batt;
  int _rssi;
  byte _msgCount;
  float _snr;
  byte _localAddress = 0xBB;
  long _lastReceiveTime, _lastupdate;

//replace default pin  OLED_SDA=4, OLED_SCL=15 with  OLED_SDA=21, OLED_SCL=22
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define LED_BUILTIN 2

char ssid[] = "anashandaru";   // your network SSID (name)
char pass[] = "anashandaru";   // your network password
unsigned long myChannelNumber = 849437;  // replace 0000000 with your channel number
const char * myWriteAPIKey = "1IK2044LXJ8TJZQP";   // replace XYZ with your channel write API Key

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, OLED_SDA, OLED_SCL); // OLED_SDA=4, OLED_SCL=15

WiFiClient client;

void setup() {
  _lastReceiveTime = millis();
  // Start LoRa
  SPI.begin(SCK, MISO, MOSI, SS);

  DEBUG_START(9600);
  while (DEBUG_WAIT)
  DEBUG_PRINTLN("cek print");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    DEBUG_PRINTLN("Starting LoRa failed!");
    while (1);
  }

  DEBUG_PRINTLN("LoRa init succeeded.");

  LoRa.setSpreadingFactor(SF);
  LoRa.setTxPower(TXP);

  LoRa.receive();

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

  updateUI();
  display.display();

  // Initialize ThingSpeak
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  connectWifi();
}

void loop() {
    if(millis() - _lastupdate > 1000){
      updateUI(); display.display();
      _lastupdate = millis();
    }
    if(!listen()) return;
    connectWifi();
    writeThingSpeak();
}

int bat2percent(int bat){
  // converts adc reading to battery percentage (1 - 100)
  // 3.2v(0%) to 4.2v(100%)
  // 551 to 651, 651 - 551 = 100
  int result = (bat-551);
  if(result>100) result = 100;
  return result;
}

bool listen(){
  int packetSize = LoRa.parsePacket();

  if(packetSize == 0) return 0;

  // read packet header bytes
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID

  // if the recipient isn't this device or broadcast,
  if (recipient != _localAddress && recipient != 0xFF) {
    DEBUG_PRINTLN("This message is not for me.");
    return 0;                             // skip rest of function
  }

  _lastReceiveTime = millis();

  readFloat(_packet.gas);
  readFloat(_packet.tm1);
  readFloat(_packet.tm2);
  readFloat(_packet.ph);
  readFloat(_packet.tds);
  readFloat(_packet.dis);
  readFloat(_batt.sender);
  readFloat(_batt.repeater);

  _msgCount = incomingMsgId;
  _rssi = LoRa.packetRssi();
  _snr = LoRa.packetSnr();

  DEBUG_PRINT("Received :");
  DEBUG_PRINT(_packet.gas); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tm1); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tm2); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.ph); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.tds); DEBUG_PRINT(",");
  DEBUG_PRINT(_packet.dis); DEBUG_PRINT(",");
  DEBUG_PRINT(_batt.sender); DEBUG_PRINT(",");
  DEBUG_PRINT(_batt.repeater); DEBUG_PRINT(",");
  DEBUG_PRINTLN("ends");

  return 1;
}

void readFloat(float& value){
  byte * p = (byte*) &value;
  for (int i = 0; i < 4; ++i)
  {
    *p++ = LoRa.read();
  }
}

void updateUI(){
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(11, 0, "S");
  display.drawString(53, 0, "R");
  display.drawString(95, 0, "G");

  display.drawProgressBar(12, 3, 30, 7, _batt.sender);
  display.drawProgressBar(54, 3, 30, 7, _batt.repeater);
  display.drawProgressBar(96, 3, 30, 7, getBattery());

  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(75, 12, String(_packet.tm1,2));
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(75, 12,"C");

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(100, 15, "Wifi");
  display.drawString(100, 25, String(WiFi.status() == WL_CONNECTED));
  display.drawString(4, 38, "RSSI " + String(_rssi));
  display.drawString(64, 38, "SNR " + String(_snr));
  display.drawString(4, 50, "Updated " + String((millis()-_lastReceiveTime)/1000) + "s ago");
}

int getBattery(){
  if((analogRead(BATPIN))>2350) return 100;
  return (analogRead(BATPIN)-1850)/6;
}

void connectWifi(){
    //return;//uncommnet this to log without wifi
    if(WiFi.status() != WL_CONNECTED){
    DEBUG_PRINT("Attempting to connect to SSID: ");
    DEBUG_PRINTLN("anashandaru");
    for(int i=0;i<3;i++){
      if(WiFi.status() == WL_CONNECTED) break;
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      DEBUG_PRINT("Number of attemp : ");
      DEBUG_PRINTLN(i+1);
      delay(5000);
    }

    DEBUG_PRINTLN("\nConnected.");
  }
}

void writeThingSpeak(){
  // set the fields with the values
  ThingSpeak.setField(1, _packet.gas);
  ThingSpeak.setField(2, _packet.tm1);
  ThingSpeak.setField(3, _packet.tm2);
  ThingSpeak.setField(4, _packet.ph);
  ThingSpeak.setField(5, _packet.tds);
  ThingSpeak.setField(6, _packet.dis);
  ThingSpeak.setField(7, _batt.sender);
  ThingSpeak.setField(8, _batt.repeater);

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}
