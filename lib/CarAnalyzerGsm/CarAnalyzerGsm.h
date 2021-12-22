#ifndef _CAR_ANALYZER_GSM_H
#define _CAR_ANALYZER_GSM_H


#ifndef TINY_GSM_MODEM_SIM7000
#define TINY_GSM_MODEM_SIM7000
#endif

#ifndef TINY_GSM_USE_GPRS
#define TINY_GSM_USE_GPRS true
#endif

#include <SoftwareSerial.h>
#include <TinyGSMClient.h>

#include "../../include/CarAnalyzerLog.h"

class CarAnalyzerGsm {
    private:
        SoftwareSerial* _gsmSerial;

        TinyGsm *_gsmModem;
        TinyGsmClient *_abrpGsmClient;
        TinyGsmClient *_homeAssistantGsmClient;
        
    public:
        CarAnalyzerGsm(void);

        void begin(uint8_t, uint8_t);
        
        bool connect(const char*, const char*, const char*, const char*);
        bool disconnect(void);
};

#endif