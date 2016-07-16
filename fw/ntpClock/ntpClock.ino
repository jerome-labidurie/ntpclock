/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <EEPROM.h>
#include "ECwire.h"

#define HOSTNAME "clock"

unsigned long pTimer = 0; /**< previous local time update time */
#define UPDT_TIMER 10000     /**< upated time every xx msc */
unsigned long pNtp   = 0; /**< previous NTP request time */
#define UPDT_NTP 3600000*2     /**< upated NTP every xx ms */
//#define UPDT_NTP 300000     /**< upated NTP every xx ms */

uint8_t brightness=0x5;  /**< global brightness */
uint8_t dts=1;           /**< local time from UTC (hours) */
#define MODE_CLOCK 0     /**< function mode : clock */
#define MODE_MSG 1       /**< function mode : loop on a message */
uint8_t displayMode = MODE_CLOCK; /**< current display function */
char currentMsg[256]; /**< displayed message in case of MODE_MSG */

unsigned int localPort = 2390;      /**< local port to listen for UDP packets */
#define NTP_SERVER_TRY 5   /**< max number of ntp try before switching to next server in ntpServerName */
uint8_t ntpServerIdx = 0;   /**< ntp server list current index */
#define NTP_SERVER_NB 3  /**< number of servers in ntpServerName */
const char* ntpServerName[] = /**< ntp server names */
    {"diskstation",
     "pool.ntp.org",
     "time.nist.gov"
     };

#define NTP_PACKET_SIZE 48 /**< NTP time stamp is in the first 48 bytes of the message */

byte packetBuffer[NTP_PACKET_SIZE]; /**< buffer to hold incoming and outgoing packets */

WiFiUDP udp; /**< A UDP instance to let us send and receive packets over UDP */
volatile unsigned long epoch = 0; /**< local time in unix format (sec since 1970/1/1) */

ESP8266WebServer server(80);

// brightness adaptation
struct ba {
  int minRead = 500;
  int maxRead = 1000;
  uint8_t minSet = 5;
  uint8_t maxSet = 128;
  bool adapt = true;
} ba;
#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)

void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println();
 ecInit();
 ecGBright(brightness);
 wChaine("boot0");

 // read config from eeprom
 EEPROM.begin(512);
 dts = EEPROM.read(0);
 Serial.print("UTC+"); Serial.println(dts);

 wChaine("boot1");
 // setup wifi
 WiFi.hostname (HOSTNAME);
 
  IPAddress myIP;
  uint8_t r = 0;
  do {
    r = wicoWifiConfig (1, HOSTNAME, &myIP);
    Serial.println(myIP);
    if (r == 0 ) {
      // AP has been created, start web server
      wChaine(HOSTNAME);
      wicoSetupWebServer ();
    }
  } while (!r);
  
 wChaine("boot2");
  // setup webserver
	server.on("/", handleRoot);
 server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println("HTTP server started");

 wChaine("boot3");
   // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(HOSTNAME, myIP)) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
    // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

// setup udp
 wChaine("boot4");
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  getNtpDate();
  pNtp = millis();
  wChaine("salut");
  dispHour();
  pTimer = millis();
  
}

void loop() {
  unsigned long cTimer = millis();
  
  if (displayMode == MODE_MSG) {
    wScroll (currentMsg);
    delay(500);
  } else {
    // MODE_CLOCK
    if (cTimer > pTimer+UPDT_TIMER) {
      epoch += (cTimer-pTimer)/1000;
      pTimer = cTimer;
      dispHour();
      if (ba.adapt == true) {
        //auto ambiant light adaptation
        // get current lightning value
        int sensorValue = analogRead(A0);
        // set new min/max read values
        ba.minRead = MIN(ba.minRead,sensorValue);
        ba.maxRead = MAX(ba.maxRead,sensorValue);
        uint8_t sensorSet = 1;
        // map read value to brightness
        sensorSet = map(sensorValue, ba.minRead, ba.maxRead, ba.minSet, ba.maxSet);
        ecIBright( sensorSet );
        Serial.print(sensorValue);
        Serial.print(":");
        Serial.println(sensorSet);
      }

    }
    if (cTimer > pNtp + UPDT_NTP) {
      pNtp = cTimer;
      getNtpDate();
      dispHour();
    }
  }
	server.handleClient();

}

uint8_t getNtpDate (void) {
  IPAddress timeServerIP; /**< NTP server address */
  uint8_t ntpServerTry = 0;
  char msg[255];
  
  int cb = 0;
  while (cb == 0) {
    ntpServerTry = 0;
    //get a random server from the pool
    WiFi.hostByName(ntpServerName[ntpServerIdx], timeServerIP);
    snprintf(msg, 255, "%d: %s %s", ntpServerIdx, ntpServerName[ntpServerIdx], timeServerIP.toString().c_str());
    Serial.println(msg);
    while ((cb == 0) && (ntpServerTry < NTP_SERVER_TRY)) {
      sendNTPpacket(timeServerIP); // send an NTP packet to a time server
      // wait to see if a reply is available
      delay(1000);
      cb = udp.parsePacket();
      Serial.println("no packet yet");
      ntpServerTry++;
    }
    if (ntpServerTry >= NTP_SERVER_TRY) {
      // server not responding, try next one
      ntpServerIdx = (ntpServerIdx + 1) % NTP_SERVER_NB;
    }
  }
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    Serial.print(epoch);
    Serial.print(" / ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    //set local time
    epoch += dts * 3600;
    // print Unix time:
    Serial.println(epoch);
    printHour();
}

void dispHour(void) {
  char c[6];
  snprintf (c, 6, "%02d-%02d",(epoch  % 86400L) / 3600,(epoch  % 3600) / 60);
  wChaine(c);
}

void printHour(void) {
      // print the hour, minute and second:
    Serial.print("The local time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

