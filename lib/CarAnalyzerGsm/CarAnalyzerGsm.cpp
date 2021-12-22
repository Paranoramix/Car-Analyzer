#include "CarAnalyzerGsm.h"

CarAnalyzerGsm::CarAnalyzerGsm(void)
{
    psramInit();

    this->_gsmSerial = (SoftwareSerial *)ps_malloc(sizeof(SoftwareSerial));
    this->_gsmSerial = new SoftwareSerial();

    this->_gsmModem = (TinyGsm *)ps_malloc(sizeof(TinyGsm));
    ;
    this->_abrpGsmClient = (TinyGsmClient *)ps_malloc(sizeof(TinyGsmClient));
    this->_homeAssistantGsmClient = (TinyGsmClient *)ps_malloc(sizeof(TinyGsmClient));
}

/**
 * @brief Initialize component.
 * 
 * @param rxPin 
 * @param txPin 
 */
void CarAnalyzerGsm::begin(uint8_t rxPin, uint8_t txPin)
{
    this->_gsmSerial->begin(9600, SWSERIAL_8N1, rxPin, txPin, false);

    this->_gsmModem = new TinyGsm(*this->_gsmSerial);
}

/**
 * @brief Connect to GPRS.
 * 
 * @param pin 
 * @param apn 
 * @param user 
 * @param password 
 * @return true 
 * @return false 
 */
bool CarAnalyzerGsm::connect(const char *pin, const char *apn, const char *user, const char *password)
{
    if (!this->_gsmModem->init(pin))
    {
        if (!this->_gsmModem->restart(pin))
        {
            CarAnalyzerLog_e("GSM cannot be restarted... [%s]", pin);
            return false;
        }
    }

    if (this->_gsmModem->getSimStatus() != 1 && this->_gsmModem->getSimStatus() != 3)
    {
        if (!this->_gsmModem->simUnlock(pin))
        {
            CarAnalyzerLog_e("GSM cannot be sim unlocked... [%s]", pin);
            return false;
        }
    }

    if (!this->_gsmModem->isGprsConnected())
    {
        if (!this->_gsmModem->gprsConnect(apn, user, password))
        {
            CarAnalyzerLog_e("GSM cannot be connect GPRS... [%s]", pin);
            return false;
        }
    }

    return this->_gsmModem->isGprsConnected();
}