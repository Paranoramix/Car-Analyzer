#ifndef _CAR_ANALYZER_CONFIG_H
#define _CAR_ANALYZER_CONFIG_H

#include "../../include/CarAnalyzerArduinoJson.h"

#include <ArduinoJson.h>

#include "../../include/CarAnalyzerLog.h"

class CarAnalyzerConfig
{
private:
    SpiRamJsonDocument *_data;

public:
    CarAnalyzerConfig(void);

    bool loadConfiguration(JsonObject);

    /**
     * @brief Returns a JsonObject corresponding to the configuration data.
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
};

#endif