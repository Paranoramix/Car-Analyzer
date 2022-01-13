#ifndef _CAR_ANALYZER_GPS_H
#define _CAR_ANALYZER_GPS_H

#ifndef TINY_GSM_MODEM_SIM7600
#define TINY_GSM_MODEM_SIM7600
#endif

#ifndef TINY_GSM_USE_GPRS
#define TINY_GSM_USE_GPRS true
#endif

//#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include "../../include/StreamDebugger.h"
#endif

#include <ArduinoJson.h>
#include <TinyGSMClient.h>
#include <ESP32Time.h>

#include "../../include/CarAnalyzerLog.h"
#include "../../include/CarAnalyzerArduinoJson.h"

/**
 * @brief This class is used to manage the GPS module provided by SIM7600E component.
 * 
 */
class CarAnalyzerGps
{
private:
    TinyGsm *_gsmModem;

    SpiRamJsonDocument *_data;

    unsigned long _lastUpdate;

    uint8_t _precision;

    float _previousLatitude;
    float _previousLongitude;
    float _previousAltitude;

public:
    /**
     * @brief Construct a new Car Analyzer Gps:: Car Analyzer Gps object
     * 
     */
    CarAnalyzerGps(void);

    /**
     * @brief Initialize component.
     * 
     * @param baud Speed to communicate with SIM7600.
     * @param rxPin SIM7600 Modem RX Pin.
     * @param txPin SIM7600 Modem TX Pin.
     */
    void begin(uint32_t, uint8_t, uint8_t);

    /**
     * @brief Connect to GPS.
     *
     * @return true when GPS is successfully enabled.
     * @return false when failed.
     */
    bool connect(void);

    /**
     * @brief Updates GPS data from SIM7600.
     * 
     */
    void update(void);
    bool disconnect(void);

    /**
     * @brief Returns a JsonObject corresponding to the GPS Data.
     * 
     * @return JsonObject 
     */
    JsonObject getData(void);

    /**
     * @brief Returns a specific value from data.
     * 
     * @param key the data name.
     * @return String value found (empty if not found).
     */
    String getValue(const char *);

    /**
     * @brief Returns the last time when data was updated.
     * 
     * @return uint32_t last Update.
     */
    unsigned long getLastUpdate(void);

    bool isValid(void);

    void setPrecision(uint8_t);
};

#endif