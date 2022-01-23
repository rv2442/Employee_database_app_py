/*----------------------------------------------------------------------------------
 * Libraries
------------------------------------------------------------------------------------*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <time.h>  

/*----------------------------------------------------------------------------------
 * pin where the load will be connected on nodemcu2
------------------------------------------------------------------------------------*/
#define load 2

/*----------------------------------------------------------------------------------
 * setting ssid and password for nodemcu2 to connect to nodemcu1
------------------------------------------------------------------------------------*/
const char* ssid_nodemcu1 = "ESP32-Access-Point";
const char* password_nodemcu1 = "123456789";

/*----------------------------------------------------------------------------------
 * setting ssid and password for wlan to access ntp pool
------------------------------------------------------------------------------------*/
const char* ssid_lan = "YOUR_SSID";
const char* password_lan = "YOUR_PSSWD";

/*----------------------------------------------------------------------------------
 * nodemcu1 IP address or domain name with URL path
------------------------------------------------------------------------------------*/
const char* server_nodemcu2_load_on = "http://192.168.4.1/nodemcu2_load_on";
const char* server_nodemcu2_load_off = "http://192.168.4.1/nodemcu2_load_off";
const char* server_nodemcu1_load_status = "http://192.168.4.1/nodemcu1_load_status";

/*----------------------------------------------------------------------------------
 *                         Time settings and configs
 *  asian pool used for lower latency               
 *  check your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)                      
------------------------------------------------------------------------------------*/
const char* NTP_SERVER = "asia.pool.ntp.org"; 
const char* TZ_INFO    = "IST-5:30"; 

tm timeinfo;
/*------------------------------------------------------------------------
 *  structure used in time.h
 * 
struct tm {
   int tm_sec;          seconds,  range 0 to 59          
   int tm_min;          minutes, range 0 to 59           
   int tm_hour;         hours, range 0 to 23             
   int tm_mday;         day of the month, range 1 to 31  
   int tm_mon;          month, range 0 to 11             
   int tm_year;         The number of years since 1900   
   int tm_wday;         day of the week, range 0 to 6    
   int tm_yday;         day in the year, range 0 to 365  
   int tm_isdst;        daylight saving time             
};
---------------------------------------------------------------------------*/
time_t now;


/*----------------------------------------------------------------------------------
 * declaring global variables
------------------------------------------------------------------------------------*/
long unsigned lastNTPtime;
unsigned long lastEntryTime;
int count;
String nodemcu1_load_status;



void setup() {
  Serial.begin(115200);
  pinMode(load,OUTPUT);
  digitalWrite(load,LOW);
  count=0
/*----------------------------------------------------------------------------------
 * nodemcu2 connects to wlan and configuring ntp server settings
 * if setenv() is commented then ntp pool will return unix time
 * See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
 * for India Time Zone is "IST-5:30"
------------------------------------------------------------------------------------*/
  Serial.println();
  WiFi.mode(WIFI_STA);
  connect_to_lan();
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);

/*----------------------------------------------------------------------------------
 * waiting 10 seconds to get time packet from ntp pool
 * sets time locally and records mills()
------------------------------------------------------------------------------------*/
  if (getNTPtime(10)) {
  } else {
    Serial.println("Time not set");
    ESP.restart();
  }
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();

/*----------------------------------------------------------------------------------
 * disconnecting from lan and connecting to nodemcu1 access point for communication
 * nodemcu1 -> server
 * nodemcu2 -> client
 * protocol -> http
------------------------------------------------------------------------------------*/
  WiFi.disconnect();
  connect_to_nodemcu1();
}

