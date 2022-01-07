#ifndef _CAR_ANALYZER_HOME_ASSISTANT_H
#define _CAR_ANALYZER_HOME_ASSISTANT_H

#include "../../include/CarAnalyzerArduinoJson.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "../../include/CarAnalyzerLog.h"
#include "../CarAnalyzerGsm/CarAnalyzerGsm.h"

/**
 * @brief This class allows to communicate with an instance of HomeAssistant.
 *
 * It's necessary to authorize access to your Home Assistant's instance.
 * You have to install the MQTT integration in your Home Assistant's instance.
 */
class CarAnalyzerHomeAssistant
{
private:
    PubSubClient *_mqttClient;

    char *_name;
    char *_mqttUsername;
    char *_mqttPassword;

    boolean _reconnect(void);

    uint32_t _lastUpdate;

public:
    /**
     * @brief Construct a new Car Analyzer Home Assistant object
     * 
     */
    CarAnalyzerHomeAssistant(CarAnalyzerGsm *);

    /**
     * @brief Sets the hostname and port for MQTT transmission.
     * 
     * @param url: hostname. 
     * @param port: port listening MQTT messages.
     * 
     */
    void begin(const char *, uint16_t);

    /**
     * @brief Connects with a specified name, username and password.
     * 
     * @param name: a name for the component initiating connection. 
     * @param username: the username allowed to connect it.
     * @param password: the associated password.
     * 
     * @return boolean indicating connection status.
     */
    boolean connect(const char *, const char *, const char *);

    boolean discover(const char *, const char *);
    boolean publish(const char *, JsonObject);

    uint32_t getLastUpdate(void);
};

#endif