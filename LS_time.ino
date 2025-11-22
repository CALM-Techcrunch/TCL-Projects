
#include "LS_config.h"
#include <RTClib.h>

#define CURRENT_YEAR    25

const char daysOfTheWeek[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char monthsOfTheYear[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
RTC_DS3231 rtc;
bool rtcFound;

bool LS_timeInit( void )
{
    Serial.println("[RTC] Initializing RTC on I2C...");
    
    if (!rtc.begin())
    {
        Serial.println("[RTC] ERROR: RTC not found on I2C bus!");
        Serial.println("[RTC] Check: DS3231 connected to D1(SCL) and D3(SDA)?");
        Serial.println("[RTC] Check: Pull-up resistors present on I2C lines?");
        rtcFound = false;
        return false;
    }
    
    Serial.println("[RTC] SUCCESS: RTC found on I2C bus!");

    if (rtc.lostPower())
    {
        Serial.println("[RTC] WARNING: RTC lost power - resetting to compile date/time");
        // RTC lost power, adjust it to compile date/time
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.print("[RTC] Time set to: ");
        Serial.println(F(__DATE__));
    }
    else
    {
        Serial.println("[RTC] Battery is good - retaining stored time");
    }

    rtcFound = true;
    Serial.println("[RTC] Initialization complete");
    return true;
}

bool LS_timeRead( void )
{
    if(!rtcFound)
    {
        Serial.println("[RTC] ERROR: RTC not initialized - cannot read time");
        return false;
    }
    
    DateTime now = rtc.now();
    
    time_date.tm_wday = now.dayOfTheWeek();
    time_date.tm_mday = now.day();
    time_date.tm_mon = now.month();
    time_date.tm_year = now.year() - 2000;
    time_date.tm_hour = now.hour();
    time_date.tm_min = now.minute();
    time_date.tm_sec = now.second();
    
    // Debug output with formatted time
    Serial.print("[RTC] Current time: ");
    Serial.print(time_date.tm_year + 2000);
    Serial.print("-");
    Serial.print(time_date.tm_mon);
    Serial.print("-");
    Serial.print(time_date.tm_mday);
    Serial.print(" ");
    Serial.print(daysOfTheWeek[time_date.tm_wday]);
    Serial.print(" ");
    if(time_date.tm_hour < 10) Serial.print("0");
    Serial.print(time_date.tm_hour);
    Serial.print(":");
    if(time_date.tm_min < 10) Serial.print("0");
    Serial.print(time_date.tm_min);
    Serial.print(":");
    if(time_date.tm_sec < 10) Serial.print("0");
    Serial.println(time_date.tm_sec);
    
    return rtcFound;
}

bool LS_timeSet(struct tm* new_time) 
{    
    if(new_time->tm_year < CURRENT_YEAR) return false;
    // January 21, 2024 at 3am you would call: rtc.adjust(DateTime(2024, 1, 21, 3, 0, 0));
    rtc.adjust(DateTime(new_time->tm_year, new_time->tm_mon, new_time->tm_mday, new_time->tm_hour, new_time->tm_min, new_time->tm_sec)); 
    return true;
}

/* ------------------------------- END OF FILE ------------------------------ */
