#ifndef _CAR_ANALYZER_DATA_H
#define _CAR_ANALYZER_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>

class CarAnalyzerDataClass {
    public:
        String getAllData(void);
        String getChangedData(void);

        void setLatitude(float);
        void setLongitude(float);
        void setSpeed(float);
        void setAltitude(float);
        void setVisibleSat(int);
        void setUsedSat(int);
        void setAccuracy(float);
        void setEpoch(long);

        void setModemInfo(String);
        void setModemName(String);
        void setSignalQuality(int16_t);
        void setLocalIP(String);
        void setSimCCID(String);
        void setImei(String);
        void setImsi(String);
        void setSimStatus(int8_t);
        void setGsmOperator(String);
        void setRegistrationStatus(int8_t);

        void setChipId(uint32_t);
        void setChipRevision(uint8_t);
        void setCpuFreqMHz(uint32_t);
        void setFlashChipSize(uint32_t);
        void setFlashChipSpeed(uint32_t);
        void setFreeHeapSize(uint32_t);
        void setFreePsramSize(uint32_t);
        void setHeapSize(uint32_t);
        void setPsramSize(uint32_t);
        void setSketchMD5(String);
        void setSketchSize(uint32_t);
        
    private:
        // GPS informations
        byte        gpsChanges          = 0;
        float       latitude            = 0;    // Can change 0000 0001  0x00
        float       longitude           = 0;    // Can change 0000 0010  0x01
        float       speed               = 0;    // Can change 0000 0100  0x04
        float       altitude            = 0;    // Can change 0000 1000  0x08
        int         visibleSat          = 0;    // Can change 0001 0000  0x10
        int         usedSat             = 0;    // Can change 0010 0000  0x20
        float       accuracy            = 0;    // Can change 0100 0000  0x40
        long        epoch               = 0;    // Can change 1000 0000  0x80

        // GSM informations
        byte        gsmChanges          = 0;
        String      modemInfo           = "";   // NO CHANGE 
        String      modemName           = "";   // NO CHANGE
        int16_t     signalQuality       = 0;    // Can change 0000 0001
        String      localIP             = "";   // Can change 0000 0010
        String      simCCID             = "";   // NO CHANGE
        String      imei                = "";   // NO CHANGE
        String      imsi                = "";   // NO CHANGE
        int8_t      simStatus           = 0;    // Can change 0000 0100
        String      gsmOperator         = "";   // Can change 0000 1000
        int8_t      registrationStatus  = 0;    // Can change 0001 0000

        // Chip Informations
        byte        chipChanges         = 0;
        uint32_t    chipId              = 0;    // NO CHANGE
        uint8_t     chipRevision        = 0;    // NO CHANGE
        uint32_t    cpuFreqMHz          = 0;    // NO CHANGE
        uint32_t    flashChipSize       = 0;    // NO CHANGE
        uint32_t    flashChipSpeed      = 0;    // NO CHANGE
        uint32_t    freeHeapSize        = 0;    // Can change 0000 0001
        uint32_t    freePsramSize       = 0;    // Can change 0000 0010
        uint32_t    heapSize            = 0;    // NO CHANGE
        uint32_t    psramSize           = 0;    // NO CHANGE
        String      sketchMD5           = "";   // NO CHANGE
        uint32_t    sketchSize          = 0;    // NO CHANGE


};


#endif