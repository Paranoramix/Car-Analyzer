#ifndef _CAR_ANALYZER_DATA_H
#define _CAR_ANALYZER_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>

class CarAnalyzerDataClass {
    public:
        String getAllData(void);
        String getAllDataCsv(void);
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

        void setStateOfCharge(uint8_t);
        void setRealStateOfCharge(uint8_t);
        void setStateOfHealth(uint8_t);
        void setInletTemperature(uint8_t);
        void setBatteryTemperature(uint8_t);
        void setBatteryCurrent(uint8_t);
        void setAuxiliaryBattery(uint8_t);
        void setCarSpeed(uint8_t);
        void setIndoorTemperature(uint8_t);
        void setOutdoorTemperature(uint8_t);
        void setOdometer(uint32_t);

        float getLatitude(void);
        float getLongitude(void);
        float getSpeed(void);
        float getAltitude(void);
        int getVisibleSat(void);
        int getUsedSat(void);
        float getAccuracy(void);
        long getEpoch(void);

        String getModemInfo(void);
        String getModemName(void);
        int16_t getSignalQuality(void);
        String getLocalIP(void);
        String getSimCCID(void);
        String getImei(void);
        String getImsi(void);
        int8_t getSimStatus(void);
        String getGsmOperator(void);
        int8_t getRegistrationStatus(void);

        uint32_t getChipId(void);
        uint8_t getChipRevision(void);
        uint32_t getCpuFreqMHz(void);
        uint32_t getFlashChipSize(void);
        uint32_t getFlashChipSpeed(void);
        uint32_t getFreeHeapSize(void);
        uint32_t getFreePsramSize(void);
        uint32_t getHeapSize(void);
        uint32_t getPsramSize(void);
        String getSketchMD5(void);
        uint32_t getSketchSize(void);

        uint8_t getStateOfCharge(void);
        uint8_t getRealStateOfCharge(void);
        uint8_t getStateOfHealth(void);
        uint8_t getInletTemperature(void);
        uint8_t getBatteryTemperature(void);
        uint8_t getBatteryCurrent(void);
        uint8_t getAuxiliaryBattery(void);
        uint8_t getCarSpeed(void);
        uint8_t getIndoorTemperature(void);
        uint8_t getOutdoorTemperature(void);
        uint32_t getOdometer(void);
        
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

        // Car Informations
        byte        carChange1          = 0;
        byte        carChange2          = 0;
        uint8_t     stateOfCharge       = 0;
        uint8_t     realStateOfCharge   = 0;
        uint8_t     stateOfHealth       = 0;
        uint8_t     inletTemperature    = 0;
        uint8_t     batteryTemperature  = 0;
        uint8_t     batteryCurrent      = 0;
        uint8_t     auxiliaryBattery    = 0;
        uint8_t     carSpeed            = 0;
        uint8_t     indoorTemperature   = 0;
        uint8_t     outdoorTemperature  = 0;
        uint32_t    odometer            = 0;
};


#endif