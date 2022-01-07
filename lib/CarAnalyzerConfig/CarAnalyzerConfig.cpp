#include "CarAnalyzerConfig.h"

CarAnalyzerConfig::CarAnalyzerConfig(void)
{
    this->_data = new SpiRamJsonDocument(2000);
}

bool CarAnalyzerConfig::loadConfiguration(JsonObject document)
{
    return this->_data->set(document);
}



String CarAnalyzerConfig::getValue(const char* key) {
        if (this->_data->containsKey(key))
    {
        
        return (*this->_data)[key].as<String>();
    }

    return "";

}


JsonObject CarAnalyzerConfig::getData(void)
{
    return (*this->_data).as<JsonObject>();
}
