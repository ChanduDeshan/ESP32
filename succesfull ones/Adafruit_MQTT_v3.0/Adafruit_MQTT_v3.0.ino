#include <SimpleDHT.h>                   // Data ---> D3 VCC ---> 3V3 GND ---> GND
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define LEDPIN 13
#define LED1 27

// WiFi parameters
#define WLAN_SSID       "test1"
#define WLAN_PASS       "88888888"
// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "chandudeshan"
#define AIO_KEY         "aio_zuro18Tc5t9wAZBJRP2Gmv1INwWQ" 
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Temperature1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature1");
Adafruit_MQTT_Publish Humidity1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity1");
Adafruit_MQTT_Subscribe brightness = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/brightness");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");


int pinDHT11 = 4;


SimpleDHT11 dht11(pinDHT11);
byte hum = 0;  //Stores humidity value
byte temp = 0; //Stores temperature value

void connect();

void setup() {
  Serial.begin(115200);
  Serial.println(F("Adafruit IO Example"));
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  pinMode(LEDPIN,OUTPUT);
  pinMode(LED1,OUTPUT);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&brightness);
  mqtt.subscribe(&onoffbutton);

  // connect to adafruit io
  connect();

}

uint32_t x=0;

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}

void loop() {
  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &brightness) {
      Serial.print(F("Got: "));
      String brightnessValue = (char *)brightness.lastread;
      Serial.println(brightnessValue);
      analogWrite(LEDPIN, brightnessValue.toInt());
      
    }
  

  

    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      String status = (char *)onoffbutton.lastread;
      Serial.println(status);
      if(status.equals("ON")){
        digitalWrite(LED1,HIGH);
        }
      else if(status.equals("OFF")){
        digitalWrite(LED1,LOW);
        }
    }
  }


  
  dht11.read(&temp, &hum, NULL);
  Serial.print((int)temp); Serial.print(" *C, "); 
  Serial.print((int)hum); Serial.println(" H");
  delay(5000);
   
   if (! Temperature1.publish(temp)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    } 
       if (! Humidity1.publish(hum)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    }
    else {
      Serial.println(F("Sent!"));
    }
}
