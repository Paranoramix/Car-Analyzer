#ifndef _CAR_ANALYZER_A_B_R_P_H
#define _CAR_ANALYZER_A_B_R_P_H

#include "../../include/CarAnalyzerArduinoJson.h"

#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

#include "../../include/CarAnalyzerLog.h"
#include "../CarAnalyzerGsm/CarAnalyzerGsm.h"

class CarAnalyzerABRP
{
private:
    CarAnalyzerGsm *_gsm;

    char* _abrpServerHost;
    uint16_t _abrpServerPort;
    char *_abrpApiKey;
    char *_abrpToken;
    char *_abrpCarModel;

    uint32_t _lastUpdate;

public:
    CarAnalyzerABRP(CarAnalyzerGsm *, const char *, uint16_t);
    void begin(const char *, const char *, const char *);
    boolean publish(JsonObject, JsonObject);

    uint32_t getLastUpdate(void);
};

#endif