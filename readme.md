# Readme

## Korte omschrijving van het project
Voor dit project heb ik mijn inspiratie gehaald uit de switchbot (https://www.youtube.com/watch?v=Q_3f6OC9p0s). De switchbot is een klein device dat via wi-fi of bluetooth buttons van bestaande apparaten aan en uit kan zetten.
Dit project is ook een universeel device dat meet of iets is geopend. Dit kan betekenen een deur, het beeldscherm van je laptop of bijvoorbeeld het klepje van de brievenbus. Hierdoor kan het worden toegepast voor veiligheids- of voor informatieve doeleinden.
Use Case:
Een docente op een middelbare school werkt vanwege coronamaatregelen thuis. Voor het nieuwe jaar zijn boeken besteld en deze worden geleverd in de postvak op de locatie van de middelbare school. Om onnodige bezoeken aan de school te voorkomen wil de docente graag weten of het pakket in haar postvak is gestopt.
### Gebruikte hardware:

-	NodeMCU
-	Capacative Touch Sensor
-	NeoPixel Ledstrip
-	Analog input Sensor

### Gebruikte software:
-	NodeJS server
-	Arduino IDE
-	MongoDB Database

## Installatie instructies
### NodeJS Server & MongoDB Database
Voor dit project wordt een NodeJS server gebruikt die via een websocket informatie ontvangt van het embedded device. Deze server ontvangt in dit geval een getal en slaat deze op in een MongoDB database. Dit is een NoSQL database.
#### Stap 1: Installeer NodeJS
Installeer Nodejs via https://nodejs.org/en/download/.
#### Stap 2: Maak een (gratis) MongoDB database aan
Via mongodb.com/cloud/atlas kan een database aangemaakt worden voor het project.
#### Stap 3: Download het project
Na het afronden van stap 1 en 2 gaan we aan de slag met het project. Download het project en verander op regel 91 van het bestand server.js het IP-adres van de pc of Raspberry PI waar de server draait.
Type node server.js in de terminal en de server gaat draaien.   

#### Info:
Om het wiel niet opnieuw uit te vinden is in de NodeJS server gebruik gemaakt van NPM packages. Dit zijn de volgende:
+ Express: Voor gemakkelijke routing en front-end templating. Wordt in dit project niet functioneel gebruikt, maar zou een toepassing kunnen zijn om van informatie te voorzien over het internet.
+ WebSocket: Een package om een websocket server op te starten en in te richten.
+ Mongoose: Mongoose vergemakkelijkt het doorsturen van een model naar de MongoDB database.


### NodeMCU
Nu de server draait gaan we verder met het embedded device. In dit project wordt gebruik gemaakt van een NodeMCU met geintegreerde Wi-Fi chip. Deze kan ook worden vervangen door een andere module met een wi-fi module. Verander dan ook de pin mapping in de arduino code.

#### Stap 1 Bevestig de Hardware aan de NodeMCU
+ Neopixel: D2 3v en Ground.
+ Analoge sensor: A0 en 3v
+ Capacative touch sensor: D1 3v en Ground.
Schema tekening
Bevestig de metalen plaatjes van het product op de gewenste plek. Eén op het vaste gedeelte van de postvak en één op de klep van de postvak. Vervolgens plaats je de button op de gewenste plek. Deze button is om de teller van het aantal openingen te resetten. Tot slot bevestig je de ledstrip op de gewenste plek.

#### Stap 2 installeer de arduino code op de NodeMCU
Download de arduino code van het project en open deze in Arduino IDE of een andere gewenste IDE. Verander de SSID en het wachtwoord in de code. Evenals het IP adres van de websocket server
Info.
Voor de Arduino code zijn verschillende libraries gebruikt. Dit zijn de volgende:
+ ESP8266WiFi: Voor het gebruik van Wi-Fi op de ESP8266 module.
+ WebSocketClient: Client afhandeling voor de websocket. Dit geeft de mogelijkheid om informatie te ontvangen en te zenden naar de websocket server.
+ Adafruit_NeoPixel: Deze library wordt gebruikt voor het aansturen van de ledstrip die verbonden is aan de NodeMCU.
A short example/tutorial
Video:

## Code documentatie
### Arduino Code:
In de arduino code benoemen we uitereaard eerst de gebruikte bronnen voor de code. Vervolgens gebruiken we de benodigde libaries en definieren we de pinnummers voor de hardware.

```C
/Sources:
  Websocket:
  Esp8266_Websocket.ino
  Capatative touch sensor:
  https://how2electronics.com/arduino-ttp223-capacitive-touch-sensor-switch/
  NeoPixel Ring simple sketch (c) 2013 Shae Erisson
  Released under the GPLv3 license to match the rest of the
  Adafruit NeoPixel library
/

#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include <Adafruit_NeoPixel.h>


#define ctsPin 5 // Pin for capactitive touch sensor
#define PIN       4 // Digital output Pin used for the Neopixel Ledstrip
#define NUMPIXELS 3 // Number of pixels on the Ledstrip
#define ledPin 2    // pin for the Builtin LED

const int analogInPin = A0;  // ESP8266 Analog Pin A0
```

Hierna stellen we een aantal variabelen in.

```C
boolean handshakeFailed = 0; // Checks handshake with the websocket server. 1 means approved 0 means failed.
String data = ""; // This string contains the received data from the websocket server.

boolean  currState = false; // turns to true if the leaver has been opened for the first time after the reset.
int   amnt = 0; // amount of times the leaver has been opened before reset.
boolean cim = false; // cim = closed in meantime. Checks if the leaver has been closed before adding to amnt.

char path[] = "/123";   //identifier of this device

const char ssid     = "";// change to network you're using for this project
const char password = "";// password of the used network.

char host = "192.168.2.44";  //replace this ip address with the ip address of your Node.JS websocket server
const int espport = 3001; // port of the websocket server
```
Na het instellen van de variablene voor dit project en het aanpassen van het netwerk's SSID, Password en IP-adres gaan we verder met de setup van het project. Om te beginnen een drietal functies vanuit de libraries en vervolgens de void setup functie.

```C
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //Neopixel library Ledstrip information

WebSocketClient webSocketClient; // WebSocketClient information

// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(115200); //Serial baud rate
  pixels.begin(); // Initialize NeoPixel strip object
  pinMode(ledPin, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(ctsPin, INPUT); // Initialize capacitive touch sensor

  delay(10); // Delay to give the device some time.

  //Connecting to a WiFi network
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

  delay(1000); // another extra time to don't rush the system

  wsconnect(); //connect to the websocket with the websocket() function
}
```

Wanneer de NodeMCU opstart wordt bovenstaande code in de setup functie doorlopen. Dit stelt de verbinding met de wi-fi in en de verbinding met de websocket. Ook wordt de hardware klaar gezet op gebruikt te worden in de volgende functie. De loop functie. Deze functie is voor het gebruik van het systeem. De loop functie herhaalt zich onbeperkt en bij elke loop bekijkt het een aantal waardes vanuit de hardware of vanuit de websocket.

```C
void loop() {

  int ctsValue = digitalRead(ctsPin); //reads the value of the capacitive touch sensor.
  int anlgValue = analogRead(analogInPin); // reads the value of the analog sensor.

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

}/end of loop/
```
In de loop functie worden afhankelijk van de status van het device functies aangeroepen. Deze worden hieronder gedefinieerd.

Om te beginnen wsconnect()
```C
//function definitions
//Connect to the websocket server. The function is called in the void setup()
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
```

Wanneer de brievenbus is geopend wordt het aantal keren van het openen van de brievenbus verzonden naar de websocket.

```C
void sendMessage(int amnt) {
  if (client.connected()) {

    webSocketClient.getData(data); // This function is not used in the code, but it retreives data from the websocket server.

    digitalWrite(2, LOW); //Visual feedback on the nodeMCU while sending to websocket server

    String amount = String(amnt); // change integer to String. This makes it possible to send it to the websocket server, since it doesn't accept the integer type.

    webSocketClient.sendData(amount); //send sensor data to websocket server

    data = ""; // set the data that has been received from the websocket server to an empty string
    delay(1000); // Some time to rest
    digitalWrite(2, HIGH); // End of feedback
  } else {
  }
}
```
Wanneer het systeem in gebruik is kan de gebruiker op locatie zien of het werkend is door middel van een ledstrip. In onderstaande functies wordt de status van de ledstrip gedefinieerd.


```C
//PixelsOn runs when a new package has been received
void pixelsOn() {
  for (int i = 0; i < NUMPIXELS; i++) { // loop trough each pixel

    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // set color of the pixel. In this case Red = 0, Green = 255(Maximum) and Blue = 0
    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(500); // Pause before next pass through loop

  }
}

// PixelsOff runs when the user touches the capacative touch sensor.
void pixelsOff() {
  pixels.clear(); // clear all colors of the pixels
  pixels.show(); // turn all leds off
  delay(500); // Some rest
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Set all pixels to blue
  pixels.setPixelColor(1, pixels.Color(0, 0, 255));
  pixels.setPixelColor(2, pixels.Color(0, 0, 255));
  pixels.show(); // Show te blue color
  delay(500); // Some rest

  for (int i = 2; i > -1; i--) { // Loop trough each pixels descending and turn them off

  pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  pixels.show();   // Send the updated pixel colors to the hardware.

  delay(500); // Pause before next pass through loop

  }
}
```

De volledige arduino code is terug te vinden in de map arduino_code.

## NodeMCU Server code
Om te beginnen definieren we alle gebruikte NPM-packages.

```Javascript

const express     = require('express');
const ejs         = require('ejs');
const app         = express();
const WebSocket   = require('ws');
const http        = require('http');
const mongoose    = require('mongoose')

require('dotenv').config(); // Makes sure we can use the values in the "secret" .env file
```
Als eerstvolgende halen we de url van de database op uit de .env file. Deze willen we niet publiek beschikbaar hebben en is daarom niet zichtbaar. Vervolgens gebruiken we een aantal statements uit de packages. De EJS voor de templates aan de front-end zijde en express voor het afhandelen van de routes.

```Javascript
const mongo_url = process.env.MONGO_URI // url value from the .env file to connect with the MongoDB database

app.set('view engine', 'ejs');
app.use(express.static(__dirname + '/public'));
```

Nu gaan we verbinding maken met de database. HDe package mongoose zorgt voor een gemakkelijke manier van verbinden met de MongoDB database. Via de functie mongoose.connect komt de verbinidng tot stand.

```Javascript

//Connect to mongoDB database
mongoose.connect(mongo_url, { useNewUrlParser: true, useUnifiedTopology: true })

// Check the connection to the database
var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function() {
  console.log("connection to db on")
});
```
Voor het opslaan op een gestructureerde manier in de NoSQL database maak je met mongoose.model een vast schema. In dit schema geef je aan welk type data de input is en kun je bijvoorbeeld ook aangeven of iets required is. In dit project is enkle het aantal pakketjes(numOfPackets) en een datestamp nodig. Met deze twee waarden is terug te vinden wanneer een pakketjes is geleverd. Of wanneer een deur open is geweest.

```Javascript
var collectionSchema = new mongoose.Schema(
{numOfPackets: String},
{timestamps: true}
)
var Collection = mongoose.model('Collection', collectionSchema)
```

Vervolgens maken we een http server en controler we het adres van de host. deze wordt gelogd in de terminal.

```Javascript
const server = http.createServer(app); //create a server

require('dns').lookup(require('os').hostname(), function (err, add, fam) {
  console.log('addr: '+add);
}) // address of the web socket server

```
Nu de NodeJS server draait gaan we verder met de websocket server.

```Javascript
//websocket setup
const s = new WebSocket.Server({ server });

//when browser sends get request, send html file to browser
app.get('/', function(req, res) {
res.render('pages/index'); // render index.js for usage in the browser. This file can be found in views > pages > index.js
});

//When a new connection is alive
s.on('connection',function(ws,req){

// when server receives messsage from client trigger function with argument message /
ws.on('message',function(message){

console.log("Received: "+message);

s.clients.forEach(function(client){ //broadcast incoming message to all clients (s.clients)
if(client!=ws && client.readyState ){ //except to the same client (ws) that sent this message
client.send("");
}
});

ws.send("From Server only to sender: "+ message); //send to client where message is from

//the saveData function saves the received value to the mongoDB database
saveData(message)

});

//When a connected device disconnects
ws.on('close', function(){
console.log("lost one client");
});

console.log("new client connected");
});
```
Wanneer een bericht wordt ontvangen in de websocket server wordt de saveData functie aangeroepen.  Deze functie zorgt ervoor dat de ontvangen waarde en de datum wordt opgeslagen in de database.

```Javascript

// Save received data to MongoDB database
function saveData(message){

// make a new collection. The Model of this collection is declared in the top of this code. In the lines below we use this model and fill it with the received data. The timestamp is automatically added and is not needed to be declared.
var collectionOne = new Collection({
  numOfPackets: message
  }
)
 collectionOne.save() // write the collection to the database.
}
```

Tot slot moet de server opgestart worden. In het gebruikte netwerk gebeurt dit op 192.168.2.44:3001. Dit moet worden aangepast aan het eigen netwerk.
```Javascript
server.listen(3001, '192.168.2.44');
```

## 	Bronnen

### Arduino
+ Websocket: Esp8266_Websocket.ino
+ Capatative touch sensor: https://how2electronics.com/arduino-ttp223-capacitive-touch-sensor-switch/
+ NeoPixel Ring simple sketch (c) 2013 Shae Erisson
Released under the GPLv3 license to match the rest of the
Adafruit NeoPixel library

### Nodejs
+ Websockets in NodeJS: https://github.com/websockets/ws
