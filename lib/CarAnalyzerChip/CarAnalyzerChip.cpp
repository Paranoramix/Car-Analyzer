#include "CarAnalyzerChip.h"

CarAnalyzerChip::CarAnalyzerChip(uint8_t batteryVoltagePin, uint8_t alimVoltagePin)
{
    psramInit();
    this->_data = new SpiRamJsonDocument(1000);

    this->_lastUpdate = millis();
    this->_batteryVoltagePin = batteryVoltagePin;
    this->_alimVoltagePin = alimVoltagePin;

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(this->_batteryVoltagePin, INPUT);
    pinMode(this->_alimVoltagePin, INPUT);
}

boolean CarAnalyzerChip::update(void)
{
    this->_data->clear();
    
    uint32_t chipId = 0;

    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    String chip = String(chipId, HEX);
    chip.toUpperCase();

    double batteryVoltage = 0;
    double alimVoltage = 0;
    for (int i = 0 ; i < 100 ; i++) {
        batteryVoltage += analogRead(this->_batteryVoltagePin);
        alimVoltage += analogRead(this->_alimVoltagePin);
    }

    batteryVoltage = batteryVoltage / 100;
    alimVoltage = alimVoltage / 100;

    if (alimVoltage > 0)
    {
        digitalWrite(LED_PIN, LOW);

        (*this->_data)["powerSupply"] = "External";
    }
    else
    {
        digitalWrite(LED_PIN, HIGH);
        (*this->_data)["powerSupply"] = "Battery";
    }

    (*this->_data)["chipExternalVoltage_voltage_V"] = (float)(2 * alimVoltage / 1000.0);
    (*this->_data)["chipBatteryVoltage_voltage_V"] = (float)(2 * batteryVoltage / 1000.0);

    (*this->_data)["id"] = chip;
    (*this->_data)["cores"] = ESP.getChipCores();
    (*this->_data)["model"] = ESP.getChipModel();
    (*this->_data)["revision"] = ESP.getChipRevision();
    (*this->_data)["cpu_frequency_MHz"] = ESP.getCpuFreqMHz();
    (*this->_data)["cycleCount__cycles"] = ESP.getCycleCount();
    (*this->_data)["eFuseMac"] = ESP.getEfuseMac();
    (*this->_data)["flashChipMode"] = ESP.getFlashChipMode();
    (*this->_data)["flashChipSize__Mb"] = (float)((ESP.getFlashChipSize() / 1024.0) / 1024.0);
    (*this->_data)["flashChipSpeed_frequency_MHz"] = (float)((ESP.getFlashChipSpeed() / 1000.0) / 1000.0);
    (*this->_data)["freeHeap__b"] = ESP.getFreeHeap();
    (*this->_data)["freePsram__b"] = ESP.getFreePsram();
    (*this->_data)["freeSketchSpace__b"] = ESP.getFreeSketchSpace();
    (*this->_data)["heapSize__b"] = ESP.getHeapSize();
    (*this->_data)["maxAllocHeap__b"] = ESP.getMaxAllocHeap();
    (*this->_data)["maxAllocPsram__b"] = ESP.getMaxAllocPsram();
    (*this->_data)["minAllocHeap__b"] = ESP.getMinFreeHeap();
    (*this->_data)["minAllocPsram__b"] = ESP.getMinFreePsram();
    (*this->_data)["psramSize__b"] = ESP.getPsramSize();
    (*this->_data)["sdkVersion"] = ESP.getSdkVersion();
    (*this->_data)["sketchMD5"] = ESP.getSketchMD5();
    (*this->_data)["sketchSize__b"] = ESP.getSketchSize();

    return true;
}

JsonObject CarAnalyzerChip::getData(void)
{
    return (*this->_data).as<JsonObject>();
}

unsigned long CarAnalyzerChip::getLastUpdate(void)
{
    return this->_lastUpdate;
}

bool CarAnalyzerChip::isBatteryPowered(void)
{
    if (analogRead(this->_alimVoltagePin) > 0)
    {
        return false;
    }

    return true;
}