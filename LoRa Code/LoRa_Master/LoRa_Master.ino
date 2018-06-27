#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#include <SPI.h>
#include <LoRa.h>
// #include "SSD1306.h"
#include<Arduino.h>

SSD1306 display(0x3c, 4, 15);

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA â€” GPIO4
//OLED_SCL â€” GPIO15
//OLED_RST â€” GPIO16

// WIFI_LoRa_32 ports

// GPIO5 â€” SX1278â€™s SCK
// GPIO19 â€” SX1278â€™s MISO
// GPIO27 â€” SX1278â€™s MOSI
// GPIO18 â€” SX1278â€™s CS
// GPIO14 â€” SX1278â€™s RESET
// GPIO26 â€” SX1278â€™s IRQ(Interrupt Request)

#define SS 18
#define RST 14
#define DI0 26

// LoRa Settings 
#define BAND 915.00E6
#define spreadingFactor 9
// #define SignalBandwidth 62.5E3
#define SignalBandwidth 31.25E3
#define preambleLength 8
#define codingRateDenominator 8

// ADC? Battery voltage
// const uint8_t vbatPin = 34;
// float VBAT;  // battery voltage from ESP32 ADC readint counter = 0;

int counter = 0;

void setup() {
  pinMode(25,OUTPUT); //Send success, LED will bright 1 second
  
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);
  
  Serial.begin(115200);
  while (!Serial); //If just the the basic function, must connect to a computer

// Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(5,5,"LoRa Master");
  display.display();
  
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  Serial.println("LoRa Sender");

  /*     
  pinMode(vbatPin, INPUT);
  VBAT = (120.0/20.0) * (float)(analogRead(vbatPin)) / 1024.0; // LiPo battery voltage in volts
  Serial.println("Vbat = "); Serial.print(VBAT); Serial.println(" Volts");
*/

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  Serial.print("LoRa Spreading Factor: ");
  Serial.println(spreadingFactor);
  LoRa.setSpreadingFactor(spreadingFactor);
  
  Serial.print("LoRa Signal Bandwidth: ");
  Serial.println(SignalBandwidth);
  LoRa.setSignalBandwidth(SignalBandwidth);

  LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setPreambleLength(preambleLength);
  
  Serial.println("LoRa Initial OK!");
  display.drawString(5,20,"LoRa Initializing OK!");
  display.display();

  String RxString = "Initialise";
  delay(2000);
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 5, "Sending packet ");
  display.drawString(50, 30, String(counter));
  display.display();
  
  // send packet
  LoRa.beginPacket();
//  LoRa.print(RxString + " ");
  LoRa.print(counter);
  LoRa.endPacket();

  // delay(100);
  // Now see if we can get our Tx strength info back

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packets
    Serial.print("Received packet. ");
    
    display.clear();
    display.setFont(ArialMT_Plain_16);
    //display.drawString(3, 0, "Received packet ");
    //display.display();
    
    // read packet
    while (LoRa.available()) {
      String data = LoRa.readString();
      Serial.print(data);
      display.drawString(0,0, "Got " + data);
      display.display();
    }
    
    // print RSSI of packet
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.print(" with SNR ");
    Serial.println(LoRa.packetSnr());
    // display.drawString(0, 45, "RSSI: ");
    // display.drawString(50, 45, (String)LoRa.packetRssi());
    String RxString = (String)LoRa.packetRssi() + "dB (" + (String)LoRa.packetSnr() +"dB)";
    display.drawString(0, 22, "Rx stats:");  
    display.drawString(0, 45, RxString);  
    display.display();
  }
  
  counter++;
  digitalWrite(25, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(1000); // wait for a second
  digitalWrite(25, LOW); // turn the LED off by making the voltage LOW
  delay(1000); // wait for a second
  
// delay(3000);
}
