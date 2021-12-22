#include <Arduino.h>

#ifndef _CAR_ANALYZER_DATA_H
#define _CAR_ANALYZER_DATA_H

struct Config {
    bool gsmUse;
    char gsmSimPin[9];
    char gsmApn[50];
    char gsmUser[50];
    char gsmPassword[50];
    bool gpsUse;
    uint8_t gpsPrecision;
    bool abrpUse;
    uint32_t abrpUpdate;
    char abrpApiKey[40];
    char abrpToken[40];
    char abrpCarModel[40];
    bool wifiUse;
    char wifiMode[4];
    char wifiSsid[64];
    char wifiPassword[64];
    bool bluetoothUse;
    char bluetoothName[40];
    char bluetoothPin[9];
    char bluetoothAddress[40];
    uint32_t carDataUpdateOnUse;
    uint32_t carDataUpdateOnSleep;
    uint8_t macAddress[6];
    bool logToFile;
};

struct GsmData {
    char modemInfo[40];
    char modemName[40];
    uint8_t signalQuality;
    char localIP[20];
    char simCCID[40];
    char imei[40];
    char imsi[40];
    uint8_t simStatus;
    char gsmOperator[40];
    uint8_t registrationStatus;
    bool isConnected;
};

struct GpsData {
    bool valid;
    float latitude;
    float longitude;
    float speed;
    float altitude;
    int visibleSat;
    int usedSat;
    float accuracy;
    long epoch;
    uint32_t lastUpdate;
};

struct BluetoothData {
    bool isConnected;
    uint8_t currentFrame;
};

struct CarData {
    float cecValue;
    float cedValue;
    float auxBattVoltage;
    float auxBattCurrent;
    float auxBattStateOfCharge;
    float battPower;
    float battTemperature;
    float battInletTemperature;
    float maxCellVolt;
    float minCellVolt;
    float maxCellVoltNumber;
    float minCellVoltNumber;
    float bmsRelay;
    float isCharging;
    float battFanSpeed;
    float coolTemperature;
    float heatTemperature1;
    float heatTemperature2;
    float trip1Charged;
    float trip1Discharged;
    float indoorTemperature;
    float outdoorTemperature;
    float rearLeftTyrePressure;
    float rearRightTyrePressure;
    float frontLeftTyrePressure;
    float frontRightTyrePressure;
    float acChargeCurrent;
    float speed;
    float voltageDifference;
    float temperatureAverage;
    float humidityAverage;
    float trip1EnergyUsed;
    float trip1EnergyAverage;

    float stateOfChargePercent;
    float stateOfChargeRealPercent;
    float stateOfHealthPercent;
    float batteryVoltage;
    float batteryCurrent;
    float batteryTemperature1;
    float batteryTemperature2;
    float batteryTemperature3;
    float batteryTemperature4;
    float batteryTemperature5;
    float batteryMaxTemperature;
    float batteryMinTemperature;
    float rearLeftTyreTemperature;
    float rearRightTyreTemperature;
    float frontLeftTyreTemperature;
    float frontRightTyreTemperature;
    float trip1Odometer;

    float operatingTime;
    float odometer;

    bool carData1Valid;
    bool carData2Valid;
    bool carData3Valid;
    bool carData4Valid;
    bool carData5Valid;

    uint32_t carData1LastUpdate;
    uint32_t carData2LastUpdate;
    uint32_t carData3LastUpdate;
    uint32_t carData4LastUpdate;
    uint32_t carData5LastUpdate;

    uint32_t carUpdate;
};

struct SDCardData {
    bool isMounted;
    uint8_t cardType;
    uint64_t cardSize;

    bool isConnected;
};

struct ChipData {
    uint32_t freeHeap;
    uint32_t freePsram;
};

struct Timers {
    long gsmTimer;
    long abrpTimer;
    long gpsTimer;
    long bluetoothTimer;
    long chipTimer;
    long carTimer;
};

Config* config;
GsmData* gsmData;
GpsData* gpsData;
BluetoothData* bluetoothData;
CarData* carData;
SDCardData* sdCardData;
ChipData* chipData;

Timers* updateTimers;

#endif