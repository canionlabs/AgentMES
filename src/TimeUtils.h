#pragma once

#include "Arduino.h"
#include <TimeLib.h>
#include <NTPtimeESP.h>

NTPtime NTPch("0.br.pool.ntp.org");
strDateTime currentDate;
time_t currentTime = 0;

time_t getNtpTime()
{
	Serial.println("Updating date...");

	currentDate = NTPch.getNTPtime(-3.0, 0);

	if (currentDate.valid)
	{
		setSyncInterval(300);
		Serial.println("Valid date");

		return currentDate.epochTime;
	}
	else
	{
		setSyncInterval(1);
	}

	return 0;
}

void digitalClockDisplay()
{
	// digital clock display of the time
	Serial.print(hour());
	Serial.print(":");
	Serial.print(minute());
	Serial.print(":");
	Serial.print(second());
	Serial.print(" ");
	Serial.print(day());
	Serial.print(" ");
	Serial.print(month());
	Serial.print(" ");
	Serial.print(year());
	Serial.println();
}

void setupTime()
{
	setSyncProvider(getNtpTime);
	setSyncInterval(1);
}

void loopTime()
{
	// Update time
	if (timeStatus() != timeNotSet)
	{
		if (now() != currentTime)
		{ //update the display only if time has changed
			currentTime = now();
			digitalClockDisplay();
		}
	}
}