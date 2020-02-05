#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <SPI.h>

#include <ArduinoJson.h>

//Set up WIFI credentials from your local network
const char* ssid = "######"; //SSID of local network
const char* password = "########"; //Password on network

//Set up Weather API credentials from api.openweathermap.org
String APIKEY = "##############################";
String CityID = "4930956"; //Boston, MA
int TimeZone = -5; //GMT +2


//Initiate variables
WiFiClient client;
char servername[]="api.openweathermap.org";
char clockservername[]="worldclockapi.com";
String weatherApiResult;
String clockResult;

boolean night = false;
int counter = 0;
String weatherDescription = "";
float Temperature;

extern unsigned char cloud[];
extern unsigned char thunder[];
extern unsigned char wind[];

//Pin assignment
static const uint8_t D5 = 14; //Maps digital pin 5 to ESP8285 pin 14 - SCK
static const uint8_t D6 = 12; //Maps digital pin 6 to ESP8285 pin 12 - MISO
static const uint8_t D7 = 13; //Maps digital pin 7 to ESP8285 pin 13 - MOSI
static const uint8_t D8 = 5; //Maps digital pin 8 to ESP8285 pin 15 - SS

//WiFi Initialization
void setup() {
  Serial.begin(115200);
  Serial.println("Start...");
  pinMode(D8,OUTPUT);
  digitalWrite(D8, HIGH); //Was SS before
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting connection...");
    delay(5000);
  }
  Serial.println("Connected");
}

//Start internal timer loop.
//Get new data every 30 minutes. Clock data and Weather data requests are offset by 12 seconds.
void loop() {
  if (counter == 1800)
  {
    counter = 0;
  } else if (counter == 3) {
    counter++;
    getClockData();
  } else if (counter == 15) {
    counter++;
    getWeatherData();
  } else {
    counter++;
    delay(1000);
    Serial.println(counter);
  }
}

//Client function to send GET request and receive data from weather api client.
void getWeatherData() {     
  Serial.println("Getting Weather Data");
  if (client.connect(servername,80)) { //starts client connection, checks for connection
    client.println("GET /data/2.5/forecast?id="+CityID+"&units=metric&cnt=1&APPID="+APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Connection failed"); //error message if no client connect
    Serial.println();
  }
  
  while (client.connected() && !client.available()) delay(1); //waits for data
    Serial.println("Waiting for data");
  
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from buffer
      weatherApiResult = weatherApiResult + c;
  }

  client.stop(); //stop client
  weatherApiResult.replace('[',' ');
  weatherApiResult.replace(']',' ');
  Serial.println(weatherApiResult);

  //Place data into an array.
  char jsonArray [weatherApiResult.length()+1];
  weatherApiResult.toCharArray(jsonArray,sizeof(jsonArray));
  jsonArray[weatherApiResult.length()+1] = '\0'; 

  //Parse JSON data
  StaticJsonDocument<1024> json_doc;
  auto error = deserializeJson(json_doc,jsonArray);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  String location = json_doc["city"]["name"];
  String temperature = json_doc["list"]["main"]["temp"];
  String weather = json_doc["list"]["weather"]["main"];
  String description = json_doc["list"]["weather"]["description"];
  String idString = json_doc["list"]["weather"]["id"];
  String timeS = json_doc["list"]["dt_txt"];

  Serial.println(location);
  Serial.println(weather);
  Serial.println(temperature);
  Serial.println(description);
  Serial.println(idString);

  //Transfer weather data to Arduino via SPI
  char c;
  digitalWrite(D8,LOW); //Open SPI connection
  SPI.transfer('@'); //Signals to Arduino that upcoming data is weather data.
  for (const char * p = weather.c_str() ; c = *p; p++) 
    { Serial.print(c);
      SPI.transfer(c);
    }
  SPI.transfer('!'); //Signal end of data transfer for weather data
  digitalWrite(D8,HIGH); //Close SPI connection
}


//Client function to send GET request and receive data from world clock api client.
void getClockData() {
  Serial.println("Getting Clock Data");
  HTTPClient http; //Declare an object of class HTTPClient

  http.begin("http://worldclockapi.com/api/json/est/now");
  int httpCode = http.GET();

  if (httpCode > 0) {
    clockResult = http.getString();
    Serial.println(clockResult);
  } 

  http.end();

  char jsonArray [clockResult.length()+1];
  clockResult.toCharArray(jsonArray,sizeof(jsonArray));
  jsonArray[clockResult.length()+1] = '\0'; 

  StaticJsonDocument<1024> json_doc;
  auto error = deserializeJson(json_doc,jsonArray);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  String dateTimeStamp = json_doc["currentDateTime"];
  boolean timeFlag = false;
  char c;
  digitalWrite(D8,LOW); //Open SPI connection
  SPI.transfer('%');    //Signals to Arduino that upcoming data is time data.

  //Iterate through dateTimeStamp, transferring only the hour and minutes.
  for (const char * p = dateTimeStamp.c_str() ; c = *p; p++) 
    { 
      if (c == '-') {
        timeFlag = false;
      }
      if (timeFlag == true) {
        Serial.print(c);
        SPI.transfer(c);
      }
      if (c == 'T') {
        timeFlag = true;
      } 
    }
  SPI.transfer('$');     //Signal end of data transfer for time data
  digitalWrite(D8,HIGH); //Close SPI connection

}
