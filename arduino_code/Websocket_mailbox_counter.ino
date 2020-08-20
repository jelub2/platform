
/*Sources: *******************
  Websocket:
  Esp8266_Websocket.ino
  Capatative touch sensor:
  https://how2electronics.com/arduino-ttp223-capacitive-touch-sensor-switch/
  Neopixel Ledstrip:
  NeoPixel Ring simple sketch (c) 2013 Shae Erisson
  Released under the GPLv3 license to match the rest of the
  Adafruit NeoPixel library
*****************************/

#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WebServer.h>

#define ctsPin 5 // Pin for capactitive touch sensor
#define PIN       4 //
#define NUMPIXELS 3 //
#define ledPin 2    // pin for the Builtin LED

const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0

boolean handshakeFailed = 0;
String data = "";
boolean   currState = false;
int   amnt = 0;
boolean   cim = false;

char path[] = "/123";   //identifier of this device

const char* ssid     = "********";
const char* password = "********";
char* host = "192.168.2.44";  //replace this ip address with the ip address of your Node.JS websocket server
const int espport = 3001;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

WebSocketClient webSocketClient;

// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(115200);
  pixels.begin(); // INITIALIZE NeoPixel strip object
  pinMode(ledPin, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(ctsPin, INPUT); // Initialize dht sensors

  delay(10);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(2, HIGH); //Somehow the HIGH and LOW on NODEMCU's LED_Builtin are by default the other way around. By using this line the Led is set to off.

  delay(1000);

  wsconnect();
}

void loop() {

  int ctsValue = digitalRead(ctsPin);
  int anlgValue = analogRead(analogInPin);

  //when a new package has been received
  if (anlgValue < 1000 && currState == false) {
    pixelsOn();
    currState = true;
    amnt = 1;
    cim = false;
    sendMessage(amnt);
  }

  // check if system has been closed before adding 1 to amnt
  if (cim == false && currState == true && anlgValue > 1000) {
    cim = true;
  }

  // when another package has been received before emptying
  if (anlgValue < 1000 && currState == true && cim == true) {
    amnt++;
    cim = false;
    Serial.println(amnt);
    sendMessage(amnt);
  }

  //when the user sets the system to default
  if (ctsValue == HIGH && currState == true) {
    pixelsOff();
    currState = false;
    amnt = 0;
    cim = false;
  }

}/*end of loop*/


//***************function definitions**********************************************************************************
void wsconnect() {
  // Connect to the websocket server
  if (client.connect(host, espport)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    delay(1000);

    if (handshakeFailed) {
      handshakeFailed = 0;
      ESP.restart();
    }
    handshakeFailed = 1;
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {

    Serial.println("Handshake failed.");
    delay(4000);

    if (handshakeFailed) {
      handshakeFailed = 0;
      ESP.restart();
    }
    handshakeFailed = 1;
  }
}

void sendMessage(int amnt) {
  if (client.connected()) {

    webSocketClient.getData(data);

    digitalWrite(2, LOW); //Visual feedback on sending to websocket server

    String amount = String(amnt);

    webSocketClient.sendData(amount);//send sensor data to websocket server

    data = "";
    delay(1000);
    digitalWrite(2, HIGH); // End of feedback
  } else {
  }
}

void pixelsOn() {
  for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...

    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(500); // Pause before next pass through loop

  }
}

void pixelsOff() {
  pixels.clear();
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.setPixelColor(1, pixels.Color(0, 0, 255));
  pixels.setPixelColor(2, pixels.Color(0, 0, 255));
  pixels.show();
  delay(500);

  for (int i = 2; i > -1; i--) { // For each pixel...

  pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  pixels.show();   // Send the updated pixel colors to the hardware.

  delay(500); // Pause before next pass through loop

  }
}
