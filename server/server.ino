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
#include "color.h"
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

bool spriteMode = false;
uint32_t sprite[NUMPIXELS];
int spriteSize = 0;
uint32_t spriteBackground = 0x000000;
uint32_t spritePosition = 0;
int spriteSpeed = 0;
unsigned long millisBefore = millis();
int SUBPIXEL_SHIFT = 8;
int SUBPIXEL_MASK = 0xff;

#define FIRE_FUEL_AMOUNT_MULTIPLIER 64
#define FIRE_FUEL_PROBABILITY_DIVISOR 16
#define FIRE_BLACK 0x000000
#define FIRE_COLD 0x400000
#define FIRE_HOT 0xff7000
bool fireMode = false;
int fireFuelAmount = 0;
int fireFuelProbability = 0;
int fireDampingAmount = 0;
uint32_t fuel[NUMPIXELS] = { 0 };
uint32_t nextFuel[NUMPIXELS] = { 0 };
uint32_t fire[NUMPIXELS] = { 0 };
uint32_t nextFire[NUMPIXELS] = { 0 };

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

    initializeGammaTables();    
  }
  
  // start the web server on port 80
  server.begin();       
  
  printWiFiStatusToSerialPort();
}

void updateLeds() {
  unsigned long millisNow = millis();
  if (fireMode) {
    // spread previous fuel
    for (int i = 1; i < NUMPIXELS - 1; i++) {
      nextFuel[i] = (fuel[i - 1] + fuel[i] * 2 + fuel[i + 1]) / 4;
    }
    nextFuel[0] = (fuel[NUMPIXELS - 1] + fuel[0] * 2 + fuel[1]) / 4;
    nextFuel[NUMPIXELS-1] = (fuel[NUMPIXELS - 2] + fuel[NUMPIXELS - 1] * 2 + fuel[0]) / 4;
    memcpy(fuel, nextFuel, NUMPIXELS * sizeof(uint32_t));

    unsigned long delta = millisNow - millisBefore;

    // add fuel to random place
    if ((rand() % 256) < fireFuelProbability * delta / FIRE_FUEL_PROBABILITY_DIVISOR) {
      uint32_t fuelToAdd = FIRE_FUEL_AMOUNT_MULTIPLIER * fireFuelAmount * delta;
      fuel[rand() % NUMPIXELS] += fuelToAdd;
    }

    // make random flames from fuel
    for (int i = 0; i < NUMPIXELS; i++) {
      if (fuel[i] > 0) {
        int maxFuel = min(255, fuel[i]);

        int useFuel = maxFuel > 0 ? (rand() % maxFuel) : 0;
        //int flame = max(255, fire[i]);
        //int flame = min(255, fire[i]);
        nextFire[i] =         
          ((fire[i-1] + fire[i] + fire[i+1]) * fireDampingAmount / 3 
          + useFuel * (255 - fireDampingAmount)) / 255;
        int flame = max(0, min(255, nextFire[i]));
        uint32_t color = flame < 128 ? 
          mixColors(FIRE_BLACK, FIRE_COLD, 127 - flame, flame, 8) 
          : mixColors(FIRE_COLD, FIRE_HOT, 255 - flame, flame - 128, 8);
        strip.setPixelColor(i, color);
        
        fuel[i] -= useFuel;       
      } else {
        strip.setPixelColor(i, 0x000000);
      }
    }
    memcpy(fire, nextFire, NUMPIXELS * sizeof(uint32_t));

    strip.show();
  } else if (spriteMode) {
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, spriteBackground);
    }    
    int spritePixelPosition = spritePosition >> SUBPIXEL_SHIFT;
    int spriteSubpixelPosition = spritePosition & SUBPIXEL_MASK;
    int spriteSubpixelPositionRest = SUBPIXEL_MASK + 1 - spriteSubpixelPosition;
    
    uint32_t color = mixColors(spriteBackground, sprite[0], spriteSubpixelPosition, spriteSubpixelPositionRest, SUBPIXEL_SHIFT);
    strip.setPixelColor(spritePixelPosition % NUMPIXELS, color);

    for (int i = 1; i < spriteSize; i++) {
      color = mixColors(sprite[i-1], sprite[i], spriteSubpixelPosition, spriteSubpixelPositionRest, SUBPIXEL_SHIFT);
      strip.setPixelColor((spritePixelPosition + i) % NUMPIXELS, color);
    }

    color = mixColors(sprite[spriteSize-1], spriteBackground, spriteSubpixelPosition, spriteSubpixelPositionRest, SUBPIXEL_SHIFT);
    strip.setPixelColor((spritePixelPosition+ spriteSize) % NUMPIXELS, color);
    
    strip.show();
    spritePosition += spriteSpeed * (millisNow - millisBefore);
  }
  millisBefore = millisNow;
}

/**
 * Commands
 * c = clear all pixels
 *     format: c
 *     
 * s = set all pixels to a single color
 *     format: a[r][g][b]
 *     
 * p = set a single pixel
 *     format: p[position][r][g][b]
 *     
 * a = set colors from an array
 *     format: a[number of pixels][r1][g1][b1][r2][g2][b2]...
 *     
 * m = draw a moving sprite
 *     format: m[background r][background g][background b][size of sprite][sprite position][sprite speed][r1][g1][b1][r2][g2][b2]...
 * 
 * g = draw a gradiant
 *     format: g[start position][end position][r1][g1][b1][r2][g2][b2]
 *     
 * f = animate flames
 *     format: f[fuel amount][fuel probability][damping amount]
 */
void processLedCommand(uint8_t line[]) {
  if (line[5] != '?') {
    return;
  }
  spriteMode = false;
  fireMode = false;
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
        int pixelPosition = getHexByte( pos, line);
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
        uint8_t arraySize = getHexByte(pos, line);
        for (int i = 0; i < arraySize && i < NUMPIXELS; i++) {
          uint32_t color = getHexColor(pos, line);
          strip.setPixelColor(i, color);          
        }
        for (int i = arraySize; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, 0x000000);
        }
        strip.show();
      }
      break;
    case 'm':
      ledStatus = "OK: command m";
      spriteBackground = getHexColor(pos, line);
      spriteSize = getHexByte(pos, line);
      spritePosition = getHexByte(pos, line) << SUBPIXEL_SHIFT;
      spriteSpeed = getHexByte(pos, line) - 128;
      for (int i = 0; i < spriteSize; i++) {
        uint32_t color = getHexColor(pos, line);
        sprite[i] = color;
      }
      spriteMode = true;
      break;
    case 'g':
      {
        ledStatus = "OK: command g";
        int startPosition = getHexByte(pos, line);
        int endPosition = getHexByte(pos, line);
        int gradientLength = endPosition - startPosition + 1;
        
        uint32_t startColor = getHexColor(pos, line);
        uint32_t endColor = getHexColor(pos, line);

        for (int i = 0; i < gradientLength; i++) {
          int endColorWeight = i * 255 / (gradientLength - 1);
          int startColorWeight = 255 - endColorWeight;
          uint32_t color = mixColors(startColor, endColor, startColorWeight, endColorWeight, 8);
          strip.setPixelColor((startPosition + i) % NUMPIXELS, color);
        }
        strip.show();
      }
      break;
    case 'f':
      ledStatus = "OK: command f";
      fireFuelAmount = getHexByte(pos, line);
      fireFuelProbability = getHexByte(pos, line);
      fireDampingAmount = getHexByte(pos, line);
      for (int i = 0; i < NUMPIXELS; i++) {
        fire[i] = 0;
        fuel[i] = 0;
      }
      fireMode = true;
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

