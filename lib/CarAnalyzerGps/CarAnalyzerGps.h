#ifndef _CAR_ANALYZER_GPS_H
#define _CAR_ANALYZER_GPS_H


#ifndef TINY_GSM_MODEM_SIM7000
#define TINY_GSM_MODEM_SIM7000
#endif

#ifndef TINY_GSM_USE_GPRS
#define TINY_GSM_USE_GPRS true
#endif

#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <TinyGSMClient.h>
#include <ESP32Time.h>

#include "../../include/CarAnalyzerLog.h"
#include "../../include/CarAnalyzerArduinoJson.h"

#ifndef _GPS_HISTORIC_MAX_SIZE
#define _GPS_HISTORIC_MAX_SIZE 50000
#endif

class CarAnalyzerGps {
    private:
        SoftwareSerial* _gsmSerial;

        TinyGsm *_gsmModem;

        uint8_t _gsmTxPin;
        uint8_t _gsmRxPin;

        const char* _simPin;

        SpiRamJsonDocument* _data;
        
    public:
        CarAnalyzerGps(void);

        void begin(uint8_t, uint8_t);
        
        bool connect(void);
        void printGPS(void);
        bool disconnect(void);
};

#endif