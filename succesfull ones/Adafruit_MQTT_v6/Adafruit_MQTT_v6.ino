#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


#include <SimpleDHT.h>                   // Data ---> D3 VCC ---> 3V3 GND ---> GND
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "AsyncUDP.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
char udpData[50];

#include <esp_wifi.h>

#include <cppQueue.h>
#include <TimeLib.h>
#define IMPLEMENTATION FIFO
#define OVERWRITE true

#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 5;     // GPIO where the DS18B20 is connected to
float ds1820_temp;            // DS1820 temperature 

OneWire oneWire(oneWireBus); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor


#define LEDPIN 13
#define LED1 27

// WiFi parameters
#define WLAN_SSID       "test1"
#define WLAN_PASS       "88888888"

// udp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AsyncUDP udp;



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
Adafruit_MQTT_Publish Temperature2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature2");


int pinDHT11 = 4;


SimpleDHT11 dht11(pinDHT11);
byte hum = 0;  //Stores humidity value
byte temp = 0; //Stores temperature value

void connect();

typedef struct strRec {                  /// 
  String t;
  float temperature;
  float humidity;
  float battery;
} Rec;

cppQueue q(sizeof(Rec), 2000, IMPLEMENTATION, OVERWRITE);

uint32_t t;

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello Esp32 ");
  display.display(); 
 
  OneWire oneWire(oneWireBus);
  sensors.begin();
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
  if(udp.listen(1234)) {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length());
        });
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
  }

  
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      
      if(onoffbutton.lastread[0] == '1'){
        digitalWrite(LED1,HIGH);
        Serial.println("1");
      
      }
      else {
        digitalWrite(LED1,LOW);
        Serial.println("0");
        }
    }
  }

   sensors.requestTemperatures(); 
  ds1820_temp   = sensors.getTempCByIndex(0); 
  Serial.println(ds1820_temp);
  Temperature2.publish(ds1820_temp);
  
  delay(2000);
  
  dht11.read(&temp, &hum, NULL);
  Serial.print((int)temp); Serial.print(" *C, "); 
  Serial.print((int)hum); Serial.println(" H");

  Rec rec;          // rec part
  rec.t = t;
  rec.temperature = temp  ; // clear error  
  rec.humidity = hum;
   //clear error
  q.push(&rec);

 int len = q.getCount();
  Serial.print("Queue length is : ");  //queue length
  Serial.println(len);

  if (len % 2 == 0) {

  
   for (int i = 0; i < len; i++) {
    q.peek(&rec); 
   if (! Temperature1.publish(temp)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    } 
       if (! Humidity1.publish(hum)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    }
    else {
      Serial.println(F("Sent!"));
       q.pop(&rec);
    }
   }

  
  }

    delay(1000);
    //Send broadcast
    //udp.broadcast("Anyone here?");
    sprintf(udpData,"Temperature value is %2.2f",ds1820_temp );
    udp.broadcast(udpData);
    delay(50);
    sprintf(udpData,"Humidity value is %2.2f",hum);
    udp.broadcast(udpData);
    delay(50);    
    sprintf(udpData,"dht temp level is %2.2f",temp );// ____________________________________________clear error minus 
    udp.broadcast(udpData);

  // clear display
  display.clearDisplay();
  
  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(temp);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  
  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(hum);
  display.print(" %"); 
  
  display.display(); 
    
    
}




void go_to_sleep() {
  Serial.println("Sleep mode activated ............");
  delay(50);
  esp_sleep_enable_timer_wakeup(1000000); //10 seconds
  esp_wifi_stop();
  esp_light_sleep_start();
  esp_wifi_start();
  wifi_connect();
  
  
}
void wifi_connect() {
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID,  WLAN_PASS);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (count >= 20) {
      break;
    }
    count++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("WiFi not available");
  }

}
