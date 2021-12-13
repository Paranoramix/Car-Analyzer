#define TINY_GSM_MODEM_SIM7000


#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <BluetoothSerial.h>
#include <CircularBuffer.h>
#include <ELMduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Time.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGSMClient.h>
#include <WiFi.h>

#include "CarAnalyzerLog.h"

#define TINY_GSM_USE_GPRS true
#define GSM_MODEM_RX 27
#define GSM_MODEM_TX 25


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
    char bluetoothAddress[40]; //dc:0d:30:a4:30
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
};

struct BluetoothData {
    bool isConnected;
};

struct CarData {
    float cecValue;
    float cedValue;
    float auxBattVolt;
    float auxBattCurrent;
    byte auxBattStateOfCharge;
    float battPower;
    float battTemperature;
    float battInTemperature;
    float maxCellVolt;
    float minCellVolt;
    byte maxCellVoltNumber;
    byte minCellVoltNumber;
    byte bmsRelay;
    byte isCharging;
    byte battFanSpeed;
    int coolTemperature;
    int heatTemperature1;
    int heatTemperature2;
    float trip1Charged;
    float trip1Discharged;
    float indoorTemperature;
    float outDoorTemperature;
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

    int stateOfChargePercent;
    int stateOfChargeRealPercrent;
    int stateOfHealthPercent;
    float batteryVoltage;
    float batteryCurrent;
    int batteryTemperature1;
    int batteryTemperature2;
    int batteryTemperature3;
    int batteryTemperature4;
    int batteryMaxTemperature;
    int batteryMinTemperature;
    int rearLeftTyreTemperature;
    int rearRightTyreTemperature;
    int frontLeftTyreTemperature;
    int frontRightTyreTemperature;
    int trip1Odometer;
};

struct Timers {
    long gsmTimer;
    long abrpTimer;
    long gpsTimer;
    long bluetoothTimer;
    long memoryUsageTimer;
};
/*
struct HistoricalData {
    CircularBuffer<uint32_t, 86400>* heapMemory;
    CircularBuffer<uint32_t, 86400>* psramMemory;
};
*/
Config* config;
GsmData* gsmData;
GpsData* gpsData;
BluetoothData* bluetoothData;
CarData* carData;

//HistoricalData* historicalData24h;

Timers* updateTimers;

HttpClient* abrpHttpClient;
TinyGsm* gsmModem;
AsyncWebServer* server;
BluetoothSerial* btSerial;
ELM327* elm;

