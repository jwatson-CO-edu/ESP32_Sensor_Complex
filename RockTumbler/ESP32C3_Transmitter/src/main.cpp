////////// INIT ////////////////////////////////////////////////////////////////////////////////////
#pragma GCC diagnostic ignored "-Wwrite-strings"

///// Includes ////////////////////////////////////////////////////////////
#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include "ADS1X15.h"
#include "WifiKey.h" // SECRETS HEADER FILE

///// Defines /////////////////////////////////////////////////////////////
#define DEBUG_PRINT(x)   Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)

///// Constants ///////////////////////////////////////////////////////////
bool _SERIAL_COMM = true;

///// Globals /////////////////////////////////////////////////////////////
float Temperature = 6e10;
float Humidity    = 6e10;

///// Temp / Humid Sensor Setup ///////////////////////////////////////////
#define DHTTYPE DHT22   // DHT 22 (AM2302), AM2321
uint8_t DHTPin = 2; // DHT Sensor pin // Digital Pin 01
DHT     dht( DHTPin, DHTTYPE ); // Initialize DHT sensor.

///// Current Sensor Setup ////////////////////////////////////////////////
ADS1115 ADS( 0x48 );
float   adsVolt0 = 0.0;
float   adsVolt1 = 0.0;
float   adsVolt2 = 0.0;
float   adsVolt3 = 0.0;


///// WLAN + Server Setup /////////////////////////////////////////////////
const char* ssid     = _SSID; // Enter your WiFi SSID
const char* password = _PSWD; // Enter your WiFi password
WebServer   server(80); // Create a web server object that listens for HTTP requests on port 80

// Generate the HTML content to display
String SendHTML( String Temperaturestat, String Humiditystat ) {
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>ESP32 Temperature & Humidity</title>\n";
    ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
    ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
    ptr += "</style>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<div id=\"webpage\">\n";
    ptr += "<h1>ESP32 Temperature & Humidity</h1>\n";

    ptr += "<p>Temperature: ";
    ptr += Temperaturestat;
    ptr += " &deg;C</p>";
    ptr += "<p>Humidity: ";
    ptr += Humiditystat;
    ptr += "%</p>";

    ptr += "<p>Current: ";
    ptr += String( adsVolt0 ) + ", " + String( adsVolt1 ) + ", " + String( adsVolt2 ) + ", " + String( adsVolt3 ) + ", ";
    ptr += "</p>";

    ptr += "</div>\n";
    ptr += "</body>\n";
    ptr += "</html>\n";
    return ptr;
}


void fetch_temp_humid(){
    Temperature = dht.readTemperature(); // Get temperature value
    Humidity    = dht.readHumidity(); // -- Get humidity value
}


void fetch_ADS_readings(){
    ADS.setGain(0);
    int16_t val_0 = ADS.readADC(0);  
    int16_t val_1 = ADS.readADC(1);  
    int16_t val_2 = ADS.readADC(2);  
    int16_t val_3 = ADS.readADC(3);  
    float   f     = ADS.toVoltage(2);  // voltage factor

    adsVolt0 = f * val_0;
    adsVolt1 = f * val_1;
    adsVolt2 = f * val_2;
    adsVolt3 = f * val_3;
}


// Handle connection to the root URL
void handle_OnConnect() {
    
    fetch_temp_humid();
    fetch_ADS_readings();

    // Check if any reading failed and send "N/A" if so
    if( isnan( Temperature ) || isnan( Humidity ) ){
        if( _SERIAL_COMM )  DEBUG_PRINTLN("Failed to read from DHT sensor!");
        server.send(200, "text/html", SendHTML("N/A", "N/A"));
    } else {
        if( _SERIAL_COMM ){
            DEBUG_PRINT( "Temperature: " );
            DEBUG_PRINTLN( Temperature );
            DEBUG_PRINT( "Humidity: " );
            DEBUG_PRINTLN( Humidity );
        }
        
        server.send( 
            200, 
            "text/html", 
            SendHTML( String( Temperature ), String( Humidity ) ) ); // Send the HTML page with the values
    }
}

// Handle not found URL
void handle_NotFound() {
    server.send( 404, "text/plain", "Not found" );    
}



////////// BOOT ////////////////////////////////////////////////////////////////////////////////////

void setup() {

    if( _SERIAL_COMM )  DEBUG_PRINTLN( "Starting to Temp Sensor ..." );
    if( _SERIAL_COMM )  Serial.begin( 115200 ); // Start the Serial communication to send messages to the computer
    delay( 100 );
    dht.begin(); // Initialize the DHT sensor
    if( _SERIAL_COMM )  DEBUG_PRINTLN( "Temp Sensor Started!" );

    if( _SERIAL_COMM ){
        DEBUG_PRINTLN( "Starting to Current Sensor ..." );
        DEBUG_PRINTLN( __FILE__ );
        DEBUG_PRINT( "ADS1X15_LIB_VERSION: " );
        DEBUG_PRINTLN( ADS1X15_LIB_VERSION );
    }  
    Wire.begin();
    ADS.begin();
    if( _SERIAL_COMM )  DEBUG_PRINTLN( "Current Sensor Started!" );

    if( _SERIAL_COMM )  DEBUG_PRINTLN( "Connecting to WiFi ..." );
    WiFi.begin( ssid, password ); // Connect to Wi-Fi network
    // Wait until the device is connected to Wi-Fi
    size_t i = 0;
    while( WiFi.status() != WL_CONNECTED ){
        ++i; 
        delay( 1000 );
        if( _SERIAL_COMM )  DEBUG_PRINT(".");
        if( i%10 == 0 ){
            if( _SERIAL_COMM )  DEBUG_PRINT("<ATTEMPT CONNECT>");
            WiFi.begin( ssid, password );
        }
    }

    DEBUG_PRINTLN("\nWiFi connected!");
    DEBUG_PRINT("IP Address: ");
    DEBUG_PRINTLN(WiFi.localIP()); // Print the IP address

    server.on("/", handle_OnConnect); // Define handling function for root URL
    server.onNotFound(handle_NotFound); // Define handling function for 404 (Not Found)

    server.begin(); // Start the HTTP server
    DEBUG_PRINTLN("HTTP server started");
}


////////// MAIN ////////////////////////////////////////////////////////////////////////////////////

void loop(){
    server.handleClient(); // Handle client requests
}

