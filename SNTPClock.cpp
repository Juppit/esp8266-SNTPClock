/*
 * SNTPClock - ESP8266-specific SNTPClock library
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
 * The library SNTPClock includes:
 * SNTPClock.cpp, SNTPClock.h
 * SNTPTime.cpp, SNTPTime.h
 *
 */

#include "SNTPClock.h"
#include <Ticker.h>

extern "C" {
#include <sntp.h>
}

//the one and only instance of SNTPClock
SNTPClock Clock;

void secondTicker() {
  Clock.secondTick();
}

SNTPClock::SNTPClock()
{
  m_timeServerName = "time.windows.com";
  m_timezone = 0;
  m_daylight = 0;
  m_localTimeStr[0] = 0;
  memset (&m_localTimeStruct, 0, sizeof(m_localTimeStruct));
  m_secsSince1900 = 0;
  m_callback = NULL;
  m_tickerCycle = TICKER_CYCLE;
  m_cbLocalTimeSec = 0;
}

SNTPClock::~SNTPClock() {
  m_secondTicker.detach();
}

void SNTPClock::attachCb(ulong seconds, callback_t callback)
{
  m_cbLocalTimeSec = m_secsSince1900 + seconds;
  m_callback = callback;
}

void SNTPClock::attachCb(tm * cbLocalTimeStruct, callback_t callback)
{
  // first remove timezone
  m_cbLocalTimeSec = mktime(cbLocalTimeStruct) - m_timezone;
  // then remove daylight -> secsSince1900
  m_cbLocalTimeSec = removeDayLight(m_cbLocalTimeSec);
  m_callback = callback;
}

void SNTPClock::detachCb()
{
  m_callback = NULL;
}

//**** this is called from ticker! ****
void SNTPClock::secondTick()
{
  // set a new cycle length, if any
  m_secondTicker.attach_ms(m_tickerCycle, secondTicker);

  m_secsSince1900 = sntp_get_current_timestamp();
  m_localTimeStruct = *localtime(m_secsSince1900 + DIFF1900TO1970, m_timezone, m_daylight);

  // check for callback
  if (m_callback != NULL)
    if (m_secsSince1900 + DIFF1900TO1970 >= m_cbLocalTimeSec)
    {
      m_callback();
    }
}

int SNTPClock::begin(String timeServerName = "time.windows.com", long timezone = 0, int daylight = 0)
{
  m_timeServerName = timeServerName;
  m_timezone = timezone;
  m_daylight = daylight;
  sntp_setservername(0, (char*)timeServerName.c_str());

  // we have to wait for sntp
  while (!m_secsSince1900)
  {
    m_secsSince1900 = sntp_get_current_timestamp();
    delay(200);
  }
  delay(1000);

  m_secondTicker.attach_ms(m_tickerCycle, secondTicker);
  return m_secsSince1900 + DIFF1900TO1970;
}

tm* SNTPClock::getTimeStruct() {
  return &m_localTimeStruct;
}

ulong SNTPClock::getTimeSeconds() {
  return m_secsSince1900;
}

char* SNTPClock::getTimeStr()
{
  sprintf(m_localTimeStr, "%02i:%02i:%02i",
          m_localTimeStruct.tm_hour, m_localTimeStruct.tm_min, m_localTimeStruct.tm_sec);
  return m_localTimeStr;
}

char* SNTPClock::getDateTimeStr()
{
  sprintf(m_localDateTimeStr, "%04i-%02i-%02i %02i:%02i:%02i",
          m_localTimeStruct.tm_year, m_localTimeStruct.tm_mon + MONTH_START, m_localTimeStruct.tm_mday,
          m_localTimeStruct.tm_hour, m_localTimeStruct.tm_min, m_localTimeStruct.tm_sec);
  return m_localDateTimeStr;
}

int SNTPClock::getSecond() {
  return m_localTimeStruct.tm_sec;
}

int SNTPClock::getMinute() {
  return m_localTimeStruct.tm_min;
}

int SNTPClock::getHour() {
  return m_localTimeStruct.tm_hour;
}

int SNTPClock::getDay() {
  return m_localTimeStruct.tm_mday;
}

int SNTPClock::getMonth() {
  return m_localTimeStruct.tm_mon/*+MONTH_START*/;
}

int SNTPClock::getYear() {
  return m_localTimeStruct.tm_year;
}

