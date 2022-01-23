/*----------------------------------------------------------------------------------
 * Libraries
 * I have used an esp32 to display working of an nodemcu
 * So the code is nodemcu compatible
------------------------------------------------------------------------------------*/
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <time.h>
#include "ESPAsyncWebServer.h"


/*----------------------------------------------------------------------------------
 * pin where the load will be connected on nodemcu2
------------------------------------------------------------------------------------*/
#define load 2


/*----------------------------------------------------------------------------------
 * setting ssid and password for wlan to access ntp pool
------------------------------------------------------------------------------------*/
const char* SSID_STA = "YOUR_SSID";
const char* PASSWORD_STA = "YOUR_PSSWD";


/*----------------------------------------------------------------------------------
 * setting network credentials for WIFI_AP mode
------------------------------------------------------------------------------------*/
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";


/*----------------------------------------------------------------------------------
 *                         Time settings and configs
 *  asian pool used for lower latency               
 *  check your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)                      
------------------------------------------------------------------------------------*/
const char* NTP_SERVER = "asia.pool.ntp.org";
const char* TZ_INFO    = "IST-5:30";  

tm timeinfo;
/*------------------------------------------------------------------------
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


/*----------------------------------------------------------------------------------
 * initialising async web server on port 80 for http protocol
------------------------------------------------------------------------------------*/
AsyncWebServer server(80);


/*----------------------------------------------------------------------------------
 * returns nodemcu1's load status (HIGH/LOW)
------------------------------------------------------------------------------------*/
String read_esp32_load() {
  if(digitalRead(load)==1){
  return "true";  
  }
  else{
  return "false";
  }
}



void setup(){
/*----------------------------------------------------------------------------------
 * nodemcu1 connects to wlan and configuring ntp server settings
 * wifi mode is set to STA + AP
 * if setenv() is commented then ntp pool will return unix time
 * See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
 * for India Time Zone is "IST-5:30"
------------------------------------------------------------------------------------*/

  Serial.begin(115200);
  pinMode(load,OUTPUT);
  digitalWrite(load,LOW);
  count =0;
  WiFi.mode(WIFI_AP_STA);
  Serial.println("\n\nNTP Time Test\n");
  WiFi.begin(SSID_STA,PASSWORD_STA);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    if (++counter > 100) ESP.restart();
    Serial.print ( "." );
  }
  Serial.println("\n\nWiFi connected\n\n");

  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);


/*----------------------------------------------------------------------------------
 * waiting 10 seconds to get time packet from ntp pool
 * sets time locally and records millis()
------------------------------------------------------------------------------------*/
  if (getNTPtime(10)) {  // wait up to 10sec to sync
  } else {
    Serial.println("Time not set");
    ESP.restart();
  }
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();

  
/*----------------------------------------------------------------------------------
 * starts a server - client communication with nodemcu2
------------------------------------------------------------------------------------*/
  start_m2m();
  
}


 
void loop(){
/*----------------------------------------------------------------------------------
 * getTimeReducedTraffic(int sec) calls ntp pool once after the seconds 
   value saved in variable sec is over
 * showTime() displays time serially
 * as soon as loop executes for the first time count is set to 1
------------------------------------------------------------------------------------*/
  getTimeReducedTraffic(3600);
  if(count==0){
    getNTPtime(10);
   ++count;
  }
  showTime(timeinfo);


/*--------------------------------------------------------------------------------------------------------------
 * (timeinfo.tm_hour >= 22) || ( (timeinfo.tm_hour <6) || ( (timeinfo.tm_hour == 6) && (timeinfo.tm_sec == 6) ))
 * the above logic will give true if time is in range [10:00pm,6:00am]
 * communication to nodemcu1 is stopped and communication is established with wlan

 * the first while loop keeps nodemcu1 load on for 10:00 pm to 3:00am and calls ntp pool every hour
 * the second while loop keeps nodemcu1 load off for 3:00 am to 6:00am and calls ntp pool every hour
 * ntp pool is called to resync the esp timer as it fluctualtes slightly after an hour about 1-2 seconds
 * after 7 hours the timer will fluctuate drastically i.e in minutes
 
 * as soon as the if statement is triggered communication with nodemcu2 is stopped 
 * when timer is out of [10pm,6am] range commnication with nodemcu2 is re-established
 * Http server is started
--------------------------------------------------------------------------------------------------------------*/
    if((timeinfo.tm_hour >= 22) || ( (timeinfo.tm_hour <6) || ( (timeinfo.tm_hour == 6) && (timeinfo.tm_sec == 6) ))){
      WiFi.mode(WIFI_STA);
      
      while((timeinfo.tm_hour >= 22) || ( (timeinfo.tm_hour <3) || ( (timeinfo.tm_hour == 3) && (timeinfo.tm_sec == 0) ))){ 
        digitalWrite(load,HIGH); // time scheldule 10::00pm to 3:00am load 1 on
        getTimeReducedTraffic(3600);
        showTime(timeinfo);
        delay(1000);
      }
      while((timeinfo.tm_hour >= 3) || ( (timeinfo.tm_hour <6) || ( (timeinfo.tm_hour == 6) && (timeinfo.tm_sec == 0) ))){
       digitalWrite(load,LOW);  // time scheldule 10::00pm to 3:00am load 1 off
       getTimeReducedTraffic(3600);
       showTime(timeinfo);
       delay(1000);
        count =0;
      }
    } 
    
    if(count ==0){
      WiFi.mode(WIFI_AP_STA);
      start_m2m();
    }
  delay(1000);
}


void start_m2m (){

  
  Serial.println("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);


/*--------------------------------------------------------------------------------------------------------------
 * 3 pages are created 
 * when nodemcu2's load is HIGH it calls 192.168.4.1/nodemcu2_load_on which sets nodemcu1's load to LOW
   and vice versa
 * Page 192.168.4.1/nodemcu1_load_status when called responds with nodemcu1's load status
--------------------------------------------------------------------------------------------------------------*/
  server.on("/nodemcu1_load_status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", read_esp32_load().c_str());
  });
  server.on("/nodemcu2_load_on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(load,LOW);
    request->send_P(200,"text/html", "OK");
  });
  server.on("/nodemcu2_load_off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(load,HIGH);
    request->send_P(200,"text/html", "OK");
   
  });
  
  server.begin();
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
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
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
 * modifications are done according to tm struct
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
