/*
 * SNTP - ESP8266-specific SNTPClock library
 * Copyright (c) 2015 Peter Dobler. All rights reserved.
 * This file is part of the esp8266 core for Arduino environment.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * The library SNTP includes:
 * SNTPClock.cpp, SNTPClock.h
 * SNTPTime.cpp, SNTPTime.h
 *
 */

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "pass.h"

#include "SNTPTime.h"
#include "SNTPClock.h"

extern "C" {
#include <sntp.h>
}

//Externals in SNTPClock.cpp
extern SNTPClock Clock;

//CallBackFunction
void ClockCbFunction()
{
  Serial.printf("%s CbFunction\r\n", Clock.getTimeStr());
  tm cbLocalTimeStruct;
  memcpy((void*)&cbLocalTimeStruct, (const void*)Clock.getTimeStruct(), sizeof(tm));
  //add some time and recalculate cbLocalTimeStruct
  addTimeSeconds(&cbLocalTimeStruct, 300UL);
  Clock.attachCb(&cbLocalTimeStruct, (SNTPClock::callback_t)ClockCbFunction);
  // to stop it detach the function
  //Clock.detachCb();
}//ClockCbFunction

void setup() {
  Serial.begin(19200);
  delay(2000);
  Serial.setDebugOutput(false);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  sntp_init();
  sntp_setservername(0, (char*)"de.pool.ntp.org");
  sntp_setservername(1, (char*)"time.windows.com");
  sntp_setservername(2, (char*)"time.nist.gov");
  sntp_set_timezone(0);

  // Connect to WiFi network
  Serial.print(F("\r\nconnecting to:\r\n "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    if (++retry > 20)
    {
      WiFi.begin(ssid, pass);
      Serial.println();
      retry = 0;
    }
  }
  Serial.print(F("\r\nconnected to:\r\n "));
  Serial.print(WiFi.SSID());
  Serial.printf("\r\n signal %3i dBm\r\n", WiFi.RSSI());

  delay(1000);
  // true: we see a timestamp once an hour (GMT)
  Serial.setDebugOutput(false);
  Clock.begin("de.pool.ntp.org", 3600, 1);
  delay(1000);
  ClockCbFunction();
}

uint32 this_second = 0;
uint32 last_second = 0;
void loop() {
  // new second appeared?
  this_second = Clock.getSecond();
  if (this_second != last_second)
  {
    last_second = this_second;
    if ((this_second % 60) == 0)
    {
      tm t = *Clock.getTimeStruct();
      Serial.printf("%i-%02i-%02i %02i:%02i:%02i\r\n",
                    t.tm_year, t.tm_mon + MONTH_START, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec);
    }
  }
  delay(500);
}
