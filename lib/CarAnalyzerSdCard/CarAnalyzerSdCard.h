#ifndef _CAR_ANALYZER_SD_CARD_H
#define _CAR_ANALYZER_SD_CARD_H

#include "../../include/CarAnalyzerArduinoJson.h"

#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>
#include <Update.h>


#include "../../include/CarAnalyzerLog.h"

class CarAnalyzerSdCard
{
private:
    SpiRamJsonDocument *_data;

    uint32_t _lastUpdate;

public:
    CarAnalyzerSdCard(uint8_t, int8_t, int8_t, int8_t);
    bool isMounted(void);

    JsonObject readJsonFile(const char*);

    bool saveFile(JsonObject, const char*, bool);
    bool updateFile(JsonObject, const char*, const char*, bool);

    void checkForUpdate(void);

    uint32_t getLastUpdate(void);
};

#endif