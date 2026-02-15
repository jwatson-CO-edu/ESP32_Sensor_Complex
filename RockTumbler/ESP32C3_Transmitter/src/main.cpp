#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include "WifiKey.h" // SECRETS HEADER FILE

#define DHTTYPE DHT22   // DHT 22 (AM2302), AM2321

/* Put your SSID & Password */
const char* ssid     = _SSID; // Enter your WiFi SSID
const char* password = _PSWD; // Enter your WiFi password
WebServer server(80); // Create a web server object that listens for HTTP requests on port 80

// DHT Sensor pin
uint8_t DHTPin = 1; // Digital Pin 01

// Initialize DHT sensor.
DHT dht( DHTPin, DHTTYPE );


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}