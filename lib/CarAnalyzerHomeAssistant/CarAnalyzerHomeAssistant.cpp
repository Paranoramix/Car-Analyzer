#include "CarAnalyzerHomeAssistant.h"

CarAnalyzerHomeAssistant::CarAnalyzerHomeAssistant(CarAnalyzerGsm *gsm)
{
    this->_mqttClient = new PubSubClient(*gsm->getHomeAssistantClient());

    this->_name = (char *)ps_malloc(100 * sizeof(char));
    this->_mqttUsername = (char *)ps_malloc(100 * sizeof(char));
    this->_mqttPassword = (char *)ps_malloc(100 * sizeof(char));
    this->_lastUpdate = millis();

    CarAnalyzerLog_d("Car Analyzer Home Assistant successfully initialized");
}

void CarAnalyzerHomeAssistant::begin(const char *url, uint16_t port)
{
    this->_mqttClient->setServer(url, port);
    this->_mqttClient->setBufferSize(2048);
}

boolean CarAnalyzerHomeAssistant::connect(const char *name, const char *mqttUsername, const char *mqttPassword)
{
    strcpy(this->_name, name);
    strcpy(this->_mqttUsername, mqttUsername);
    strcpy(this->_mqttPassword, mqttPassword);

    return this->_mqttClient->connect(name, mqttUsername, mqttPassword);
}

boolean CarAnalyzerHomeAssistant::_reconnect(void)
{
    return this->connect(this->_name, this->_mqttUsername, this->_mqttPassword);
}

boolean CarAnalyzerHomeAssistant::discover(const char *component, const char *sensorName)
{
    uint32_t chipId = 0;

    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    String sensorNameStr = String(sensorName);
    SpiRamJsonDocument doc(1000);

    if (sensorNameStr.indexOf("_") > -1)
    {
        if (sensorNameStr.lastIndexOf("_") != sensorNameStr.indexOf("_") + 1)
        {

            if (sensorNameStr.lastIndexOf("_") == sensorNameStr.indexOf("_"))
            {
                String device_class = sensorNameStr.substring(sensorNameStr.indexOf("_") + 1);
                doc["device_class"] = device_class;
            }
            else
            {
                String device_class = sensorNameStr.substring(sensorNameStr.indexOf("_") + 1, sensorNameStr.lastIndexOf("_"));
                doc["device_class"] = device_class;
            }
        }

        if (sensorNameStr.lastIndexOf("_") != sensorNameStr.indexOf("_") && sensorNameStr.lastIndexOf("_") + 1 < sensorNameStr.length())
        {
            String unit_of_measurement = sensorNameStr.substring(sensorNameStr.lastIndexOf("_") + 1);
            doc["unit_of_measurement"] = unit_of_measurement;
        }
    }

    doc["name"] = "Car Analyzer - " + String(chipId, HEX) + " - " + sensorNameStr.substring(0, sensorNameStr.indexOf("_"));
    doc["uniq_id"] = String(chipId, HEX) + "-" + sensorNameStr.substring(0, sensorNameStr.indexOf("_"));

    JsonObject device = doc.createNestedObject("device");
    device["name"] = "Car Analyzer " + String(chipId, HEX);
    device["model"] = "Car Analyzer";
    device["manufacturer"] = "Paranoramix";

    JsonArray deviceIds = device.createNestedArray("ids");
    deviceIds.add(String(chipId, HEX));

    // Specific component: GPS
    if (sensorNameStr.equals("gpslocation")) {
        doc["source_type"] = "gps";
        doc["json_attributes_topic"] = "homeassistant/device_tracker/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "/attributes";
        doc["state_topic"] = "homeassistant/device_tracker/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "/state";
    } else {
        doc["state_topic"] = "homeassistant/sensor/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "/state";
        doc["value_template"] = "{{ value_json." + sensorNameStr.substring(0, sensorNameStr.indexOf("_")) + " }}";
    }


    if (!this->_mqttClient->connected())
    {
        if (!this->_reconnect())
        {
            return false;
        }
    }

    char *serializedDoc = (char *)ps_malloc(measureJson(doc) + 1);
    size_t n = serializeJson(doc, serializedDoc, measureJson(doc) + 1);

    bool r= false;
    if (sensorNameStr.equals("gpslocation")) {
        r = this->_mqttClient->publish(("homeassistant/device_tracker/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "_" + sensorNameStr.substring(0, sensorNameStr.indexOf("_")) + "/config").c_str(), serializedDoc, n);
    } else {
        r = this->_mqttClient->publish(("homeassistant/sensor/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "_" + sensorNameStr.substring(0, sensorNameStr.indexOf("_")) + "/config").c_str(), serializedDoc, n);
    }

    free(serializedDoc);
    return r;
}

boolean CarAnalyzerHomeAssistant::publish(const char *component, JsonObject json)
{
    SpiRamJsonDocument doc(2000);

    uint32_t chipId = 0;

    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    for (JsonPair kv : json)
    {
        String sensorName = kv.key().c_str();

        this->discover(component, sensorName.c_str());
        if (sensorName.indexOf("gpslocation") > -1) 
        {
            JsonObject attr = kv.value().as<JsonObject>();
            char* serializedAttr = (char *)ps_malloc(measureJson(attr) + 1);
            size_t na = serializeJson(attr, serializedAttr, measureJson(attr) + 1);
            this->_mqttClient->publish(("homeassistant/device_tracker/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "/attributes").c_str(), serializedAttr, na);
            free(serializedAttr);
        } else {
            doc[sensorName.substring(0, sensorName.indexOf("_"))] = kv.value();
        }
    }

    if (!this->_mqttClient->connected())
    {
        if (!this->_reconnect())
        {
            return false;
        }
    }

    char *serializedDoc = (char *)ps_malloc(measureJson(doc) + 1);
    size_t n = serializeJson(doc, serializedDoc, measureJson(doc) + 1);

    CarAnalyzerLog_d("%s", serializedDoc);

    bool r = this->_mqttClient->publish(("homeassistant/sensor/CarAnalyzer/" + String(chipId, HEX) + "_" + component + "/state").c_str(), serializedDoc, n);
    free(serializedDoc);
    return r;
}

uint32_t CarAnalyzerHomeAssistant::getLastUpdate(void)
{
    return this->_lastUpdate;
}