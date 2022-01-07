#ifndef _CAR_ANALYZER_GSM_H
#define _CAR_ANALYZER_GSM_H

#ifndef TINY_GSM_MODEM_SIM7600
#define TINY_GSM_MODEM_SIM7600
#endif

#ifndef TINY_GSM_USE_GPRS
#define TINY_GSM_USE_GPRS true
#endif

#ifdef DUMP_AT_COMMANDS
#include "../../include/StreamDebugger.h"
#endif

#include "../../include/CarAnalyzerArduinoJson.h"

#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <TinyGSMClient.h>

#include "../../include/CarAnalyzerLog.h"
#include "../CarAnalyzerGps/CarAnalyzerGps.h"

class CarAnalyzerGsm
{
private:
    TinyGsm *_gsmModem;
    TinyGsmClient *_abrpGsmClient;
    TinyGsmClient *_homeAssistantGsmClient;

    uint8_t _gsmPowerKey;
    uint8_t _gsmFlight;

    char* _pin;
    char* _apn;
    char* _username;
    char* _password;

    uint32_t _lastUpdate;

    SpiRamJsonDocument *_data;

public:
    CarAnalyzerGsm(uint8_t, uint8_t);

    void begin(uint32_t, uint8_t, uint8_t);

    bool connect(const char *, const char *, const char *, const char *);
    bool disconnect(void);
    void update(void);

    void powerOff(void);

    TinyGsmClient *getHomeAssistantClient(void);
    TinyGsmClient *getABRPClient(void);

    JsonObject getData(void);

    uint32_t getLastUpdate(void);
};

#endif