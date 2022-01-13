#include "CarAnalyzerABRP.h"

CarAnalyzerABRP::CarAnalyzerABRP(CarAnalyzerGsm *gsm, const char *abrpServerHost, uint16_t abrpServerPort)
{

    this->_abrpServerHost = (char *)ps_malloc(50 * sizeof(char));
    this->_abrpApiKey = (char *)ps_malloc(50 * sizeof(char));
    this->_abrpToken = (char *)ps_malloc(50 * sizeof(char));
    this->_abrpCarModel = (char *)ps_malloc(50 * sizeof(char));

    strcpy(this->_abrpServerHost, abrpServerHost);
    this->_abrpServerPort = abrpServerPort;

    this->_gsm = gsm;

    this->_lastUpdate = millis();

    CarAnalyzerLog_d("Car Analyzer ABRP successfully initialized");
}

void CarAnalyzerABRP::begin(const char *abrpApiKey, const char *abrpToken, const char *abrpCarModel)
{
    CarAnalyzerLog_d("Starts to initialize ABRP parameters: apiKey=%s, token=%s, carModel=%s", abrpApiKey, abrpToken, abrpCarModel);
    strcpy(this->_abrpApiKey, abrpApiKey);
    strcpy(this->_abrpToken, abrpToken);
    strcpy(this->_abrpCarModel, abrpCarModel);
    CarAnalyzerLog_d("ABRP parameters initiialized: apiKey=%s, token=%s, carModel=%s", this->_abrpApiKey, this->_abrpToken, this->_abrpCarModel);
}

boolean CarAnalyzerABRP::publish(JsonObject gpsJson, JsonObject carJson)
{
    this->_lastUpdate = millis();

    SpiRamJsonDocument doc(1000);

    if (!carJson.isNull())
    {

        if (carJson.containsKey("SOCDisplay_battery_%"))
        {
            doc["soc"] = carJson["SOCDisplay_battery_%"];
        }
        if (carJson.containsKey("SOH_battery_%"))
        {
            doc["soh"] = carJson["SOH_battery_%"];
        }
    }

    if (!gpsJson.isNull() && gpsJson["valid"])
    {
        if (gpsJson.containsKey("epoch"))
        {
            doc["utc"] = gpsJson["epoch"];
        }
        if (gpsJson.containsKey("speed__km"))
        {
            doc["speed"] = gpsJson["speed__km/h"];
        }
        if (gpsJson.containsKey("latitude__°"))
        {
            doc["lat"] = gpsJson["latitude__°"];
        }
        if (gpsJson.containsKey("longitude__°"))
        {
            doc["lon"] = gpsJson["longitude__°"];
        }
        if (gpsJson.containsKey("altitude__m"))
        {
            doc["elevation"] = gpsJson["altitude__m"];
        }
    }

    if (!doc.isNull())
    {
        doc["car_model"] = this->_abrpCarModel;

        char *serializedDoc = (char *)ps_malloc(measureJson(doc) + 1);
        serializeJson(doc, serializedDoc, measureJson(doc) + 1);
        CarAnalyzerLog_d("Data for ABRP: %s", serializedDoc);

        HttpClient httpClient(*this->_gsm->getABRPClient(), this->_abrpServerHost, this->_abrpServerPort);

        String r = String("/1/tlm/send") + "?api_key=" + String(this->_abrpApiKey) + "&token=" + String(this->_abrpToken) + "&tlm=" + serializedDoc;
        CarAnalyzerLog_d("URL ABRP: %s", r.c_str());

        httpClient.get(r);

        CarAnalyzerLog_d("result code form ABRP: %d", httpClient.responseStatusCode());

        free(serializedDoc);

        CarAnalyzerLog_d("After Abrp Update:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

        if (httpClient.responseStatusCode() == 200)
        {
            return true;
        }
    }
    return false;
}

uint32_t CarAnalyzerABRP::getLastUpdate(void)
{
    return this->_lastUpdate;
}