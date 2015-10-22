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
 */

#ifndef SNTPCLOCK_H
#define SNTPCLOCK_H

#include <arduino.h>
#include <Ticker.h>
#include "SNTPTime.h"

#define TICKER_CYCLE 1000

class SNTPClock
{
  public:
    typedef void (*callback_t)(void);
    callback_t m_callback;

  private:
    Ticker m_secondTicker;
    int    m_tickerCycle;

    String m_timeServerName;
    long   m_timezone;
    int    m_daylight;

    tm     m_localTimeStruct; // this is localtime
    ulong  m_secsSince1900;   // used in NTP time
    char   m_localTimeStr[20];
    char   m_localDateTimeStr[20];

    ulong  m_cbLocalTimeSec;

  public:
    SNTPClock();
    ~SNTPClock();

    int   begin(String timeServerName, long timezone, int daylight);

    void  secondTick();
    void  attachCb(tm * cbTime, callback_t callback);
    void  attachCb(ulong seconds, callback_t callback);
    void  detachCb();

    tm*   getTimeStruct();
    ulong getTimeSeconds();
    char* getTimeStr();
    char* getDateTimeStr();

    int   getSecond();
    int   getMinute();
    int   getHour();

  private:

    int   getDay();
    int   getMonth();
    int   getYear();

};

#endif // SNTPCLOCK_H

