#include "CarAnalyzerGsm.h"

#ifdef DUMP_AT_COMMANDS
StreamDebugger debuggerGsm(Serial1, Serial);
#endif

/**
 * @brief Construct a new Car Analyzer Gsm:: Car Analyzer Gsm object
 * 
 * @param powerPin 
 * @param flightPin 
 */
CarAnalyzerGsm::CarAnalyzerGsm(uint8_t powerPin, uint8_t flightPin)
{
    psramInit();

    this->_gsmPowerKey = powerPin;
    this->_gsmFlight = flightPin;

    pinMode(this->_gsmPowerKey, OUTPUT);
    pinMode(this->_gsmFlight, OUTPUT);

    digitalWrite(this->_gsmPowerKey, HIGH);
    delay(300);
    digitalWrite(this->_gsmPowerKey, LOW);

    digitalWrite(this->_gsmFlight, HIGH);

    this->_gsmModem = (TinyGsm *)ps_malloc(sizeof(TinyGsm));
    ;
    this->_abrpGsmClient = (TinyGsmClient *)ps_malloc(sizeof(TinyGsmClient));
    this->_homeAssistantGsmClient = (TinyGsmClient *)ps_malloc(sizeof(TinyGsmClient));

    this->_data = new SpiRamJsonDocument(2000);

    this->_lastUpdate = millis();

    this->_pin = (char *)ps_malloc(10 * sizeof(char));
    this->_apn = (char *)ps_malloc(50 * sizeof(char));
    this->_username = (char *)ps_malloc(50 * sizeof(char));
    this->_password = (char *)ps_malloc(50 * sizeof(char));
}

/**
 * @brief Initialize component.
 * 
 * @param rxPin 
 * @param txPin 
 */
void CarAnalyzerGsm::begin(uint32_t baud, uint8_t rxPin, uint8_t txPin)
{
    Serial1.begin(baud, SERIAL_8N1, rxPin, txPin);

#ifdef DUMP_AT_COMMANDS
    this->_gsmModem = new TinyGsm(debuggerGsm);
#else
    this->_gsmModem = new TinyGsm(Serial1);
#endif

    this->_abrpGsmClient = new TinyGsmClient(*this->_gsmModem, 1);
    this->_homeAssistantGsmClient = new TinyGsmClient(*this->_gsmModem, 2);
}

void CarAnalyzerGsm::reconnect(void) {
    this->connect(this->_pin, this->_apn, this->_username, this->_password);
}

void CarAnalyzerGsm::reset(void) {
    if (this->_abrpGsmClient->connected()) {
        this->_abrpGsmClient->stop();
    }

    if (this->_homeAssistantGsmClient->connected()) {
        this->_homeAssistantGsmClient->stop();
    }
    
    this->_gsmModem->init(this->_pin);
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
    strcpy(this->_pin, pin);
    strcpy(this->_apn, apn);
    strcpy(this->_username, user);
    strcpy(this->_password, password);

    CarAnalyzerLog_d("Modem Name: %s", this->_gsmModem->getModemName().c_str());
    CarAnalyzerLog_d("Modem Info: %d", this->_gsmModem->getModemInfo().c_str());
    CarAnalyzerLog_d("GSM connection");

    this->reset();

    if (!this->_gsmModem->init(this->_pin))
    {
        if (!this->_gsmModem->restart(this->_pin))
        {
            CarAnalyzerLog_e("GSM cannot be restarted... [%s]", this->_pin);
            return false;
        }

        CarAnalyzerLog_d("GSM restart success");
    }
    else
    {
        CarAnalyzerLog_d("GSM init success");
    }

    CarAnalyzerLog_d("Sim status: %d", this->_gsmModem->getSimStatus());
    CarAnalyzerLog_d("Network mode: %d", this->_gsmModem->getNetworkMode());

    if (this->_gsmModem->getSimStatus() != 1 && this->_gsmModem->getSimStatus() != 3)
    {
        if (!this->_gsmModem->simUnlock(this->_pin))
        {
            CarAnalyzerLog_e("GSM cannot be sim unlocked... [%s]", this->_pin);
            return false;
        }
    }

    if (!this->_gsmModem->isGprsConnected())
    {
        if (!this->_gsmModem->gprsConnect(this->_apn, this->_username, this->_password))
        {
            CarAnalyzerLog_e("GSM cannot be connect GPRS... [%s]", this->_pin);
            return false;
        }
    }

    CarAnalyzerLog_v("GPRS status: %d", this->_gsmModem->isGprsConnected());
    CarAnalyzerLog_v("CCID: %s", this->_gsmModem->getSimCCID().c_str());
    CarAnalyzerLog_v("IMEI: %s", this->_gsmModem->getIMEI().c_str());
    CarAnalyzerLog_v("IMSI: %s", this->_gsmModem->getIMSI().c_str());
    CarAnalyzerLog_v("Operator: %s", this->_gsmModem->getOperator().c_str());
    CarAnalyzerLog_v("local IP: %s", this->_gsmModem->getLocalIP().c_str());
    CarAnalyzerLog_v("Signal Quality: %d", this->_gsmModem->getSignalQuality());

    this->_abrpGsmClient->init(this->_gsmModem, 1);
    this->_homeAssistantGsmClient->init(this->_gsmModem, 2);

    return this->_gsmModem->isGprsConnected();
}