struct SpiRamAllocator {
        void* allocate(size_t size) {
                return ps_malloc(size);

        }
        void deallocate(void* pointer) {
                free(pointer);
        }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

void loadConfiguration(const char *filename, Config &config) {
    if (!SD.begin()) {
        CarAnalyzerLog_e("SD.begin() failed. Check: ");
        CarAnalyzerLog_e("  card insertion,");
        CarAnalyzerLog_e("  SD shield I/O pins and chip select,");
        CarAnalyzerLog_e("  card formatting.");
    }

    File file = SD.open(filename);
    SpiRamJsonDocument doc(file.size() * 2);
    DeserializationError error = deserializeJson(doc, file);

    if (error) {
        CarAnalyzerLog_e("Failed to read configuration file [%s]. Using default configuration", filename);
    }

    config.gsmUse = doc["gsmUse"] | true;
    CarAnalyzerLog_v("gsmUSe: %d", config.gsmUse);

    strlcpy(config.gsmSimPin, doc["gsmSimPin"] | "0000",sizeof(config.gsmSimPin));
    CarAnalyzerLog_v("gsmSimPin: %s", config.gsmSimPin);

    strlcpy(config.gsmApn, doc["gsmApn"] | "orange.fr",sizeof(config.gsmApn));
    CarAnalyzerLog_v("gsmApn: %s", config.gsmApn);

    strlcpy(config.gsmUser, doc["gsmUser"] | "orange",sizeof(config.gsmUser));
    CarAnalyzerLog_v("gsmUser: %s", config.gsmUser);

    strlcpy(config.gsmPassword, doc["gsmPassword"] | "orange",sizeof(config.gsmPassword));
    CarAnalyzerLog_v("gsmPassword: %s", config.gsmPassword);

    config.gpsUse = doc["gpsUse"] | true;
    CarAnalyzerLog_v("gpsUse: %d", config.gpsUse);

    config.gpsPrecision = doc["gpsPrecision"] | 8;
    CarAnalyzerLog_v("gpsPrecision: %d", config.gpsPrecision);

    config.abrpUse = doc["abrpUse"] | false;
    CarAnalyzerLog_v("abrpUse: %d", config.abrpUse);

    config.abrpUpdate = doc["abrpUpdate"] | 10000;
    CarAnalyzerLog_v("abrpUpdate: %u", config.abrpUpdate);

    strlcpy(config.abrpApiKey, doc["abrpApiKey"] | "32b2162f-9599-4647-8139-66e9f9528370",sizeof(config.abrpApiKey));
    CarAnalyzerLog_v("abrpApiKey: %s", config.abrpApiKey);

    strlcpy(config.abrpToken, doc["abrpToken"] | "68131ce8-a73d-4420-90f9-10a93ea30208",sizeof(config.abrpToken));
    CarAnalyzerLog_v("abrpToken: %s", config.abrpToken);

    strlcpy(config.abrpCarModel, doc["abrpCarModel"] | "hyundai:ioniq5:22:74",sizeof(config.abrpCarModel));
    CarAnalyzerLog_v("abrpCarModel: %s", config.abrpCarModel);


    config.wifiUse = doc["wifiUse"] | true;
    CarAnalyzerLog_v("wifiUse: %d", config.wifiUse);

    strlcpy(config.wifiMode, doc["wifiMode"] | "AP",sizeof(config.wifiMode));
    CarAnalyzerLog_v("wifiMode: %s", config.wifiMode);

    strlcpy(config.wifiSsid, doc["wifiSsid"] | "Car-Analyzer",sizeof(config.wifiSsid));
    CarAnalyzerLog_v("wifiSsid: %s", config.wifiSsid);

    strlcpy(config.wifiPassword, doc["wifiPassword"] | "123456789",sizeof(config.wifiPassword));
    CarAnalyzerLog_v("wifiPassword: %s", config.wifiPassword);


    config.bluetoothUse = doc["bluetoothUse"] | true;
    CarAnalyzerLog_v("bluetoothUse: %d", config.bluetoothUse);

    strlcpy(config.bluetoothName, doc["bluetoothName"] | "CarAnalyzer",sizeof(config.bluetoothName));
    CarAnalyzerLog_v("bluetoothName: %s", config.bluetoothName);

    strlcpy(config.bluetoothPin, doc["bluetoothPin"] | "1234",sizeof(config.bluetoothPin));
    CarAnalyzerLog_v("bluetoothPin: %s", config.bluetoothPin);

    strlcpy(config.bluetoothAddress, doc["bluetoothAddress"] | "OBDII",sizeof(config.bluetoothAddress));
    CarAnalyzerLog_v("bluetoothAddress: %s", config.bluetoothAddress);
    sscanf(config.bluetoothAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &config.macAddress[0], &config.macAddress[1], &config.macAddress[2],&config.macAddress[3], &config.macAddress[4], &config.macAddress[5]);

    config.logToFile = doc["logToFile"] | true;
    CarAnalyzerLog_v("logToFile: %d", config.logToFile);

    file.close();

    SD.end();
}

// Saves the configuration to a file
void saveConfiguration(const char *filename, Config &config) {
    if (!SD.begin()) {
        CarAnalyzerLog_e("SD.begin() failed. Check: ");
        CarAnalyzerLog_e("  card insertion,");
        CarAnalyzerLog_e("  SD shield I/O pins and chip select,");
        CarAnalyzerLog_e("  card formatting.");
    }

    // Delete existing file, otherwise the configuration is appended to the file
    File myFileIn = SD.open(filename, FILE_READ);
    File myFileOut = SD.open(String(filename) + ".bkp", FILE_WRITE);
    while (myFileIn.available()) {
        myFileOut.write(myFileIn.read());
    }

    myFileIn.close();
    myFileOut.close();

    // Open file for writing
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        CarAnalyzerLog_e("Failed to create file");
        return;
    }

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    SpiRamJsonDocument doc(1000);

