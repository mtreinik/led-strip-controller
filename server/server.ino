/*
 * Web Server for controlling a color LED strip
 *
 * Circuit:
 * - WiFi shield attached
 * - LED data at pin 5 and clock at pin 6
 * 
 * Author: Mikko Reinikainen <mikko.reinikainen@iki.fi> 
 */

 
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <WiFi101.h>

#include "hex.h"
#include "credentials.h"

#define NUMPIXELS 240 // Number of LEDs in strip
#define CLOCKPIN 6
#define DATAPIN 5
#define TIMEOUT 1500

Adafruit_DotStar strip = Adafruit_DotStar(
  NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

int status = WL_IDLE_STATUS;
WiFiServer server(80);

#define MAX_LINE_LENGTH 2100
uint8_t currentLine[MAX_LINE_LENGTH+1];
char buf[MAX_LINE_LENGTH+1] = {};

uint32_t sprite[NUMPIXELS];
int spriteSize = 0;
uint32_t spriteBackground = 0x000000;
uint32_t spritePosition = 0;
int spriteSpeed = 0;
int spriteVisible = 0;

String ledStatus = "OK";

void setup() {
  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);
  
  Serial.begin(9600);      // initialize serial communication
  pinMode(9, OUTPUT);      // set the LED pin mode

  // check for the presence of the WiFi shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off
  
  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(wifiSSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(wifiSSID, wifiPassword);
    
    // wait 12 seconds for connection:
    delay(12000);
  }
  
  // start the web server on port 80
  server.begin();       
  
  printWiFiStatusToSerialPort();
}

void drawSprite() {
  if (spriteVisible) {
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, spriteBackground);
    }
    for (int i = 0; i < spriteSize; i++) {
      int spritePixelPosition = ((spritePosition >> 4) + i) % NUMPIXELS;
      strip.setPixelColor(spritePixelPosition, sprite[i]);
    }
    strip.show();
    // TODO use millis() to make position depend on actual time and speed
    spritePosition += spriteSpeed;
  }
}

void updateLeds() {
  drawSprite();
}

/**
 * Commands
 * a = set colors from an array
 *     format: a[number of pixels][r1][g1][b1][r2][g2][b2]...
 *     
 * s = set all pixels to a single color
 *     format: a[r][g][b]
 *     
 * c = clear all pixels
 *     format: c
 *     
 * m = draw a moving sprite
 *     format: m[background r][background g][background b][size of sprite][sprite position][sprite speed][r1][g1][b1][r2][g2][b2]...
 * 
 * p = set a single pixel
 *     format: p[position][r][g][b]
 */
void processLedCommand(uint8_t line[]) {
  if (line[5] != '?') {
    return;
  }
  spriteVisible = 0;
  int pos = 6;
  uint8_t command = line[pos++];
  
  switch (command) {
    case 'c':
      ledStatus = "OK: command c";
      strip.clear();
      for (int i = 0; i < NUMPIXELS; i++) {
        strip.setPixelColor(i, 0x000000);
      }
      strip.show();
      break;
    case 's':
      {
        ledStatus = "OK: command s";
        uint32_t color = getHexColor(pos, line);
        for (int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, color);
        }
        strip.show();
      }
      break;
    case 'p':
      {
        ledStatus = "OK: command p ";
        int pixelPosition = getHexByte(pos, line);
        pos += 2;
        uint32_t color = getHexColor(pos, line);
        for (int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(pixelPosition, color);
        }
        strip.show();
      }
      break;
    case 'a':
      {
        ledStatus = "OK: command a";
        uint8_t numPixels = getHexByte(pos, line);
        pos += 2;
        for (int i = 0; i < numPixels && i < NUMPIXELS; i++) {
          uint32_t color = getHexColor(pos, line);
          strip.setPixelColor(i, color);
          pos += 6;
        }
        for (int i = numPixels; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, 0x000000);
        }
        strip.show();
      }
      break;
    case 'm':
      ledStatus = "OK: command m";
      spriteBackground = getHexColor(pos, line);
      pos += 6;
      spriteSize = getHexByte(pos, line);
      pos += 2;
      spritePosition = getHexByte(pos, line) << 4;
      pos += 2;
      spriteSpeed = getHexByte(pos, line) - 128;
      pos += 2;
      for (int i = 0; i < spriteSize; i++) {
        uint32_t color = getHexColor(pos, line);
        sprite[i] = color;
        pos += 6;
      }
      spriteVisible = 1;
      break;
    default:
      ledStatus = "Unknown command";
      Serial.println("Unknown command");
      Serial.println(command);
  }  
}

void loop() {
  WiFiClient client = server.available();
  if (client) {                      
    Serial.println("new client");    
    if (client.connected()) {
      int bytesRead = 0;
      int timeoutMillis = millis() + TIMEOUT;
      bool foundNewline = false;
      bool timeout = false;
      while (client.connected() && bytesRead < MAX_LINE_LENGTH && !foundNewline && !timeout) {
        updateLeds();
        if (millis() > timeoutMillis) {
          timeout = true;
        }
        if (client.available()) {
          char c = client.read();

          Serial.print("read: ");
          Serial.println(c);
        
          if (c == '\n') {
            foundNewline = true;
            break;
         }
        
         currentLine[bytesRead++] = c;
        } else {
          Serial.print(".");
        }
      }

      if (timeout) {
        client.stop();
        Serial.println("client disconnected due to timeout");
        return;
      }
      client.flush();
      currentLine[bytesRead++] = '\0';

    
      memcpy(buf, currentLine, bytesRead);
      String printableCurrentLine = String(buf);    
      Serial.print("processing line: ");  
      Serial.println(printableCurrentLine);

      if (printableCurrentLine.substring(5, 15) == "favicon.ico") {
        Serial.print("skipping request for favicon.ico");
      } else {
        processLedCommand(currentLine);
      }
    
      // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
      // and a content-type so the client knows what's coming, then a blank line:
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Access-Control-Allow-Origin: *");
      client.println();

      // the content of the HTTP response follows the header:
      client.println("<html>");
      client.println(ledStatus);
      client.println("</html>");

      // The HTTP response ends with another blank line:
      client.println();

      
      // disconnect client
      client.stop();
      Serial.println("client disonnected");

      updateLeds();    
    } else {
      Serial.println("not connected");
    }
  } else {
    updateLeds();
  }
}

void printWiFiStatusToSerialPort() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP Address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  Serial.print("signal strength (RSSI):");
  long rssi = WiFi.RSSI();
  Serial.print(rssi);
  Serial.println(" dBm");
}

