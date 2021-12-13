#include "CarAnalyzerData.h"


struct SpiRamAllocator {
        void* allocate(size_t size) {
                return ps_malloc(size);

        }
        void deallocate(void* pointer) {
                free(pointer);
        }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;


String CarAnalyzerDataClass::getAllData(void) {
    SpiRamJsonDocument data(100000);
    
    JsonObject gps = data.createNestedObject("gps");
    gps["latitude"]             = this->latitude;
    gps["longitude"]            = this->longitude;
    gps["speed"]                = this->speed;
    gps["altitude"]             = this->altitude;
    gps["visibleSat"]           = this->visibleSat;
    gps["usedSat"]              = this->usedSat;
    gps["accuracy"]             = this->accuracy;
    gps["epoch"]                = this->epoch;

    JsonObject gsm = data.createNestedObject("gsm");
    gsm["modemName"]            = this->modemName;
    gsm["modemInfo"]            = this->modemInfo;
    gsm["simStatus"]            = this->simStatus;
    gsm["localIP"]              = this->localIP;
    gsm["ccid"]                 = this->simCCID;
    gsm["imei"]                 = this->imei;
    gsm["imsi"]                 = this->imsi;
    gsm["gsmOperator"]          = this->gsmOperator;
    gsm["signalQuality"]        = this->signalQuality;
    gsm["registrationStatus"]   = this->registrationStatus;

    JsonObject chip = data.createNestedObject("chip");
    chip["chipId"]           = this->chipId;
    chip["chipRevision"]     = this->chipRevision;
    chip["cpuFreqMHz"]       = this->cpuFreqMHz;
    chip["flashChipSize"]    = this->flashChipSize;
    chip["flashChipSpeed"]   = this->flashChipSpeed;
    chip["freeHeap"]         = this->freeHeapSize;
    chip["freePsram"]        = this->freePsramSize;
    chip["sketchMD5"]        = this->sketchMD5;
    chip["sketchSize"]       = this->sketchSize;

    String d;
    serializeJson(data, d);

    return d;
}


String CarAnalyzerDataClass::getAllDataCsv(void) {
    String data = "";
    data += millis();
    data += ";";

    if (this->epoch == 0) {
        data += ";;;;;;;;";
    } else {
        data += this->epoch;
        data += ";";
        data += String(this->latitude, 6);
        data += ";";
        data += String(this->longitude, 6);
        data += ";";
        data += this->speed;
        data += ";";
        data += this->altitude;
        data += ";";
        data += this->visibleSat;
        data += ";";
        data += this->usedSat;
        data += ";";
        data += this->accuracy;
        data += ";";
    }
    data += this->modemName;
    data += ";";
    data += this->modemInfo;
    data += ";";
    data += this->simStatus;
    data += ";";
    data += this->localIP;
    data += ";";
    data += this->simCCID;
    data += ";";
    data += this->imei;
    data += ";";
    data += this->imsi;
    data += ";";
    data += this->gsmOperator;
    data += ";";
    data += this->signalQuality;
    data += ";";
    data += this->registrationStatus;
    data += ";";
    data += this->chipId;
    data += ";";
    data += this->chipRevision;
    data += ";";
    data += this->cpuFreqMHz;
    data += ";";
    data += this->flashChipSize;
    data += ";";
    data += this->flashChipSpeed;
    data += ";";
    data += this->freeHeapSize;
    data += ";";
    data += this->freePsramSize;
    data += ";";
    data += this->sketchMD5;
    data += ";";
    data += this->sketchSize;

    return data;
}


String CarAnalyzerDataClass::getChangedData(void) {
    SpiRamJsonDocument data(100000);
    
    JsonObject gps = data.createNestedObject("gps");
    if (this->gpsChanges & 0b00000001) {
        gps["latitude"]             = this->latitude;
    }

    if (this->gpsChanges & 0b00000010) {
        gps["longitude"]            = this->longitude;
    }

    if (this->gpsChanges & 0b00000100) {
        gps["speed"]                = this->speed;
    }

    if (this->gpsChanges & 0b00001000) {
        gps["altitude"]             = this->altitude;
    }

    if (this->gpsChanges & 0b00010000) {
        gps["visibleSat"]           = this->visibleSat;
    }

    if (this->gpsChanges & 0b00100000) {
        gps["usedSat"]              = this->usedSat;
    }

    if (this->gpsChanges & 0b01000000) {
        gps["accuracy"]             = this->accuracy;
    }

    if (this->gpsChanges & 0b10000000) {
        gps["epoch"]                = this->epoch;
    }

    this->gpsChanges = 0b00000000;

    JsonObject gsm = data.createNestedObject("gsm");
    if (this->gsmChanges & 0b00000001) {
        gsm["signalQuality"]        = this->signalQuality;
    }
    if (this->gsmChanges & 0b00000010) {
        gsm["localIP"]              = this->localIP;
    }
    if (this->gsmChanges & 0b00000100) {
        gsm["simStatus"]            = this->simStatus;
    }
    if (this->gsmChanges & 0b00001000) {
        gsm["gsmOperator"]          = this->gsmOperator;
    }
    if (this->gsmChanges & 0b00010000) {
        gsm["registrationStatus"]   = this->registrationStatus;
    }
    
    this->gsmChanges = 0b00000000;
 

    JsonObject chip = data.createNestedObject("chip");
    if (this->chipChanges & 0b00000001) {
        chip["FreeHeap"]         = this->freeHeapSize;
    }
    if (this->chipChanges & 0b00000010) {
        chip["FreePsram"]        = this->freePsramSize;
    }

    this->chipChanges = 0b00000000;

    String d;
    serializeJson(data, d);

    return d;
}

void CarAnalyzerDataClass::setLatitude(float latitude) {
    if (this->latitude != latitude) {
        this->latitude = latitude;
        this->gpsChanges += 0b00000001;
    }
}

void CarAnalyzerDataClass::setLongitude(float longitude) {
    if (this->longitude != longitude) {
        this->longitude = longitude;
        this->gpsChanges += 0b00000010;
    }
}

void CarAnalyzerDataClass::setSpeed(float speed) {
    if (this->speed != speed) {
        this->speed = speed;
        this->gpsChanges += 0b00000100;
    }
}

void CarAnalyzerDataClass::setAltitude(float altitude) {
    if (this->altitude != altitude) {
        this->altitude = altitude;
        this->gpsChanges += 0b00001000;
    }
}

void CarAnalyzerDataClass::setVisibleSat(int visibleSat) {
    if (this->visibleSat != visibleSat) {
        this->visibleSat = visibleSat;
        this->gpsChanges += 0b00010000;
    }
}

void CarAnalyzerDataClass::setUsedSat(int usedSat) {
    if (this->usedSat != usedSat) {
        this->usedSat = usedSat;
        this->gpsChanges += 0b00100000;
    }
}

void CarAnalyzerDataClass::setAccuracy(float accuracy) {
    if (this->accuracy != accuracy) {
        this->accuracy = accuracy;
        this->gpsChanges += 0b01000000;
    }
}

void CarAnalyzerDataClass::setEpoch(long epoch) {
    if (this->epoch != epoch) {
        this->epoch = epoch;
        this->gpsChanges += 0b10000000;
    }
}


void CarAnalyzerDataClass::setModemInfo(String modemInfo) {
    this->modemInfo = modemInfo;
}

void CarAnalyzerDataClass::setModemName(String modemName) {
    this->modemName = modemName;
}

void CarAnalyzerDataClass::setSignalQuality(int16_t signalQuality) {
    if (this->signalQuality != signalQuality) {
        this->signalQuality = signalQuality;
        this->gsmChanges += 0b00000001;
    }
}

void CarAnalyzerDataClass::setLocalIP(String localIP) {
    if (this->localIP != localIP) {
        this->localIP = localIP;
        this->gsmChanges += 0b00000010;
    }
}

void CarAnalyzerDataClass::setSimCCID(String simCCID) {
    this->simCCID = simCCID;
}

void CarAnalyzerDataClass::setImei(String imei) {
    this->imei = imei;
}

void CarAnalyzerDataClass::setImsi(String imsi) {
    this->imsi = imsi;
}

void CarAnalyzerDataClass::setSimStatus(int8_t simStatus) {
    if (this->simStatus != simStatus) {
        this->simStatus = simStatus;
        this->gsmChanges += 0b00000100;
    }
}

void CarAnalyzerDataClass::setGsmOperator(String gsmOperator) {
    if (this->gsmOperator != gsmOperator) {
        this->gsmOperator = gsmOperator;
        this->gsmChanges += 0b00001000;
    }
}

void CarAnalyzerDataClass::setRegistrationStatus(int8_t registrationStatus) {
    if (this->registrationStatus != registrationStatus) {
        this->registrationStatus = registrationStatus;
        this->gsmChanges += 0b00010000;
    }
}


void CarAnalyzerDataClass::setChipId(uint32_t chipId) {
    this->chipId = chipId;
}

void CarAnalyzerDataClass::setChipRevision(uint8_t chipRevision) {
    this->chipRevision = chipRevision;
}

void CarAnalyzerDataClass::setCpuFreqMHz(uint32_t cpuFreqMHz) {
    this->cpuFreqMHz = cpuFreqMHz;
}

void CarAnalyzerDataClass::setFlashChipSize(uint32_t flashChipSize) {
    this->flashChipSize = flashChipSize;
}

void CarAnalyzerDataClass::setFlashChipSpeed(uint32_t flashChipSpeed) {
    this->flashChipSpeed = flashChipSpeed;
}

void CarAnalyzerDataClass::setFreeHeapSize(uint32_t freeHeapSize) {
    if (this->freeHeapSize != freeHeapSize) {
        this->freeHeapSize = freeHeapSize;
        this->chipChanges += 0b00000001;
    }
}

void CarAnalyzerDataClass::setFreePsramSize(uint32_t freePsramSize) {
    if (this->freePsramSize != freePsramSize) {
        this->freePsramSize = freePsramSize;
        this->chipChanges += 0b00000010;
    }
}

void CarAnalyzerDataClass::setHeapSize(uint32_t heapSize) {
    this->heapSize = heapSize;
}

void CarAnalyzerDataClass::setPsramSize(uint32_t psramSize) {
    this->psramSize = psramSize;
}

void CarAnalyzerDataClass::setSketchMD5(String sketchMD5) {
    this->sketchMD5 = sketchMD5;
}

void CarAnalyzerDataClass::setSketchSize(uint32_t sketchSize) {
    this->sketchSize = sketchSize;
}

void CarAnalyzerDataClass::setStateOfCharge(uint8_t stateOfCharge) {
    this->stateOfCharge = stateOfCharge;
}

void CarAnalyzerDataClass::setRealStateOfCharge(uint8_t realStateOfCharge) {
    this->realStateOfCharge = realStateOfCharge;
}

void CarAnalyzerDataClass::setStateOfHealth(uint8_t stateOfHealth) {
    this->stateOfHealth = stateOfHealth;
}

void CarAnalyzerDataClass::setInletTemperature(uint8_t inletTemperature) {
    this->inletTemperature = inletTemperature;
}

void CarAnalyzerDataClass::setBatteryTemperature(uint8_t batteryTemperature) {
    this->batteryTemperature = batteryTemperature;
}

void CarAnalyzerDataClass::setBatteryCurrent(uint8_t batteryCurrent) {
    this->batteryCurrent = batteryCurrent;
}

void CarAnalyzerDataClass::setAuxiliaryBattery(uint8_t auxiliaryBattery) {
    this->auxiliaryBattery = auxiliaryBattery;
}

void CarAnalyzerDataClass::setCarSpeed(uint8_t carSpeed) {
    this->carSpeed = carSpeed;
}

void CarAnalyzerDataClass::setIndoorTemperature(uint8_t indoorTemperature) {
    this->indoorTemperature = indoorTemperature;
}

void CarAnalyzerDataClass::setOutdoorTemperature(uint8_t outdoorTemperature) {
    this->outdoorTemperature = outdoorTemperature;
}

void CarAnalyzerDataClass::setOdometer(uint32_t odometer) {
    this->odometer = odometer;
}


float CarAnalyzerDataClass::getLatitude(void) {
    return this->latitude;
}


float CarAnalyzerDataClass::getLongitude(void) {
    return this->longitude;
}


float CarAnalyzerDataClass::getSpeed(void) {
    return this->speed;
}


float CarAnalyzerDataClass::getAltitude(void) {
    return this->altitude;
}


int CarAnalyzerDataClass::getVisibleSat(void) {
    return this->visibleSat;
}


int CarAnalyzerDataClass::getUsedSat(void) {
    return this->usedSat;
}


float CarAnalyzerDataClass::getAccuracy(void) {
    return this->accuracy;
}


long CarAnalyzerDataClass::getEpoch(void) {
    return this->epoch;
}



String CarAnalyzerDataClass::getModemInfo(void) {
    return this->modemInfo;
}


String CarAnalyzerDataClass::getModemName(void) {
    return this->modemName;
}


int16_t CarAnalyzerDataClass::getSignalQuality(void) {
    return this->signalQuality;
}


String CarAnalyzerDataClass::getLocalIP(void) {
    return this->localIP;
}


String CarAnalyzerDataClass::getSimCCID(void) {
    return this->simCCID;
}


String CarAnalyzerDataClass::getImei(void) {
    return this->imei;
}


String CarAnalyzerDataClass::getImsi(void) {
    return this->imsi;
}


int8_t CarAnalyzerDataClass::getSimStatus(void) {
    return this->simStatus;
}


String CarAnalyzerDataClass::getGsmOperator(void) {
    return this->gsmOperator;
}


int8_t CarAnalyzerDataClass::getRegistrationStatus(void) {
    return this->registrationStatus;
}

uint32_t CarAnalyzerDataClass::getChipId(void) {
    return this->chipId;
}


uint8_t CarAnalyzerDataClass::getChipRevision(void) {
    return this->chipRevision;
}


uint32_t CarAnalyzerDataClass::getCpuFreqMHz(void) {
    return this->cpuFreqMHz;
}


uint32_t CarAnalyzerDataClass::getFlashChipSize(void) {
    return this->flashChipSize;
}


uint32_t CarAnalyzerDataClass::getFlashChipSpeed(void) {
    return this->flashChipSpeed;
}


uint32_t CarAnalyzerDataClass::getFreeHeapSize(void) {
    return this->freeHeapSize;
}


uint32_t CarAnalyzerDataClass::getFreePsramSize(void) {
    return this->freePsramSize;
}


uint32_t CarAnalyzerDataClass::getHeapSize(void) {
    return this->heapSize;
}


uint32_t CarAnalyzerDataClass::getPsramSize(void) {
    return this->psramSize;
}


String CarAnalyzerDataClass::getSketchMD5(void) {
    return this->sketchMD5;
}


uint32_t CarAnalyzerDataClass::getSketchSize(void) {
    return this->sketchSize;
}

uint8_t CarAnalyzerDataClass::getStateOfCharge(void) {
    return this->stateOfCharge;
}

uint8_t CarAnalyzerDataClass::getRealStateOfCharge(void) {
    return this->realStateOfCharge;
}

uint8_t CarAnalyzerDataClass::getStateOfHealth(void) {
    return this->stateOfHealth;
}

uint8_t CarAnalyzerDataClass::getInletTemperature(void) {
    return this->inletTemperature;
}

uint8_t CarAnalyzerDataClass::getBatteryTemperature(void) {
    return this->batteryTemperature;
}

uint8_t CarAnalyzerDataClass::getBatteryCurrent(void) {
    return this->batteryCurrent;
}

uint8_t CarAnalyzerDataClass::getAuxiliaryBattery(void) {
    return this->auxiliaryBattery;
}

uint8_t CarAnalyzerDataClass::getCarSpeed(void) {
    return this->carSpeed;
}

uint8_t CarAnalyzerDataClass::getIndoorTemperature(void) {
    return this->indoorTemperature;
}

uint8_t CarAnalyzerDataClass::getOutdoorTemperature(void) {
    return this->outdoorTemperature;
}

uint32_t CarAnalyzerDataClass::getOdometer(void) {
    return this->odometer;
}