    // Set the values in the document
    doc["gsmUse"] = config.gsmUse;
    doc["gsmSimPin"] = config.gsmSimPin;
    doc["gsmApn"] = config.gsmApn;
    doc["gsmUser"] = config.gsmUser;
    doc["gsmPassword"] = config.gsmPassword;
    doc["gpsUse"] = config.gpsUse;
    doc["gpsPrecision"] = config.gpsPrecision;
    doc["abrpUse"] = config.abrpUse;
    doc["abrpUpdate"] = config.abrpUpdate;
    doc["abrpApiKey"] = config.abrpApiKey;
    doc["abrpToken"] = config.abrpToken;
    doc["abrpCarModel"] = config.abrpCarModel;
    doc["wifiUse"] = config.wifiUse;
    doc["wifiMode"] = config.wifiMode;
    doc["wifiSsid"] = config.wifiSsid;
    doc["wifiPassword"] = config.wifiPassword;
    doc["bluetoothUse"] = config.bluetoothUse;
    doc["bluetoothPin"] = config.bluetoothPin;
    doc["bluetoothAddress"] = config.bluetoothAddress;
    doc["logToFile"] = config.logToFile;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
        CarAnalyzerLog_e("Failed to write to file");
    }

    // Close the file
    file.close();

    SD.end();
}

void writeData(const char *filename, const char* data) {
    if (!SD.begin()) {
        CarAnalyzerLog_e("SD.begin() failed. Check: ");
        CarAnalyzerLog_e("  card insertion,");
        CarAnalyzerLog_e("  SD shield I/O pins and chip select,");
        CarAnalyzerLog_e("  card formatting.");
    }

    File file = SD.open(filename, FILE_WRITE);
    file.print(data + '\n');
    file.close();

    SD.end();
}