void CarAnalyzerGsm::update(void)
{
    CarAnalyzerLog_v("Updating GSM informations");

    this->_data->clear();

    this->_lastUpdate = millis();

    if (this->_gsmModem->isGprsConnected())
    {
        (*this->_data)["gprs"] = "on";
    }
    else
    {
        (*this->_data)["gprs"] = "off";
    }
    (*this->_data)["ccid"] = this->_gsmModem->getSimCCID();
    (*this->_data)["imei"] = this->_gsmModem->getIMEI();
    (*this->_data)["imsi"] = this->_gsmModem->getIMSI();
    (*this->_data)["operator"] = this->_gsmModem->getOperator();
    (*this->_data)["localIP"] = this->_gsmModem->getLocalIP();

    if (this->_gsmModem->getSignalQuality() == 99)
    {
        (*this->_data)["signalQualityCondition"] = "Not detectable";
    }
    else
    {
        (*this->_data)["signalQuality_signal_strength_dBm"] = -113 + 2 * this->_gsmModem->getSignalQuality();

        if (this->_gsmModem->getSignalQuality() <= 9)
        {
            (*this->_data)["signalQualityCondition"] = "Marginal";
        }
        else if (this->_gsmModem->getSignalQuality() <= 14)
        {
            (*this->_data)["signalQualityCondition"] = "OK";
        }
        else if (this->_gsmModem->getSignalQuality() <= 19)
        {
            (*this->_data)["signalQualityCondition"] = "Good";
        }
        else if (this->_gsmModem->getSignalQuality() <= 30)
        {
            (*this->_data)["signalQualityCondition"] = "Excellent";
        }
        else
        {
            (*this->_data)["signalQualityCondition"] = "Ideal";
        }
    }

    (*this->_data)["registrationStatus"] = this->_gsmModem->getRegistrationStatus();
    (*this->_data)["temperature_temperature_°C"] = this->_gsmModem->getTemperature();
    (*this->_data)["simStatus"] = this->_gsmModem->getSimStatus();

    if (this->_gsmModem->isNetworkConnected())
    {
        (*this->_data)["network"] = "on";
    }
    else
    {
        (*this->_data)["network"] = "off";
    }

    double gsmVoltage = 0;

    for (int i = 0 ; i < 100 ; i++) {
        gsmVoltage += this->_gsmModem->getBattVoltage();
    }

    gsmVoltage = gsmVoltage / 100.0;

    (*this->_data)["GsmBattVoltage_voltage_V"] = gsmVoltage / 1000.0;

    if (gsmVoltage >= 4000)
    {
        (*this->_data)["GsmBattPercent_battery_%"] = 100;
    }
    else if (gsmVoltage >= 3800)
    {
        (*this->_data)["GsmBattPercent_battery_%"] = 75;
    }
    else if (gsmVoltage >= 3600)
    {
        (*this->_data)["GsmBattPercent_battery_%"] = 50;
    }
    else if (gsmVoltage >= 3400)
    {
        (*this->_data)["GsmBattPercent_battery_%"] = 25;
    }
    else if (gsmVoltage < 3400)
    {
        (*this->_data)["GsmBattPercent_battery_%"] = 0;
    }

    CarAnalyzerLog_v("Battery Voltage: %f", (gsmVoltage / 1000.0));

    CarAnalyzerLog_v("GPRS: %d", this->_gsmModem->isGprsConnected());
    CarAnalyzerLog_v("ccid: %s", this->_gsmModem->getSimCCID().c_str());
    CarAnalyzerLog_v("imei: %s", this->_gsmModem->getIMEI().c_str());
    CarAnalyzerLog_v("imsi: %s", this->_gsmModem->getIMSI().c_str());
    CarAnalyzerLog_v("operator: %s", this->_gsmModem->getOperator().c_str());
    CarAnalyzerLog_v("localIP: %s", this->_gsmModem->getLocalIP().c_str());
    CarAnalyzerLog_v("signalQuality: %ld", this->_gsmModem->getSignalQuality());
    CarAnalyzerLog_v("registrationStatus: %d", this->_gsmModem->getRegistrationStatus());
    CarAnalyzerLog_v("temperature: %f", this->_gsmModem->getTemperature());
    CarAnalyzerLog_v("simStatus: %d", this->_gsmModem->getSimStatus());
    CarAnalyzerLog_v("network: %d", this->_gsmModem->isNetworkConnected());

    return;
}

TinyGsmClient *CarAnalyzerGsm::getHomeAssistantClient(void)
{
    return this->_homeAssistantGsmClient;
}

TinyGsmClient *CarAnalyzerGsm::getABRPClient(void)
{
    return this->_abrpGsmClient;
}

JsonObject CarAnalyzerGsm::getData(void)
{
    return (*this->_data).as<JsonObject>();
}

uint32_t CarAnalyzerGsm::getLastUpdate(void)
{
    return this->_lastUpdate;
}

void CarAnalyzerGsm::powerOff(void)
{
    this->_gsmModem->poweroff();

    esp_sleep_enable_timer_wakeup(5 * 1000000ULL);
    esp_light_sleep_start();
}