#include <Adafruit_NeoPixel.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>

const char* ssid        = "commdat";
const char* password    = "0p3nm35h";
const int LEDS          = 3;
int rssi_max            = 0;
int rssi_min            = 100;
int rssi_previous       = 0;
int cal_timer           = 500;

#define PIN 4

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);

WiFiUDP Udp;

void setup() {
  
  //setup serial port
  Serial.begin(115200);
  Serial.println("Booting...");

  //test the led strip
  Serial.println("LED Test...");
  strip.begin();
  strip.setBrightness(150);
  strip_test(LEDS);

  //Connect to the wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Wifi Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//basic led strip test
void strip_test(int number_leds) {
  for (int i = 0; i < (number_leds); i++) {
    strip.setPixelColor(i, 255, 0, 0);
    strip.show();
    delay(200);
    strip.setPixelColor(i, 0, 255, 0);
    strip.show();
    delay(200);
    strip.setPixelColor(i, 0, 0, 255);
    strip.show();
    delay(200);
    strip.setPixelColor(i, 0, 0, 0);
    strip.show();
  }
}

//sends a udp packet and returns the rssi
int get_rssi() {
  int rssi = 0;
  
  //sends a packet before reading the rssi
  //I think the rssi function uses a recent packet to get signal, this keeps it updating faster
  Udp.beginPacket('255.255.255.255', '1234');
  Udp.write('test');
  Udp.endPacket();
  
  //get the strenght of the current network
  rssi = WiFi.RSSI();

  return rssi;
}

//Converts rssi from a neg number, runs the auto_rssi_minmax, reverses the direction, scales on 0-255 
int convert_rssi(int rssi_in) {
  int rssi_out = 0;

  rssi_in = abs(rssi_in);

  //only run the scaling if the jumper is installed
  if (cal_timer > 0) {
    Serial.println("Calibrating..."+String(cal_timer));
    rssi_minmax(rssi_in);
    cal_timer--;
  }
    
  if (rssi_min != rssi_max) {
    //map function to scale the input to 0-255
    rssi_out = map(rssi_in, rssi_min, rssi_max, 0, 255);
  }

  //make sure it doesn't scale past the bounds since we can turn scaling off
  if (rssi_in > rssi_max) {
    rssi_out = 255;
  } else if (rssi_min > rssi_in) {
    rssi_out = 0;
  }

  //reverse the range
  rssi_out = 255 - rssi_out;
  return (int) rssi_out;
    
}

//return the min and max given the current rssi
void rssi_minmax(int rssi_in) {
  int rssi_diff = 0;

  if (rssi_in > rssi_max) {
    rssi_max = rssi_in;
  }

  if (rssi_min > rssi_in) {
    rssi_min = rssi_in;
  }
  
}

void loop() {

  int rssi = 0;
  int rssi_in = 0;
  
  //get the strenght of the current network
  rssi_in = get_rssi();
  rssi = convert_rssi(rssi_in);

  //only update the strip if there's a change
  if (rssi != rssi_previous) {
    //strip.setBrightness(rssi);
    for (int i = 0; i < LEDS; i++) {
      strip.setPixelColor(i, 200, rssi, 0);
      
    }
    
    
    //set led 0 red while we calibrate
    if (cal_timer > 0) {
      strip.setPixelColor(0, 255, 0, 0);
      strip.setBrightness(100);
      strip.show();
    } else {
      strip.setBrightness(100);
       strip.show(); 
    }

    rssi_previous = rssi;
    /* Testing */
    Serial.println("Min/RSSI/Max//Convert " + String(rssi_min) + "/" +  String(abs(rssi_in)) + "/" + String(rssi_max) + "//" +  String(rssi));   
  }
  delay(150);
  
}