void setup(void) {
    Serial.begin(9600);
    Serial.println();
    
    psramInit();
    
	CarAnalyzerLog_d("###################################");
	CarAnalyzerLog_d("on start:");
	CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
	CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
	CarAnalyzerLog_d("ChipRevision %d, Cpu Freq %d, SDK Version %s", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    CarAnalyzerLog_d("Flash Size %u, Flash Speed %u", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
	CarAnalyzerLog_d("###################################");


    config = (Config*) ps_malloc(sizeof(Config));
    gsmData = (GsmData*) ps_malloc(sizeof(GsmData));
    gpsData = (GpsData*) ps_malloc(sizeof(GpsData));
    bluetoothData = (BluetoothData*) ps_malloc(sizeof(BluetoothData));
    carData = (CarData*) ps_malloc(sizeof(CarData));

    //historicalData24h = (HistoricalData*) ps_malloc(sizeof(HistoricalData));
    //historicalData24h->heapMemory = new CircularBuffer<uint32_t, 86400>();
    //historicalData24h->psramMemory = new CircularBuffer<uint32_t, 86400>();

    updateTimers = (Timers*) ps_malloc(sizeof(Timers));

    gpsData->valid = false;

    loadConfiguration("/config.json", *config);
    saveConfiguration("/config.json", *config);

	CarAnalyzerLog_d("After configuration Loading:");
    CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
	CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

    // En fonction des valeurs des paramètres, nous allons pouvoir démarrer les différents services.
    if (config->gsmUse || config->gpsUse) {
        CarAnalyzerLog_d("Starting GSM...");
        SoftwareSerial* gsmSerial = (SoftwareSerial*) ps_malloc(sizeof(SoftwareSerial));
        gsmSerial = new SoftwareSerial();
        gsmSerial->begin(9600, SWSERIAL_8N1, GSM_MODEM_RX, GSM_MODEM_TX, false);

        gsmModem = (TinyGsm*) ps_malloc(sizeof(TinyGsm));
        gsmModem = new TinyGsm(*gsmSerial);

        if (config->gsmUse) {
            CarAnalyzerLog_d("GPRS Status: %d", gsmModem->isGprsConnected());
            if (!gsmModem->isGprsConnected()) {
                CarAnalyzerLog_w("GPRS is not connected");

                if (!gsmModem->init(config->gsmSimPin)) {
                    CarAnalyzerLog_w("Init cannot be done!");

                    if (!gsmModem->restart(config->gsmSimPin)) {
                        CarAnalyzerLog_e("GSM cannot be started...");
                    }
                }
            }

            CarAnalyzerLog_d("GSM Sim Status: %d", gsmModem->getSimStatus());

            if (gsmModem->getSimStatus() != 1 && gsmModem->getSimStatus() != 3) {
                if (!gsmModem->simUnlock(config->gsmSimPin)) {
                    CarAnalyzerLog_e("GSM cannot be unlocked... [%s]", config->gsmSimPin);
                }
            }

            if (!gsmModem->isGprsConnected())  {
                if(!gsmModem->gprsConnect(config->gsmApn, config->gsmUser, config->gsmPassword)) {
                    CarAnalyzerLog_e("GSM network connection cannot be started... [APN: %s, user: %s, pass: %s]", config->gsmApn, config->gsmUser, config->gsmPassword);
                }
            }  
            
            if (gsmModem->isGprsConnected())  {
                CarAnalyzerLog_i("GSM is started and connected to network");
            }
            
            CarAnalyzerLog_d("After starting GSM:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        }

        if (config->gpsUse) {
            updateTimers->gpsTimer = millis();

            if (gsmModem->enableGPS()) {
                CarAnalyzerLog_i("GPS is started...");

            }
            
            CarAnalyzerLog_d("After starting GPS:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        }
    }

    if (config->abrpUse) {
        updateTimers->abrpTimer = millis();
        CarAnalyzerLog_d("After Abrp Initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    if (config->wifiUse) {
        if (String("AP").equals(config->wifiMode)) {
            WiFi.softAP(config->wifiSsid, config->wifiPassword);
        }

        if (String("STA").equals(config->wifiMode)) {
            WiFi.begin(config->wifiSsid, config->wifiPassword);
        }

        CarAnalyzerLog_d("After WiFi Initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

        // On configure maintenant l'affichage des pages web
        server = new AsyncWebServer(80);

        server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", "Hello");
        });

        server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse * response = new AsyncJsonResponse();
            response->addHeader("Server","ESP Async Web Server");
            response->addHeader("Access-Control-Allow-Origin","*");
            JsonObject doc = response->getRoot();
            doc["gsmUse"] = config->gsmUse;
            doc["gsmSimPin"] = config->gsmSimPin;
            doc["gsmApn"] = config->gsmApn;
            doc["gsmUser"] = config->gsmUser;
            doc["gsmPassword"] = config->gsmPassword;
            doc["gpsUse"] = config->gpsUse;
            doc["gpsPrecision"] = config->gpsPrecision;
            doc["abrpUse"] = config->abrpUse;
            doc["abrpUpdate"] = config->abrpUpdate;
            doc["abrpApiKey"] = config->abrpApiKey;
            doc["abrpToken"] = config->abrpToken;
            doc["abrpCarModel"] = config->abrpCarModel;
            doc["wifiUse"] = config->wifiUse;
            doc["wifiMode"] = config->wifiMode;
            doc["wifiSsid"] = config->wifiSsid;
            doc["wifiPassword"] = config->wifiPassword;
            doc["bluetoothUse"] = config->bluetoothUse;
            doc["bluetoothPin"] = config->bluetoothPin;
            doc["bluetoothAddress"] = config->bluetoothAddress;
            doc["logToFile"] = config->logToFile;
            response->setLength();
            request->send(response);
        });

        server->on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
            int params = request->params();
            for(int i=0;i<params;i++){
                AsyncWebParameter* p = request->getParam(i);
                if(p->isFile()){ //p->isPost() is also true
                    Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                } else if(p->isPost()){
                    Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                } else {
                    Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
            }

            if (request->hasParam("gsmUse", true)) {
                AsyncWebParameter* p = request->getParam("gsmUse", true);
                CarAnalyzerLog_d("gsmUse param received %s", p->value());

                if (p->value().equals("true")) {
                    config->gsmUse = true;
                    if(!gsmModem->isGprsConnected()) {
                        CarAnalyzerLog_d("result for connect %d", gsmModem->gprsConnect(config->gsmApn, config->gsmUser, config->gsmPassword));
                    }
                } else {
                    config->gsmUse = false;
                    CarAnalyzerLog_d("disconntecting gsm %d", gsmModem->isGprsConnected());
                    if(gsmModem->isGprsConnected()) {
                        CarAnalyzerLog_d("result for disconnect %d", gsmModem->gprsDisconnect());
                    }
                }


            } 
            
            if (request->hasParam("gsmSimPin", true)) {
                AsyncWebParameter* p = request->getParam("gsmSimPin", true);
                CarAnalyzerLog_d("gsmSimPin param received %s", p->value());
                strlcpy(config->gsmSimPin, p->value().c_str(),sizeof(config->gsmSimPin));
            }
            
            if (request->hasParam("gsmApn", true)) {
                AsyncWebParameter* p = request->getParam("gsmApn", true);
                CarAnalyzerLog_d("gsmApn param received %s", p->value());
                strlcpy(config->gsmApn, p->value().c_str(),sizeof(config->gsmApn));
            }
            
            if (request->hasParam("gsmUser", true)) {
                AsyncWebParameter* p = request->getParam("gsmUser", true);
                CarAnalyzerLog_d("gsmUser param received %s", p->value());
                strlcpy(config->gsmUser, p->value().c_str(),sizeof(config->gsmUser));
            }
            
            if (request->hasParam("gsmPassword", true)) {
                AsyncWebParameter* p = request->getParam("gsmPassword", true);
                CarAnalyzerLog_d("gsmPassword param received %s", p->value());
                strlcpy(config->gsmPassword, p->value().c_str(),sizeof(config->gsmPassword));
            }
            

            if (request->hasParam("gpsUse", true)) {
                AsyncWebParameter* p = request->getParam("gpsUse", true);
                CarAnalyzerLog_d("gpsUse param received %s", p->value());

                if (p->value().equals("true")) {
                    config->gpsUse = true;
                    CarAnalyzerLog_d("result for connect %d", gsmModem->enableGPS());
                } else {
                    config->gpsUse = false;
                    CarAnalyzerLog_d("result for disconnect %d", gsmModem->disableGPS());
                }
            } 
            
            if (request->hasParam("gpsPrecision", true)) {
                AsyncWebParameter* p = request->getParam("gpsPrecision", true);
                CarAnalyzerLog_d("gpsPrecision param received %d", p->value().toInt());
                config->gpsPrecision = p->value().toInt();
            }

            saveConfiguration("/config.json", *config);
            
            AsyncJsonResponse * response = new AsyncJsonResponse();
            response->addHeader("Server","ESP Async Web Server");
            response->addHeader("Access-Control-Allow-Origin","*");
            response->setCode(200);
            request->send(response);
        });


        server->on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse * response = new AsyncJsonResponse();
            response->addHeader("Server","ESP Async Web Server");
            response->addHeader("Access-Control-Allow-Origin","*");
            JsonObject doc = response->getRoot();
            doc["gsmOperator"] = gsmData->gsmOperator;
            doc["imei"] = gsmData->imei;
            doc["imsi"] = gsmData->imsi;
            doc["isConnected"] = gsmData->isConnected;
            doc["localIP"] = gsmData->localIP;
            doc["modemInfo"] = gsmData->modemInfo;
            doc["modemName"] = gsmData->modemName;
            doc["registrationStatus"] = gsmData->registrationStatus;
            doc["signalQuality"] = gsmData->signalQuality;
            doc["simCCID"] = gsmData->simCCID;
            doc["simStatus"] = gsmData->simStatus;

            response->setLength();
            request->send(response);
        });

        server->on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", "Restarting card...");
            ESP.restart();
        });



        server->onNotFound([](AsyncWebServerRequest *request) {
            request->send(404, "text/plain", "Page Not Found");
        });

        server->begin();
        
        CarAnalyzerLog_d("After Web Server started:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

    }

    if (config->bluetoothUse) {
        btSerial = new BluetoothSerial();

        btSerial->begin(config->bluetoothName, true);
        
        CarAnalyzerLog_d("After Bluetooth Initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        
        btSerial->setPin("1234");

        if (!btSerial->connect(config->macAddress)) {
            CarAnalyzerLog_e("Connection to BT device failed.");
        } else {
            elm = new ELM327();
            if (!elm->begin(*btSerial, false, 2000)) {
                CarAnalyzerLog_e("ELM327 initializeation failed.");
            } else {
                CarAnalyzerLog_d("After ELM327 Initialization success:");
                CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
                CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
            }
        }
    }
}

void loop(void) {
    if (millis() - updateTimers->gpsTimer > 5000) {
        updateTimers->gpsTimer = millis();

        float latitude          = 0;
        float longitude         = 0;
        float speed             = 0;
        float altitude          = 0;
        int   visibleSat        = 0;
        int   usedSat           = 0;
        float accuracy          = 0;
        int   year              = 0;
        int   month             = 0;
        int   day               = 0;
        int   hour              = 0;
        int   minute            = 0;
        int   second            = 0;

        ESP32Time rtc;

        if (gsmModem->getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
            rtc.setTime(second, minute, hour, day, month, year);
            gpsData->epoch = rtc.getEpoch();

            gpsData->latitude = latitude;
            gpsData->longitude = longitude;
            gpsData->accuracy = accuracy;
            gpsData->altitude = altitude;
            gpsData->speed = speed;
            gpsData->usedSat = usedSat;
            gpsData->visibleSat = visibleSat;

            gpsData->valid = true;

            CarAnalyzerLog_d("After GPS Update:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        } else {
            CarAnalyzerLog_w("No data from GPS: %s", gsmModem->getGPSraw());

            gpsData->valid = false;
        }
    }

    if (millis() - updateTimers->gsmTimer > 30000) {
        strlcpy(gsmData->modemInfo, gsmModem->getModemInfo().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->modemName, gsmModem->getModemName().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->imei, gsmModem->getIMEI().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->imsi, gsmModem->getIMSI().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->localIP, gsmModem->getLocalIP().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->simCCID, gsmModem->getSimCCID().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->gsmOperator, gsmModem->getOperator().c_str(), sizeof(gsmData->modemInfo));
        gsmData->simStatus = gsmModem->getSimStatus();
        gsmData->signalQuality = gsmModem->getSignalQuality();
        gsmData->registrationStatus = gsmModem->getRegistrationStatus();
        gsmData->isConnected = gsmModem->isGprsConnected();
    }

    if (millis() - updateTimers->abrpTimer > config->abrpUpdate) {
        updateTimers->abrpTimer = millis();

        // On demande à mettre à jour ABRP.
        SpiRamJsonDocument doc(1500);
        doc["soc"] = 16;
        doc["soh"] = 100;

        if (gpsData->valid) {
            doc["speed"] = gpsData->speed;
            doc["lat"] = String(gpsData->latitude, config->gpsPrecision);
            doc["lon"] = String(gpsData->longitude, config->gpsPrecision);
            doc["elevation"] = gpsData->altitude;
        }

        doc["car_model"] = config->abrpCarModel;

        char* serializedDoc = (char*) ps_malloc(measureJson(doc) + 1);

        serializeJson(doc, serializedDoc, measureJson(doc) + 1);

        CarAnalyzerLog_d("data to abrp: %s", serializedDoc);

        CarAnalyzerLog_d("After Abrp Update:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    if (millis() - updateTimers->bluetoothTimer > 30000) {
        updateTimers->bluetoothTimer = millis();

        CarAnalyzerLog_d("ELM Connection status: %d", btSerial->connected());
        if (!btSerial->connected()) {
            CarAnalyzerLog_w("Connection to ELM device has been lost!");
            btSerial->end();

            btSerial->begin("Car-Analyzer", true);
            
            CarAnalyzerLog_d("After Bluetooth Initialization:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
            
            btSerial->setPin("1234");
            if (!btSerial->connect(config->macAddress)) {
                CarAnalyzerLog_e("Connection to BT device failed.");
            } else {
                if (!elm->begin(*btSerial, true, 2000)) {
                    CarAnalyzerLog_e("CELM327 initializeation failed.");
                } else {
                    CarAnalyzerLog_d("After ELM327 Initialization success:");

                }
            }
        }

        //CarAnalyzerLog_d("Response for command AT SH 7E4: %d", elm->sendCommand("AT SH 7E4"));
        //elm->rpm();
        //if (elm->status == ELM_SUCCESS) {
        //    CarAnalyzerLog_d("Response for ELM.rpm: %u", elm->rpm());      
        //} else {
        //    CarAnalyzerLog_w("No data received for ELM.rpm");
        //    elm->printError();
        //}

        CarAnalyzerLog_d("ELM Connection status: %d", btSerial->connected());

    }
/*
    if (millis() - updateTimers->memoryUsageTimer > 1000) {
        updateTimers->memoryUsageTimer = millis();

        CarAnalyzerLog_d("Before storing data in buffer:");

        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

        //historicalData24h->heapMemory->push(ESP.getFreeHeap());
        //historicalData24h->psramMemory->push(ESP.getFreePsram());
        

        CarAnalyzerLog_d("After storing data in buffer:");

        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

        CarAnalyzerLog_d("Free Heap Memory:");
        CarAnalyzerLog_d("Data stored count: %u", historicalData24h->heapMemory->size());
        CarAnalyzerLog_d("Data stored last data: %u", historicalData24h->heapMemory->last());

        CarAnalyzerLog_d("Free PSRam Memory:");
        CarAnalyzerLog_d("Data stored count: %u", historicalData24h->psramMemory->size());
        CarAnalyzerLog_d("Data stored last data: %u", historicalData24h->psramMemory->last());

    }
*/
}

