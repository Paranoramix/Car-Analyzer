#include "CarAnalyzerData.h"


String CarAnalyzerDataClass::getAllData(void) {
    DynamicJsonDocument data(2000);
    
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


String CarAnalyzerDataClass::getChangedData(void) {
    DynamicJsonDocument data(2000);
    
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
