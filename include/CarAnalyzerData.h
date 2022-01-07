#include <Arduino.h>

#ifndef _CAR_ANALYZER_DATA_H
#define _CAR_ANALYZER_DATA_H


struct Timers
{
    long gsmTimer;
    long abrpTimer;
    long gpsTimer;
    long bluetoothTimer;
    long chipTimer;
    long carTimer;
    long homeAssistantTimer;
};


Timers *updateTimers;

#endif