void loop() {
/*----------------------------------------------------------------------------------
 * getTimeReducedTraffic(int sec) calls ntp pool once after the seconds 
   value saved in variable sec is over
 * showTime() displays time serially
 * as soon as loop executes for the first time count is set to 1
------------------------------------------------------------------------------------*/
  getTimeReducedTraffic(7200);
  showTime(timeinfo);
  if(count ==0){
  ++count;  
  }

/*--------------------------------------------------------------------------------------------------------------
 * (timeinfo.tm_hour >= 22) || ( (timeinfo.tm_hour <6) || ( (timeinfo.tm_hour == 6) && (timeinfo.tm_sec == 6) ))
 * the above logic will give true if time is in range [10:00pm,6:00am]
 * communication to nodemcu1 is stopped and communication is established with wlan

 * the first while loop keeps nodemcu2 load off for 10:00 pm to 3:00am and calls ntp pool every hour
 * the second while loop keeps nodemcu2 load on for 3:00 am to 6:00am and calls ntp pool every hour
 * ntp pool is called to resync the esp timer as it fluctualtes slightly after an hour about 1-2 seconds
 * after 7 hours the timer will fluctuate drastically i.e in minutes


 * at the end of while loop 2 communication with wlan is disconnected and re-established with nodemcu1
--------------------------------------------------------------------------------------------------------------*/
  if((timeinfo.tm_hour >= 22) || ( (timeinfo.tm_hour <6) || ( (timeinfo.tm_hour == 6) && (timeinfo.tm_sec == 6) ))){
     WiFi.disconnect();
     connect_to_lan();
     ++count;
    while((timeinfo.tm_hour >= 22) || ( (timeinfo.tm_hour <3) || ( (timeinfo.tm_hour == 3) && (timeinfo.tm_sec == 0) ))){ 
      digitalWrite(load,LOW); // time scheldule 10:00pm to 3:00am load 2 off
      getTimeReducedTraffic(3600);
      showTime(timeinfo);
      delay(1000);
    }
    while((timeinfo.tm_hour >= 3) || ( (timeinfo.tm_hour <6) || ( (timeinfo.tm_hour == 6) && (timeinfo.tm_sec == 0) ))){
     digitalWrite(load,HIGH);  // time scheldule 10:00pm to 3:00am load 2 on
     getTimeReducedTraffic(3600);
     showTime(timeinfo);
     delay(1000);
      count =0;
    } 
    WiFi.disconnect();
    connect_to_nodemcu1();  
  }
/*-----------------------------------------------------------------------------------------------------
 * outside the range [10pm,6am] at every hour when time is range 59 min [55,59] seconds, communication
   with nodemcu1 will be stopped and re-established with wlan to sync time with ntp

 * ntp pool is called 5 times for redundancy 

 * when time synced, then communication is re-established with nodemcu1
------------------------------------------------------------------------------------------------------*/
  
  else if((timeinfo.tm_min == 59)&&(timeinfo.tm_sec >= 55)){
  WiFi.disconnect();
  connect_to_lan();
    for(int i=0; i<=5;i++){
      getTimeReducedTraffic(1);
      delay(1000);
    }
    WiFi.disconnect();
    connect_to_nodemcu1();
  }
/*----------------------------------------------------------------------------------
 * here nodemcu2 calls nodemcu1's hosted http page 
 * it is requesting nodemcu1's load status
 * when nodemcu1's load is HIGH then nodemcu2's load is set to LOW and vice versa
------------------------------------------------------------------------------------*/    
    if ((WiFi.status() == WL_CONNECTED)) {

      delay(1000);
      nodemcu1_load_status = "";
      nodemcu1_load_status = httpGETRequest(server_nodemcu1_load_status);

      
      if(nodemcu1_load_status == "true"){
        digitalWrite(load,LOW);
        httpGETRequest(server_nodemcu2_load_off);  
        
      }
      else if(nodemcu1_load_status == "false"){
        digitalWrite(load,HIGH);
        httpGETRequest(server_nodemcu2_load_on);
        
      }
      
      if(digitalRead(load) == 0){
        digitalWrite(load,LOW);
        httpGETRequest(server_nodemcu2_load_off);  
      
      }
      else{
        digitalWrite(load,HIGH);
        httpGETRequest(server_nodemcu2_load_on);
        
      }
      Serial.println("Esp32 load status: " + nodemcu1_load_status);
      Serial.println("My Load Status: " + String(digitalRead(load)));
      delay(1000); 
    }

}

/*----------------------------------------------------------------------------------
 * initialising client library
------------------------------------------------------------------------------------*/
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  
/*----------------------------------------------------------------------------------
 * IP address with path or Domain name with URL path 
------------------------------------------------------------------------------------*/
  http.begin(client, serverName);
  
/*----------------------------------------------------------------------------------
 * Send HTTP POST request
------------------------------------------------------------------------------------*/
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
/*----------------------------------------------------------------------------------
 * Freeing resources
------------------------------------------------------------------------------------*/
  http.end();

  return payload;
}

void connect_to_lan(){
  int counter=0;
  Serial.print("Connecting to ");
  Serial.println(ssid_lan);
  WiFi.begin(ssid_lan, password_lan);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    if (++counter > 100) ESP.restart();
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");  
  
}

void connect_to_nodemcu1(){
  
  Serial.print("Connecting to ");
  Serial.println(ssid_nodemcu1);
  WiFi.begin(ssid_nodemcu1, password_nodemcu1);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");  
}
/*--------------------------------------------------------------------------------------------------------
 * time() calls the ntp pool
 * locatime_r() resyncs time with local timer, where "now" has ntp time and saves it inside timeinfo struct
 * sec variables stores how much maximum time will be given for one ntp call
---------------------------------------------------------------------------------------------------------*/
bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      Serial.print(".");
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2022 - 1900)));
    if (timeinfo.tm_year <= (2022 - 1900)) return false;  // the NTP call was not successful
    Serial.print("now ");  Serial.println(now);
    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
    Serial.println(time_output);
    Serial.println();
  }
  return true;
}

/*----------------------------------------------------------------------------------
 * setting pointer to tm struct
 * if difference between last ntp call and current time is less than value in variable
   "sec" in seconds then local time is updated
 * else a new ntp call is made and synced with local timer
------------------------------------------------------------------------------------*/
void getTimeReducedTraffic(int sec) {
  tm *ptm;
  if ((millis() - lastEntryTime) < (1000 * sec)) {
    now = lastNTPtime + (int)(millis() - lastEntryTime) / 1000;
  } else {
    lastEntryTime = millis();
    lastNTPtime = time(&now);
    now = lastNTPtime;
    Serial.println("Get NTP time");
  }
  ptm = localtime(&now);
  timeinfo = *ptm;
}

/*----------------------------------------------------------------------------------
 * Prints time serially
------------------------------------------------------------------------------------*/
void showTime(tm localTime) {

  Serial.print(localTime.tm_mday);
  Serial.print('/');
  Serial.print(localTime.tm_mon + 1);
  Serial.print('/');
  Serial.print(localTime.tm_year - 100);
  Serial.print('-');
  Serial.print(localTime.tm_hour);
  Serial.print(':');
  Serial.print(localTime.tm_min);
  Serial.print(':');
  Serial.print(localTime.tm_sec);
  Serial.print(" Day of Week ");
  if (localTime.tm_wday == 0)   Serial.println(7);
  else Serial.println(localTime.tm_wday);
}
