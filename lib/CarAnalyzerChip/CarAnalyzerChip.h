#ifndef _CAR_ANALYZER_CHIP_H
#define _CAR_ANALYZER_CHIP_H

#include "../../include/CarAnalyzerArduinoJson.h"
#include "../../include/CarAnalyzerConstants.h"

#include <ArduinoJson.h>

#include "../../include/CarAnalyzerLog.h"

class CarAnalyzerChip {
    private:
    SpiRamJsonDocument *_data;

    uint8_t _batteryVoltagePin;
    unsigned long _lastUpdate;

    public:
        CarAnalyzerChip(uint8_t);

        boolean update(void);

        JsonObject getData(void);

        JsonObject getDiscovery(const char*);

        unsigned long getLastUpdate(void);

        bool isBatteryPowered(void);
};

#endif