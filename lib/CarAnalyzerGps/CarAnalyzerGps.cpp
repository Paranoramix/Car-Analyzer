#include "CarAnalyzerGps.h"

CarAnalyzerGps::CarAnalyzerGps(void)
{
    psramInit();

    this->_gsmSerial = (SoftwareSerial *)ps_malloc(sizeof(SoftwareSerial));
    this->_gsmSerial = new SoftwareSerial();

    this->_gsmModem = (TinyGsm *)ps_malloc(sizeof(TinyGsm));

    this->_data = new SpiRamJsonDocument(_GPS_HISTORIC_MAX_SIZE);
}

/**
 * @brief Initialize component.
 * 
 * @param rxPin 
 * @param txPin 
 */
void CarAnalyzerGps::begin(uint8_t rxPin, uint8_t txPin)
{

    this->_gsmTxPin = txPin;
    this->_gsmRxPin = rxPin;

    this->_gsmSerial->begin(9600, SWSERIAL_8N1, this->_gsmRxPin, this->_gsmTxPin, false);

    this->_gsmModem = new TinyGsm(*this->_gsmSerial);
}

/**
 * @brief Connect to GPS.
 * 
 * @param pin 
 * @param apn 
 * @param user 
 * @param password 
 * @return true 
 * @return false 
 */
bool CarAnalyzerGps::connect(void)
{
    return this->_gsmModem->enableGPS();
}

void CarAnalyzerGps::printGPS(void)
{  
    float latitude = 0;
    float longitude = 0;
    float speed = 0;
    float altitude = 0;
    int visibleSat = 0;
    int usedSat = 0;
    float accuracy = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    ESP32Time rtc;


    if (this->_gsmModem->getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
        rtc.setTime(second, minute, hour, day, month, year);

        SpiRamJsonDocument obj(300);
        
        obj["lat"] = latitude;
        obj["lon"] = longitude;
        obj["speed"] = speed;
        obj["alt"] = altitude;
        obj["vSat"] = visibleSat;
        obj["uSat"] = usedSat;
        obj["acc"] = accuracy;
        obj["epoch"] = rtc.getEpoch();

        //serializeJson(obj, Serial);

        if (measureJson(obj) + measureJson(*this->_data) >= _GPS_HISTORIC_MAX_SIZE) {
            this->_data->remove(0);
            this->_data->garbageCollect();
        }

        this->_data->add(obj);
    } else {
       CarAnalyzerLog_d("%s", this->_gsmModem->getGPSraw()); 
    }

    serializeJsonPretty((this->_data->getElement(0)), Serial);
    serializeJsonPretty((this->_data->getElement(this->_data->size() - 1)), Serial);
    CarAnalyzerLog_d("elements: %d, sizeof %lu", this->_data->size(), measureJson(*this->_data));
}