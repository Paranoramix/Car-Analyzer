#ifndef _CAR_ANALYZER_OBD_H
#define _CAR_ANALYZER_OBD_H

#include "../../include/CarAnalyzerArduinoJson.h"

#include <ArduinoJson.h>

#include "../../include/CarAnalyzerLog.h"

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include "tinyexpr.h"


class CarAnalyzerObd
{
private:
    BluetoothSerial *_btSerial;
    ELM327 *_elm;

    uint8_t _elmPinControl;
    uint8_t _macAddress[6];

    const char *_name;
    const char *_pin;

    uint32_t _lastUpdate;

    SpiRamJsonDocument *_configuration;
    SpiRamJsonDocument* _data;
    SpiRamJsonDocument* _dataRaw;

    double extractCarData(const char *, uint8_t, uint8_t );


public:
    CarAnalyzerObd(uint8_t);

    bool begin(const char *, const char *, const char *);
    bool connect(void);
    bool disconnect(void);

    bool loadConfiguration(JsonObject);

    bool readCarData(const char *, const char *, uint8_t);

    bool switchOff(void);
    bool switchOn(void);

    bool update(void);

    void end(void);

    uint32_t getLastUpdate(void);
    
    /**
     * @brief Returns a JsonObject corresponding to the extracted Car Data.
     * 
     * @return JsonObject 
     */
    JsonObject getData(void);
    JsonObject getDataRaw(void);

};

#endif