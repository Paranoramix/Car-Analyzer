#include "CarAnalyzerGps.h"

#ifdef DUMP_AT_COMMANDS
StreamDebugger debuggerGps(Serial1, Serial);
#endif

CarAnalyzerGps::CarAnalyzerGps(void)
{
    psramInit();

    this->_gsmModem = (TinyGsm *)ps_malloc(sizeof(TinyGsm));

    this->_data = new SpiRamJsonDocument(2000);

    this->_lastUpdate = millis();
    this->_precision = 4;

    this->_previousLatitude = 0;
    this->_previousLongitude = 0;
    this->_previousAltitude = 0;
}

void CarAnalyzerGps::begin(uint32_t baud, uint8_t rxPin, uint8_t txPin)
{

    Serial1.begin(baud, SERIAL_8N1, rxPin, txPin);

#ifdef DUMP_AT_COMMANDS
    this->_gsmModem = new TinyGsm(debuggerGps);
#else
    this->_gsmModem = new TinyGsm(Serial1);
#endif
}

bool CarAnalyzerGps::connect(void)
{
    return this->_gsmModem->enableGPS();
}

bool CarAnalyzerGps::disconnect(void)
{
    return this->_gsmModem->disableGPS();
}

void CarAnalyzerGps::update(void)
{
    CarAnalyzerLog_v("Updating GPS informations");

    if (this->_data->containsKey("latitude__°")) {
        this->_previousLatitude = (*this->_data)["latitude__°"].as<float>();
    }

    if (this->_data->containsKey("longitude__°")) {
        this->_previousLongitude = (*this->_data)["longitude__°"].as<float>();
    }

    if (this->_data->containsKey("altitude__m")) {
        this->_previousAltitude = (*this->_data)["altitude__m"].as<float>();
    }
    

    this->_data->clear();
    
    this->_lastUpdate = millis();

    ESP32Time rtc;

    String gpsRawData = this->_gsmModem->getGPSraw();
    gpsRawData.trim();

    int mode = 0;
    int gpsSatValidNumber = 0;
    int glonassSatValidNumber = 0;
    int beidouSatValidNumber = 0;
    int latitudeDD = 0;
    float latitudeMM = 0;
    char northSouth = 'N';
    int longitudeDD = 0;
    float longitudeMM = 0;
    char eastWest = 'E';
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    float altitude;
    float speed = 0;
    float course = 0;
    float pdop = 0;
    float hdop = 0;
    float vdop = 0;

    // Lecture des données GPS brutes.
    if (gpsRawData.indexOf(",,,,,,,,,,,,,,,") > -1)
    {
        CarAnalyzerLog_d("GPS Data: %s", gpsRawData.c_str());
        return;
    }

    if (gpsRawData.indexOf(",,") > -1)
    {
        sscanf(gpsRawData.c_str(), "%d,%d,%d,%d,%2d%f,%c,%3d%f,%c,%2d%2d%2d,%2d%2d%2d.0,%f,%f,,%f,%f,%f", &mode, &gpsSatValidNumber, &glonassSatValidNumber, &beidouSatValidNumber, &latitudeDD, &latitudeMM, &northSouth, &longitudeDD, &longitudeMM, &eastWest, &day, &month, &year, &hour, &minute, &second, &altitude, &speed, &pdop, &hdop, &vdop);
    }
    else
    {
        sscanf(gpsRawData.c_str(), "%d,%d,%d,%d,%2d%f,%c,%3d%f,%c,%2d%2d%2d,%2d%2d%2d.0,%f,%f,%f,%f,%f,%f", &mode, &gpsSatValidNumber, &glonassSatValidNumber, &beidouSatValidNumber, &latitudeDD, &latitudeMM, &northSouth, &longitudeDD, &longitudeMM, &eastWest, &day, &month, &year, &hour, &minute, &second, &altitude, &speed, &course, &pdop, &hdop, &vdop);
    }

    CarAnalyzerLog_v("GPS Fix Mode: %d", mode);
    CarAnalyzerLog_v("GPS Satellite valid numbers: %d", gpsSatValidNumber);
    CarAnalyzerLog_v("Glonass Satellite valid numbers: %d", glonassSatValidNumber);
    CarAnalyzerLog_v("Beidou Satellite valid numbers: %d", beidouSatValidNumber);
    CarAnalyzerLog_v("GPS latitude: %f", latitudeDD + latitudeMM / 60);
    CarAnalyzerLog_v("GPS N/S indicator: %c", northSouth);
    CarAnalyzerLog_v("GPS longitude: %f", longitudeDD + longitudeMM / 60);
    CarAnalyzerLog_v("GPS E/W indicator: %c", eastWest);
    CarAnalyzerLog_v("GPS Year: %4d", 2000 + year);
    CarAnalyzerLog_v("GPS Month: %2d", month);
    CarAnalyzerLog_v("GPS Day: %2d", day);
    CarAnalyzerLog_v("GPS Hour: %2d", hour);
    CarAnalyzerLog_v("GPS Minute: %2d", minute);
    CarAnalyzerLog_v("GPS Second: %2d", second);
    CarAnalyzerLog_v("GPS Altitude: %f", altitude);
    CarAnalyzerLog_v("GPS Speed: %f", speed);
    CarAnalyzerLog_v("GPS Course: %f", course);
    CarAnalyzerLog_v("GPS Position Dilution of precision: %f", pdop);
    CarAnalyzerLog_v("GPS Horizontal Dilution of precision: %f", hdop);
    CarAnalyzerLog_v("GPS Vertical Dilution of precision: %f", vdop);

    CarAnalyzerLog_v("GPS Data: %s", gpsRawData.c_str());

    if (mode <= 1)
    {
        // Les données ne sont pas valides.
        return;
    }

    float latitude = latitudeDD + latitudeMM / 60;
    if (northSouth == 'S')
    {
        latitude = -1 * latitude;
    }

    float longitude = longitudeDD + longitudeMM / 60;
    if (eastWest == 'W')
    {
        longitude = -1 * longitude;
    }

    rtc.setTime(second, minute, hour, day, month, 2000 + year);

    (*this->_data)["valid"] = true;
    (*this->_data)["mode"] = String(mode);
    (*this->_data)["gpsSVs__Sat"] = String(gpsSatValidNumber);
    (*this->_data)["glonassSVs__Sat"] = String(glonassSatValidNumber);
    (*this->_data)["beiduSVs__Sat"] = String(beidouSatValidNumber);

    // La latitude a changé significativement, ou le véhicule est en déplacement.
    // On met à jour la latitude.
    if (String(latitude, 3) != String(this->_previousLatitude, 3) || speed > 5) {
        (*this->_data)["latitude__°"] = String(latitude, this->_precision).toFloat();
    }

    // La longitude a changé significativement, ou le véhicule est en déplacement.
    // On met à jour la longitude.
    if (String(longitude, 3) != String(this->_previousLongitude, 3)) {
        (*this->_data)["longitude__°"] = String(longitude, this->_precision).toFloat();
    }
    (*this->_data)["speed__knots"] = speed;
    (*this->_data)["speed__km/h"] = speed * 1.852;

    // L'altitude a changé significativement, ou le véhicule est en déplacement.
    // On met à jour l'altitude.
    if (altitude < this->_previousAltitude * 0.9 && altitude > this->_previousAltitude * 1.10) {
        (*this->_data)["altitude__m"] = altitude;
    }

    (*this->_data)["course__°"] = course;
    (*this->_data)["positionDop__dop"] = pdop;

    if (pdop < 1)
    {
        (*this->_data)["pdopCondition"] = "Ideal";
    }
    else if (pdop < 2)
    {
        (*this->_data)["pdopCondition"] = "Excellent";
    }
    else if (pdop < 5)
    {
        (*this->_data)["pdopCondition"] = "Good";
    }
    else if (pdop < 10)
    {
        (*this->_data)["pdopCondition"] = "Moderate";
    }
    else if (pdop < 20)
    {
        (*this->_data)["pdopCondition"] = "Fair";
    }
    else
    {
        (*this->_data)["pdopCondition"] = "Poor";
    }

    (*this->_data)["verticalDop__dop"] = vdop;
    if (vdop < 1)
    {
        (*this->_data)["vdopCondition"] = "Ideal";
    }
    else if (vdop < 2)
    {
        (*this->_data)["vdopCondition"] = "Excellent";
    }
    else if (vdop < 5)
    {
        (*this->_data)["vdopCondition"] = "Good";
    }
    else if (vdop < 10)
    {
        (*this->_data)["vdopCondition"] = "Moderate";
    }
    else if (vdop < 20)
    {
        (*this->_data)["vdopCondition"] = "Fair";
    }
    else
    {
        (*this->_data)["vdopCondition"] = "Poor";
    }

    (*this->_data)["horizontalDop__dop"] = hdop;
    if (hdop < 1)
    {
        (*this->_data)["hdopCondition"] = "Ideal";
    }
    else if (hdop < 2)
    {
        (*this->_data)["hdopCondition"] = "Excellent";
    }
    else if (hdop < 5)
    {
        (*this->_data)["hdopCondition"] = "Good";
    }
    else if (hdop < 10)
    {
        (*this->_data)["hdopCondition"] = "Moderate";
    }
    else if (hdop < 20)
    {
        (*this->_data)["hdopCondition"] = "Fair";
    }
    else
    {
        (*this->_data)["hdopCondition"] = "Poor";
    }

    (*this->_data)["epoch"] = String(rtc.getEpoch());

    JsonObject device_tracker = (*this->_data).createNestedObject("gpslocation");
    device_tracker["source_type"] = "gps";
    device_tracker["latitude"] = String(latitude, this->_precision).toFloat();
    device_tracker["longitude"] = String(longitude, this->_precision).toFloat();
    device_tracker["gps_accuracy"] = pdop;
}

String CarAnalyzerGps::getValue(const char *key)
{
    if (this->_data->containsKey(key))
    {
        return (*this->_data)[key].as<String>();
    }

    return String();
}

JsonObject CarAnalyzerGps::getData(void)
{
    return (*this->_data).as<JsonObject>();
}

unsigned long CarAnalyzerGps::getLastUpdate(void)
{
    return this->_lastUpdate;
}

bool CarAnalyzerGps::isValid(void)
{
    return (*this->_data)["valid"].as<bool>();
}

void CarAnalyzerGps::setPrecision(uint8_t precision) {
    this->_precision = precision;
}