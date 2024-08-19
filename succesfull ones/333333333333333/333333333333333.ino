//library
#include <WiFi.h> 
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <PubSubClient.h> //MQTT library
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <cppQueue.h>
#include <TimeLib.h>
#include <DHT.h>
#include <Arduino_JSON.h>
#include "AsyncUDP.h"


#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
#define RELAY1 26
#define IMPLEMENTATION FIFO
#define OVERWRITE true
#define queue_size 2000   //______________________________________________size of the que 2000

DHT dht(DHTPIN, DHTTYPE);


char httpRequestData[50];
const char* serverName = "https://0764834343.000webhostapp.com/index.php";   //__________________________________________________________ httpp sever name 
String api_key = "Test8888";

typedef struct strRec {                  /// 
  String t;
  float temperature;
  float humidity;
  float battery;
} Rec;

cppQueue q(sizeof(Rec), 2000, IMPLEMENTATION, OVERWRITE);


uint32_t t;                        
int switch_1 = 0;

char udpData[50];


// ***WiFi network begins***
const char* ssid     = "test1";   // ssid
const char* password = "88888888"; // paasword 


//mqtt server begins
const char* mqtt_server = "broker.hivemq.com"; // mqtt server hivemq  // app

//temperature bus and sensor begins
const int oneWireBus = 5;     //________________________________________________________________________ DS18B20 is connected to 5
float ds1820_temp;
float humidity;
float battery;


//setting variable to store mqtt messages begins
char mqtt_msg[20];
char mqtt_msg2[20];


//initializing onewire bus // get data from temp 

OneWire oneWire(oneWireBus); //to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor

//initializing wifi client begins
WiFiClient espClient;   // esp client
PubSubClient client(espClient);

// udp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AsyncUDP udp;


void setup() {
  
  pinMode(RELAY1, OUTPUT);  // can conect more relay
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  OneWire oneWire(oneWireBus);
  sensors.begin();
  dht.begin();
  wifi_connect();
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    if(udp.listen(44444)) {                       // udp  port 
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
            packet.printf("Got %u bytes of data", packet.length());            
        });
    }
  client.setServer(mqtt_server, 1883);   // mqtt client
  timeClient.begin();
}

void loop() {
  wifi_connect();                           // wifi conect
  if (WiFi.status() == WL_CONNECTED) {
    setSwitchStatus();
  }
                                          // get data temp,hum and bat
  ds1820_temp   = get_temperature();
  humidity = get_humidity();
  battery = map(analogRead(33), 0.0f, 4095.0f, 0, 100);   // conect to 33,prasantj(0-100) 4095 stp siz
  t = now();

    sprintf(udpData,"Temperature value is %2.2f",ds1820_temp );
    udp.broadcast(udpData);
    delay(50);
    sprintf(udpData,"Humidity value is %2.2f",humidity);
    udp.broadcast(udpData);
    delay(50);    
    sprintf(udpData,"Battery level is %2.2f",battery );// ____________________________________________clear error minus 
    udp.broadcast(udpData);
    
  Rec rec;          // rec part
  rec.t = t;
  rec.temperature = ds1820_temp  ; // clear error  
  rec.humidity = humidity;
  rec.battery = battery ;  //clear error
  q.push(&rec);

  int len = q.getCount();
  Serial.print("Queue length is :10 ");  //queue length
  Serial.println(len);

  if (len % 10 == 0) {
    if (WiFi.status() == WL_CONNECTED) {      
      HTTPClient http;
      http.begin(serverName);
      for (int i = 0; i < len; i++) {
        q.peek(&rec);                      //mqtt
        mqtt_publish("Temperature value is %2.2f C",rec.temperature,"2020AE25/temperature");
        mqtt_publish("Humidity value is %2.2f %%",rec.humidity,"2020AE25/humidity");
        mqtt_publish("Battery level is %2.2f %%",rec.battery,"2020AE25/battery");//-----------------------------------------------------------------------rec.battery-100----------------10 correction

         //http ---------------------------------html link code -----------------------------------------------------------------------------------------------------
        HTTPClient http;
        http.begin(serverName);              
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");       
        sprintf(httpRequestData, "api_key=Test8888&temperature=%8.4f&humidity=%8.4f&battery=%8.4f",rec.temperature,rec.humidity,rec.battery);
        int httpResponseCode = http.POST(httpRequestData); 
        delay(100);
        Serial.println(httpRequestData);
        Serial.println(httpResponseCode);       
              
        if (httpResponseCode > 0) {
          q.pop(&rec);
        }
        else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        delay(50);
      }
      http.end();
    }
    Serial.println("WiFi Disconnected");
  }
  go_to_sleep();    // esp sleep 
}

void go_to_sleep() {
  Serial.println("Sleep mode activated ............");
  delay(50);
  esp_sleep_enable_timer_wakeup(1000000); //10 seconds
  esp_wifi_stop();
  esp_light_sleep_start();
  esp_wifi_start();
}

String httpGETRequest(const char* sserverName) {                //
  HTTPClient http;
  http.begin(sserverName);
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}

                      // relay 1 ///
String setSwitchStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    String payload = httpGETRequest("https://2020ae25.000webhostapp.com/test.php");
    JSONVar myObject = JSON.parse(payload);
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return "failed";
    }
    JSONVar keys = myObject.keys();
    switch_1 = int(myObject[keys[0]]);
   
    if (switch_1 == 1) {
      digitalWrite(RELAY1, HIGH);
      Serial.println("HIGH");
    }
    else if(switch_1 == 0) {
      digitalWrite(RELAY1, LOW);
      Serial.println("LOW");
    }
    
    return "success";
  }
  else {
    return "failed";
  }
}

void mqtt_publish(char msg[50], float value1, char path[50]) {
  sprintf(mqtt_msg, msg, value1);
  client.connect("clientId-1ynSqPiLB9");
  delay(100);
  client.publish(path, mqtt_msg);
}
     // get data  temp,hum,bat
float get_temperature() {
 sensors.requestTemperatures();
 return sensors.getTempCByIndex(0);
  
}

float get_humidity() {
  humidity = dht.readHumidity();
  return humidity;
}
                          // wifi connect
void wifi_connect() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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
                // time
String ntp_connect() {                         
  timeClient.setTimeOffset(19800); // GMT +5:30 = 19800
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getFormattedTime();
}
