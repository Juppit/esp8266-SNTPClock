/*
 * SNTPTime.cpp - ESP8266-specific SNTPClock library
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
 * The implementation of localtime is based on:
 * http://www.mikrocontroller.net/attachment/highlight/8391
 *
 */

#include "SNTPTime.h"
#include "SNTPClock.h"

//Externals in SNTPClock.cpp
extern SNTPClock Clock;

// set locals...
static ulong confTimeZone;
static int confDayLight;

void configTime(ulong timezone, int daylight)
{
  confTimeZone = timezone;
  confDayLight = daylight;
}

// seconds since 1970
time_t mktime(tm * t)
{
  // system_mktime expects month in range 1..12
  return DIFF1900TO1970 + system_mktime(t->tm_year, t->tm_mon + MONTH_START, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

tm * mkTmStruct(ulong secsSince1900)
{
  return localtime(secsSince1900, confTimeZone, confDayLight);
}

time_t time(time_t * t)
{
  time_t seconds = Clock.getTimeSeconds();
  if (t)
    *t = seconds;
  return seconds ;
}

// The gettimeofday() function is marked obsolescent.
// Applications should use the clock_gettime() function instead
// see: http://pubs.opengroup.org/onlinepubs/9699919799/
int gettimeofday(void * tp, void * tzp)
{
  timeval tv;
  tv.tv_sec  = millis() / 1000;
  tv.tv_usec = micros() % 1000000L;
  ((timeval*)tp)->tv_sec  = tv.tv_sec;
  ((timeval*)tp)->tv_usec = tv.tv_usec;
  return 0;
}

int clock_gettime(clockid_t clock_id, struct timespec * tp)
{
  timeval tv;
  tv.tv_sec  = millis() / 1000;
  tv.tv_usec = micros() % 1000000L;
  ((timeval*)tp)->tv_sec  = tv.tv_sec;
  ((timeval*)tp)->tv_usec = tv.tv_usec;
  return 0;
}

tm * gmtime(const time_t * secsSince1900)
{
  return localtime(*secsSince1900, 0, 0);
}

char * ctime(const time_t secsSince1900)
{
  static char ctimeStr[27]; // with "\r\n"
  tm t;
  memset((void*)&t, 0, sizeof(tm));
  t = *localtime(secsSince1900, confTimeZone, confDayLight);
  sprintf(ctimeStr, "%3s %3s %2d %02d:%02d:%02d %d",
          wday_name[t.tm_wday], mon_name[t.tm_mon], t.tm_mday,
          t.tm_hour, t.tm_min, t.tm_sec, t.tm_year);
  return ctimeStr;
}

char * asctime(const tm * t)
{
  static char timeStr[27]; // with "\r\n"
  sprintf(timeStr, "%3s %3s %2d %02d:%02d:%02d %d",
          wday_name[t->tm_wday], mon_name[t->tm_mon], t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec, t->tm_year);
  return timeStr;
}

tm* splittime(ulong secsSince1900)
{
  return localtime(secsSince1900, 0, 0);
}

void addTimeSeconds(tm * t, ulong sec1)
{
  ulong sec2 = mktime(t);
  *t = *splittime(sec1 + sec2);
}

void addTimeStruct(tm * t1, tm * t2)
{
  ulong sec1 = mktime(t1);
  ulong sec2 = mktime(t2);
  *t1 = *splittime(sec1 + sec2);
}

int compareTime(tm * t1, tm * t2)
{
  ulong sec1 = mktime(t1);
  ulong sec2 = mktime(t2);
  return (sec1 - sec2);
}

time_t testDayLight(ulong secsSince1900)
{
  tm t;
  t = *mkTmStruct(secsSince1900);

  int month = t.tm_mon;
  if (month < 2 || month > 9) // month 1, 2, 11, 12
    return 0;  // -> Winter

  // after last Sunday 2:00 in March
  if (t.tm_mday - t.tm_wday >= 25 && (t.tm_wday || t.tm_hour >= 2))
    if (month == 9) // October -> Winter
      return 0;
  // before last Sunday 2:00 in March
    else if (month == 2) // -> Winter
      return 0;
  // we have daylight set, so return the timediff
  return 3600;
}

ulong removeDayLight(ulong secsSince1900)
{
  ulong dst=0;
  dst = testDayLight(secsSince1900);
  return secsSince1900 - dst;
}

//see http://www.mikrocontroller.net/attachment/highlight/8391
struct tm * localtime(const ulong secsSince1900, long timezone, int daylight)
{
  static tm t;
  memcpy(&t, &Time, sizeof(tm));
  // Unix time starts 1970. That's 2208988800 seconds from 1900:
#define STARTYEAR 1970  /* start year */
  const ulong seventyYears = 2208988800UL; //0x83AA7E80
  // subtract seventy years:
  ulong epoch = (ulong)secsSince1900 - seventyYears;

  epoch += timezone;

  int year, month, days, leap400, daysofyear;
  t.tm_sec  = epoch % 60;
  epoch /= 60;
  t.tm_min  = epoch % 60;
  epoch /= 60;
  t.tm_hour = epoch % 24;
  days = epoch / 24;
  t.tm_yday = days;

  t.tm_wday = (days + 4) % 7; // sunday=0

  year = STARTYEAR % 100;
  leap400 = 4 - ((STARTYEAR - 1) / 100 & 3);

  while (true) //until break
  {
    daysofyear = 365;
    if ((year & 3) == 0)
    {
      daysofyear = 366; // leap year
      if (year == 0 || year == 100 || year == 200) // 100 year exception
        if (--leap400)  // 400 year exception
          daysofyear = 365;
    }
    if (days < daysofyear)
      break;
    days -= daysofyear;
    year++;            // 00..136 / 99..235
  }
  t.tm_year = year + STARTYEAR / 100 * 100;
  if (daysofyear & 1 && days > 58) // no leap year and after 28.2.
    days++;           // skip 29.2.

  for (month = 0; days >= dayOfMonth[month]; month++)
    days -= dayOfMonth[month];

  t.tm_mon  = month;    // 0..11
  t.tm_mday = days + 1; // 1..31

  if (daylight > 0)
  {
    ulong secs = mktime(&t);
    secs += testDayLight(secs);
    t = *mkTmStruct(secs);
  }

  return &t;
}

