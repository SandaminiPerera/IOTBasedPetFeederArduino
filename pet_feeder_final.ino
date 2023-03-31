#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include<Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);

Servo servo;
LiquidCrystal_I2C lcd(0x27 , 16, 2);

#define WIFI_SSID "Galaxy A10s1329"
#define WIFI_PASS "sandy123"

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "SandaminiPerera"
#define MQTT_PASS "aio_HkvB12psmF2Tp9jZ0dnLmrF20Vee"

int CLOSE_ANGLE = 0;  // The closing angle of the servo motor arm
int OPEN_ANGLE = 90;  // The opening angle of the servo motor arm
int  hh, mm, ss;
int feed_hour = 0;
int feed_minute = 0;

//Set up MQTT and WiFi clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/IOTPetFeeder");
boolean feed = true; // condition for alarm

const char* host = "maker.ifttt.com";

const int trigPin = 12;
const int echoPin = 14;

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

void setup()
{
  Serial.begin(9600);
  

  timeClient.begin();
  Wire.begin(D2, D1);
  lcd.begin();
  
  //Connect to WiFi
  Serial.print("\n\nConnecting Wifi... ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Serial.println("OK!");

  //Subscribe to the onoff feed
  mqtt.subscribe(&onoff);
  servo.attach(D3);
  servo.write(CLOSE_ANGLE);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
}

void loop()
{
   MQTT_connect();
   timeClient.update();
   hh = timeClient.getHours();
   mm = timeClient.getMinutes();
   ss = timeClient.getSeconds();
   lcd.setCursor(0,0);
   lcd.print("Time:");
    if(hh>12)
  {
    hh=hh-12;
    lcd.print(hh);
    lcd.print(":");
    lcd.print(mm);
    lcd.print(":");
    lcd.print(ss);
    lcd.println(" PM  ");
  }
  else
  {
    lcd.print(hh);
    lcd.print(":");
    lcd.print(mm);
    lcd.print(":");
    lcd.print(ss);
    lcd.println(" AM  ");   
  }
   lcd.setCursor(0,1);
   lcd.print("Feed Time:");
   lcd.print(feed_hour);
   lcd.print(':');
   lcd.print(feed_minute   );

  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    
    if (subscription == &onoff)
    {
      //Print the new value to the serial monitor
      Serial.println((char*) onoff.lastread);
     
    if (!strcmp((char*) onoff.lastread, "ON"))
      {
        
        open_door();
        delay(1000);
        close_door();
       }
      if (!strcmp((char*) onoff.lastread, "Morning"))
      {
        feed_hour = 10;
        feed_minute = 30; 
      }
      if (!strcmp((char*) onoff.lastread, "Afternoon"))
      {
        feed_hour = 1;
        feed_minute = 15; 
      }
      if (!strcmp((char*) onoff.lastread, "Evening"))
      {
        feed_hour = 6;
        feed_minute = 30; 
      }
     }
   }
   if( hh == feed_hour && mm == feed_minute &&feed==true) //Comparing the current time with the Feed time

    { 
      open_door();
      delay(1000);
      close_door();
      feed= false; 
      }


WiFiClient client; 
      const int httpPort = 80;  
      if (!client.connect(host, httpPort)) 
      {  
         Serial.println("connection failed");  
         return;
      }
      
 {        // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
        // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  
  delay(1000);       

  
         
         if (distanceInch <= 2.00 && distanceInch > 0.00)
        {    
            String url = "/trigger/note/json/with/key/04ka1TR_re2AVEkxUMa05"; 
            Serial.print("Requesting URL: ");
            Serial.println(url);                 
            client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");   
            delay(50000); 
         }                     
          else
          {
            Serial.println("----------------------------------------------");            
          }
                
        
} 
}

void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  { 
       
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  
}

void open_door(){
  
  servo.write(OPEN_ANGLE);   // Send the command to the servo motor to open the trap door
}

void close_door(){
  
  servo.write(CLOSE_ANGLE);   // Send te command to the servo motor to close the trap door
}
