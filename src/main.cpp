#define TINY_GSM_MODEM_SIM7000

#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Time.h>
#include <SafeString.h>

#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGSMClient.h>
#include <WiFi.h>

#include "CarAnalyzerLog.h"
#include "CarAnalyzerData.h"
#include "CarAnalyzerArduinoJson.h"


#include "CarAnalyzerObd.h"
#include "CarAnalyzerGsm.h"
#include "CarAnalyzerGps.h"

#define TINY_GSM_USE_GPRS true
#define GSM_MODEM_RX 27
#define GSM_MODEM_TX 25

HttpClient *abrpHttpClient;
TinyGsm *gsmModem;
TinyGsmClient *abrpGsmClient;

AsyncWebServer *server;

CarAnalyzerObd *carAnalyzerObd;
CarAnalyzerGsm *carAnalyzerGsm;
CarAnalyzerGps *carAnalyzerGps;



BluetoothSerial *btSerial;
ELM327 *elm;

/**
 * @brief Extrait une valeur depuis une trame récupérée par l'ODB.
 * 
 * @param source trame OBD2
 * @param position position à partir de laquelle extraire la valeur.
 * @param length  longueur de la valeur.
 * @return long 
 */
long extractCarData(char *source, uint8_t position, uint8_t length)
{
    if (String(source).startsWith("SEARCHING"))
    {
        position += 9;
    }

    if (strlen(source) >= position + length + 1)
    {
        char *value = NULL;
        char *endPtr;

        value = (char *)ps_malloc((length + 1) * sizeof(char));

        memcpy(value, &source[position], length);
        value[length] = '\0';

        return strtol(value, &endPtr, HEX);
    }
    else
    {
        return -999;
    }
}

/**
 * @brief Lit une données depuis le dongle OBD.
 * 
 * @param header 
 * @param query 
 * @param lengthPayloadAttempted 
 * @return true 
 * @return false 
 */
bool readCarData(const char *header, const char *query, uint8_t lengthPayloadAttempted = 0)
{
    CarAnalyzerLog_d("(%lu) Header: %s    Query: %s", millis(), header, query);

    int8_t statusHeader = elm->sendCommand(header);
    if (statusHeader != ELM_SUCCESS)
    {
        CarAnalyzerLog_w("Error when sending header %s: %d", header, statusHeader);
        return false;
    }

    delay(500);

    int8_t statusQuery = elm->sendCommand(query);
    if (statusQuery != ELM_SUCCESS)
    {
        CarAnalyzerLog_w("Error when sending query %s: %d", query, statusQuery);

        if (statusQuery == ELM_GENERAL_ERROR)
        {
            btSerial->disconnect();
        }

        return false;
    }

    delay(500);

    if (lengthPayloadAttempted > 0 && strlen(elm->payload) != lengthPayloadAttempted)
    {
        CarAnalyzerLog_w("Payload Length error: %d (attempted: %d)", strlen(elm->payload), lengthPayloadAttempted);

        return false;
    }
    CarAnalyzerLog_d("payload: %s", elm->payload);

    if (!sdCardData->isMounted)
    {
        return true;
    }

    File file = SD.open("/" + String(query) + ".log", FILE_APPEND);
    file.printf("%ld;%s\n", millis(), elm->payload);
    file.flush();
    file.close();

    return true;
}


void loadConfiguration(const char *filename, Config &config)
{
    if (!sdCardData->isMounted)
    {
        return;
    }

    File file = SD.open(filename);
    SpiRamJsonDocument doc(file.size() * 2);
    DeserializationError error = deserializeJson(doc, file);

    if (error)
    {
        CarAnalyzerLog_e("Failed to read configuration file [%s]. Using default configuration", filename);
    }

    config.gsmUse = doc["gsmUse"] | true;
    CarAnalyzerLog_v("gsmUSe: %d", config.gsmUse);

    strlcpy(config.gsmSimPin, doc["gsmSimPin"] | "0000", sizeof(config.gsmSimPin));
    CarAnalyzerLog_v("gsmSimPin: %s", config.gsmSimPin);

    strlcpy(config.gsmApn, doc["gsmApn"] | "orange.fr", sizeof(config.gsmApn));
    CarAnalyzerLog_v("gsmApn: %s", config.gsmApn);

    strlcpy(config.gsmUser, doc["gsmUser"] | "orange", sizeof(config.gsmUser));
    CarAnalyzerLog_v("gsmUser: %s", config.gsmUser);

    strlcpy(config.gsmPassword, doc["gsmPassword"] | "orange", sizeof(config.gsmPassword));
    CarAnalyzerLog_v("gsmPassword: %s", config.gsmPassword);

    config.gpsUse = doc["gpsUse"] | true;
    CarAnalyzerLog_v("gpsUse: %d", config.gpsUse);

    config.gpsPrecision = doc["gpsPrecision"] | 8;
    CarAnalyzerLog_v("gpsPrecision: %d", config.gpsPrecision);

    config.abrpUse = doc["abrpUse"] | false;
    CarAnalyzerLog_v("abrpUse: %d", config.abrpUse);

    config.abrpUpdate = doc["abrpUpdate"] | 60000;
    CarAnalyzerLog_v("abrpUpdate: %u", config.abrpUpdate);

    strlcpy(config.abrpApiKey, doc["abrpApiKey"] | "32b2162f-9599-4647-8139-66e9f9528370", sizeof(config.abrpApiKey));
    CarAnalyzerLog_v("abrpApiKey: %s", config.abrpApiKey);

    strlcpy(config.abrpToken, doc["abrpToken"] | "68131ce8-a73d-4420-90f9-10a93ea30208", sizeof(config.abrpToken));
    CarAnalyzerLog_v("abrpToken: %s", config.abrpToken);

    strlcpy(config.abrpCarModel, doc["abrpCarModel"] | "hyundai:ioniq5:22:74", sizeof(config.abrpCarModel));
    CarAnalyzerLog_v("abrpCarModel: %s", config.abrpCarModel);

    config.wifiUse = doc["wifiUse"] | true;
    CarAnalyzerLog_v("wifiUse: %d", config.wifiUse);

    strlcpy(config.wifiMode, doc["wifiMode"] | "AP", sizeof(config.wifiMode));
    CarAnalyzerLog_v("wifiMode: %s", config.wifiMode);

    strlcpy(config.wifiSsid, doc["wifiSsid"] | "Car-Analyzer", sizeof(config.wifiSsid));
    CarAnalyzerLog_v("wifiSsid: %s", config.wifiSsid);

    strlcpy(config.wifiPassword, doc["wifiPassword"] | "123456789", sizeof(config.wifiPassword));
    CarAnalyzerLog_v("wifiPassword: %s", config.wifiPassword);

    config.bluetoothUse = doc["bluetoothUse"] | true;
    CarAnalyzerLog_v("bluetoothUse: %d", config.bluetoothUse);

    strlcpy(config.bluetoothName, doc["bluetoothName"] | "CarAnalyzer", sizeof(config.bluetoothName));
    CarAnalyzerLog_v("bluetoothName: %s", config.bluetoothName);

    strlcpy(config.bluetoothPin, doc["bluetoothPin"] | "1234", sizeof(config.bluetoothPin));
    CarAnalyzerLog_v("bluetoothPin: %s", config.bluetoothPin);

    strlcpy(config.bluetoothAddress, doc["bluetoothAddress"] | "dc:0d:30:a4:3e:30", sizeof(config.bluetoothAddress));
    CarAnalyzerLog_v("bluetoothAddress: %s", config.bluetoothAddress);
    sscanf(config.bluetoothAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &config.macAddress[0], &config.macAddress[1], &config.macAddress[2], &config.macAddress[3], &config.macAddress[4], &config.macAddress[5]);

    config.carDataUpdateOnUse = doc["carDataUpdateOnUse"] | 1000;
    CarAnalyzerLog_v("carDataUpdateOnUse: %u", config.carDataUpdateOnUse);

    config.carDataUpdateOnSleep = doc["carDataUpdateOnSleep"] | 60000;
    CarAnalyzerLog_v("carDataUpdateOnSleep: %u", config.carDataUpdateOnSleep);

    config.logToFile = doc["logToFile"] | true;
    CarAnalyzerLog_v("logToFile: %d", config.logToFile);

    file.close();
}

// Saves the configuration to a file
void saveConfiguration(const char *filename, Config &config)
{
    if (!sdCardData->isMounted)
    {
        return;
    }

    // Delete existing file, otherwise the configuration is appended to the file
    File myFileIn = SD.open(filename, FILE_READ);
    File myFileOut = SD.open(String(filename) + ".bkp", FILE_WRITE);
    while (myFileIn.available())
    {
        myFileOut.write(myFileIn.read());
    }

    myFileIn.close();
    myFileOut.close();

    // Open file for writing
    File file = SD.open(filename, FILE_WRITE);
    if (!file)
    {
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
    doc["carDataUpdateOnUse"] = config.carDataUpdateOnUse;
    doc["carDataUpdateOnSleep"] = config.carDataUpdateOnSleep;
    doc["logToFile"] = config.logToFile;

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        CarAnalyzerLog_e("Failed to write to file");
    }

    // Close the file
    file.close();
}

void writeData(const char *filename, const char *data)
{
    if (!sdCardData->isMounted)
    {
        return;
    }

    char *d = (char *)ps_calloc(100, sizeof(char));
    sprintf(d, "%ld,%s", gpsData->epoch, data);

    File file = SD.open(filename, FILE_APPEND);
    file.println(d);
    file.close();

    free(d);
}

void setup(void)
{
    Serial.begin(115200);
    Serial.println();

    psramInit();

    CarAnalyzerLog_d("###################################");
    CarAnalyzerLog_d("on start:");
    CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
    CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    CarAnalyzerLog_d("ChipRevision %d, Cpu Freq %d, SDK Version %s", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    CarAnalyzerLog_d("Flash Size %u, Flash Speed %u", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    CarAnalyzerLog_d("###################################");

    config = (Config *)ps_malloc(sizeof(Config));
    gsmData = (GsmData *)ps_malloc(sizeof(GsmData));
    gpsData = (GpsData *)ps_malloc(sizeof(GpsData));
    bluetoothData = (BluetoothData *)ps_malloc(sizeof(BluetoothData));
    carData = (CarData *)ps_malloc(sizeof(CarData));
    sdCardData = (SDCardData *)ps_malloc(sizeof(SDCardData));
    chipData = (ChipData *)ps_malloc(sizeof(ChipData));

    //historicalData24h = (HistoricalData*) ps_malloc(sizeof(HistoricalData));
    //historicalData24h->heapMemory = new CircularBuffer<uint32_t, 86400>();
    //historicalData24h->psramMemory = new CircularBuffer<uint32_t, 86400>();

    updateTimers = (Timers *)ps_malloc(sizeof(Timers));

    gpsData->valid = false;

    if (!SD.begin())
    {
        CarAnalyzerLog_d("Cart mount failed!");
    }

    sdCardData->isMounted = true;
    sdCardData->cardType = SD.cardType();
    sdCardData->cardSize = SD.cardSize() / (1024 * 1024);

    loadConfiguration("/config.json", *config);
    saveConfiguration("/config.json", *config);

    CarAnalyzerLog_d("After configuration Loading:");
    CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
    CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());


    if (config->bluetoothUse) {
        // If the user requested to use BT, we start bluetooth device.

        // First, we have to initialize dedicated component.
        // This component manages bluetoothSerial and ELMDuino objects.
        carAnalyzerObd = new CarAnalyzerObd();
        
        // Now, we can begin to use component.
        carAnalyzerObd->begin(21, 22, config->bluetoothAddress, config->bluetoothPin, config->bluetoothName);

        // Because it's not possible to determine if bluetooth OBD is switched on, 
        // we have to try to connect.
        if (!carAnalyzerObd->connect()) {
            carAnalyzerObd->switchOff();
            delay(500);
            carAnalyzerObd->connect();
        }

        CarAnalyzerLog_d("After OBD initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());


    }

    if (config->gsmUse) {
        carAnalyzerGsm = new CarAnalyzerGsm();

        carAnalyzerGsm->begin(27, 25);

        carAnalyzerGsm->connect(config->gsmSimPin, config->gsmApn, config->gsmUser, config->gsmPassword);

        CarAnalyzerLog_d("After GSM initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }


    if (config->gpsUse) {
        carAnalyzerGps = new CarAnalyzerGps();

        carAnalyzerGps->begin(27, 25);

        carAnalyzerGps->connect();

        CarAnalyzerLog_d("After GPS initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    uint32_t t = millis();

    while(true) {
        if (millis() - t > 1000) {
            t = millis();
            carAnalyzerGps->printGPS();
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        } else {
            delay(10);
        }
    }




















    // En fonction des valeurs des paramètres, nous allons pouvoir démarrer les différents services.
    if (config->gsmUse || config->gpsUse)
    {
        CarAnalyzerLog_d("Starting GSM...");
        SoftwareSerial *gsmSerial = (SoftwareSerial *)ps_malloc(sizeof(SoftwareSerial));
        gsmSerial = new SoftwareSerial();
        gsmSerial->begin(9600, SWSERIAL_8N1, GSM_MODEM_RX, GSM_MODEM_TX, false);

        gsmModem = (TinyGsm *)ps_malloc(sizeof(TinyGsm));
        gsmModem = new TinyGsm(*gsmSerial);

        if (config->gsmUse)
        {
            CarAnalyzerLog_d("GPRS Status: %d", gsmModem->isGprsConnected());
            if (!gsmModem->isGprsConnected())
            {
                CarAnalyzerLog_w("GPRS is not connected");

                if (!gsmModem->init(config->gsmSimPin))
                {
                    CarAnalyzerLog_w("Init cannot be done!");

                    if (!gsmModem->restart(config->gsmSimPin))
                    {
                        CarAnalyzerLog_e("GSM cannot be started...");
                    }
                }
            }

            CarAnalyzerLog_d("GSM Sim Status: %d", gsmModem->getSimStatus());

            if (gsmModem->getSimStatus() != 1 && gsmModem->getSimStatus() != 3)
            {
                if (!gsmModem->simUnlock(config->gsmSimPin))
                {
                    CarAnalyzerLog_e("GSM cannot be unlocked... [%s]", config->gsmSimPin);
                }
            }

            if (!gsmModem->isGprsConnected())
            {
                if (!gsmModem->gprsConnect(config->gsmApn, config->gsmUser, config->gsmPassword))
                {
                    CarAnalyzerLog_e("GSM network connection cannot be started... [APN: %s, user: %s, pass: %s]", config->gsmApn, config->gsmUser, config->gsmPassword);
                }
            }

            if (gsmModem->isGprsConnected())
            {
                CarAnalyzerLog_i("GSM is started and connected to network");
            }

            if (config->abrpUse)
            {
                abrpGsmClient = new TinyGsmClient(*gsmModem, 1);
            }

            CarAnalyzerLog_d("After starting GSM:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        }

        if (config->gpsUse)
        {
            updateTimers->gpsTimer = millis();

            if (gsmModem->enableGPS())
            {
                CarAnalyzerLog_i("GPS is started...");
            }

            CarAnalyzerLog_d("After starting GPS:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        }
    }

    if (config->abrpUse)
    {
        updateTimers->abrpTimer = millis();
        CarAnalyzerLog_d("After Abrp Initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    if (config->wifiUse)
    {
        if (String("AP").equals(config->wifiMode))
        {
            WiFi.softAP(config->wifiSsid, config->wifiPassword);
        }

        if (String("STA").equals(config->wifiMode))
        {
            WiFi.begin(config->wifiSsid, config->wifiPassword);
        }

        CarAnalyzerLog_d("After WiFi Initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

        // On configure maintenant l'affichage des pages web
        server = new AsyncWebServer(80);

        server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                       AsyncJsonResponse *response = new AsyncJsonResponse();
                       response->addHeader("Server", "ESP Async Web Server");
                       response->addHeader("Access-Control-Allow-Origin", "*");
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

        server->on("/config", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                       int params = request->params();
                       for (int i = 0; i < params; i++)
                       {
                           AsyncWebParameter *p = request->getParam(i);
                           if (p->isFile())
                           { //p->isPost() is also true
                               Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                           }
                           else if (p->isPost())
                           {
                               Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                           }
                           else
                           {
                               Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                           }
                       }

                       if (request->hasParam("gsmUse", true))
                       {
                           AsyncWebParameter *p = request->getParam("gsmUse", true);
                           CarAnalyzerLog_d("gsmUse param received %s", p->value());

                           if (p->value().equals("true"))
                           {
                               config->gsmUse = true;
                               if (!gsmModem->isGprsConnected())
                               {
                                   CarAnalyzerLog_d("result for connect %d", gsmModem->gprsConnect(config->gsmApn, config->gsmUser, config->gsmPassword));
                               }
                           }
                           else
                           {
                               config->gsmUse = false;
                               CarAnalyzerLog_d("disconntecting gsm %d", gsmModem->isGprsConnected());
                               if (gsmModem->isGprsConnected())
                               {
                                   CarAnalyzerLog_d("result for disconnect %d", gsmModem->gprsDisconnect());
                               }
                           }
                       }

                       if (request->hasParam("gsmSimPin", true))
                       {
                           AsyncWebParameter *p = request->getParam("gsmSimPin", true);
                           CarAnalyzerLog_d("gsmSimPin param received %s", p->value());
                           strlcpy(config->gsmSimPin, p->value().c_str(), sizeof(config->gsmSimPin));
                       }

                       if (request->hasParam("gsmApn", true))
                       {
                           AsyncWebParameter *p = request->getParam("gsmApn", true);
                           CarAnalyzerLog_d("gsmApn param received %s", p->value());
                           strlcpy(config->gsmApn, p->value().c_str(), sizeof(config->gsmApn));
                       }

                       if (request->hasParam("gsmUser", true))
                       {
                           AsyncWebParameter *p = request->getParam("gsmUser", true);
                           CarAnalyzerLog_d("gsmUser param received %s", p->value());
                           strlcpy(config->gsmUser, p->value().c_str(), sizeof(config->gsmUser));
                       }

                       if (request->hasParam("gsmPassword", true))
                       {
                           AsyncWebParameter *p = request->getParam("gsmPassword", true);
                           CarAnalyzerLog_d("gsmPassword param received %s", p->value());
                           strlcpy(config->gsmPassword, p->value().c_str(), sizeof(config->gsmPassword));
                       }

                       if (request->hasParam("gpsUse", true))
                       {
                           AsyncWebParameter *p = request->getParam("gpsUse", true);
                           CarAnalyzerLog_d("gpsUse param received %s", p->value());

                           if (p->value().equals("true"))
                           {
                               config->gpsUse = true;
                               CarAnalyzerLog_d("result for connect %d", gsmModem->enableGPS());
                           }
                           else
                           {
                               config->gpsUse = false;
                               CarAnalyzerLog_d("result for disconnect %d", gsmModem->disableGPS());
                           }
                       }

                       if (request->hasParam("gpsPrecision", true))
                       {
                           AsyncWebParameter *p = request->getParam("gpsPrecision", true);
                           CarAnalyzerLog_d("gpsPrecision param received %d", p->value().toInt());
                           config->gpsPrecision = p->value().toInt();
                       }

                       saveConfiguration("/config.json", *config);

                       AsyncJsonResponse *response = new AsyncJsonResponse();
                       response->addHeader("Server", "ESP Async Web Server");
                       response->addHeader("Access-Control-Allow-Origin", "*");
                       response->setCode(200);
                       request->send(response);
                   });

        server->on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                   {

                       AsyncJsonResponse *response = new AsyncJsonResponse(false, 4096U);
                       response->addHeader("Server", "ESP Async Web Server");
                       response->addHeader("Access-Control-Allow-Origin", "*");

                       JsonObject doc = response->getRoot();
                       

                       if (gpsData->valid)
                       {
                           JsonObject gps = doc.createNestedObject("gps");
                           gps["accuracy"] = gpsData->accuracy;
                           gps["altitude"] = gpsData->altitude;
                           gps["epoch"] = gpsData->epoch;
                           gps["latitude"] = gpsData->latitude;
                           gps["longitude"] = gpsData->longitude;
                           gps["speed"] = gpsData->speed;
                           gps["usedSat"] = gpsData->usedSat;
                           gps["valid"] = gpsData->valid;
                           gps["visibleSat"] = gpsData->visibleSat;
                           gps["lastUpdate"] = gpsData->lastUpdate;
                       }

                       JsonObject bluetooth = doc.createNestedObject("bluetooth");
                       bluetooth["isConnected"] = bluetoothData->isConnected;

                       if (carData->carData1Valid || carData->carData2Valid || carData->carData3Valid || carData->carData4Valid || carData->carData5Valid)
                       {
                           JsonObject car = doc.createNestedObject("car");

                           if (carData->carData1Valid)
                           {
                               JsonObject carData1 = car.createNestedObject("220101");
                               carData1["batteryCurrent"] = carData->batteryCurrent;
                               carData1["batteryVoltage"] = carData->batteryVoltage;
                               carData1["batteryMaxTemperature"] = carData->batteryMaxTemperature;
                               carData1["batteryMinTemperature"] = carData->batteryMinTemperature;
                               carData1["batteryTemperature1"] = carData->batteryTemperature1;
                               carData1["batteryTemperature2"] = carData->batteryTemperature2;
                               carData1["batteryTemperature3"] = carData->batteryTemperature3;
                               carData1["batteryTemperature4"] = carData->batteryTemperature4;
                               carData1["batteryTemperature5"] = carData->batteryTemperature5;
                               carData1["battInletTemperature"] = carData->battInletTemperature;
                               carData1["maxCellVolt"] = carData->maxCellVolt;
                               carData1["maxCellVoltNumber"] = carData->maxCellVoltNumber;
                               carData1["minCellVolt"] = carData->minCellVolt;
                               carData1["minCellVoltNumber"] = carData->minCellVoltNumber;
                               carData1["battFanSpeed"] = carData->battFanSpeed;
                               carData1["auxBattVoltage"] = carData->auxBattVoltage;
                               carData1["stateOfChargeRealPercent"] = carData->stateOfChargeRealPercent;
                               carData1["bmsRelay"] = carData->bmsRelay;
                               carData1["cecValue"] = carData->cecValue;
                               carData1["cedValue"] = carData->cedValue;
                               carData1["operatingTime"] = carData->operatingTime;
                               carData1["lastUpdate"] = carData->carData1LastUpdate;
                           }

                           if (carData->carData2Valid)
                           {

                               JsonObject carData2 = car.createNestedObject("220105");
                               carData2["voltageDifference"] = carData->voltageDifference;
                               carData2["stateOfChargePercent"] = carData->stateOfChargePercent;
                               carData2["stateOfHealthPercent"] = carData->stateOfHealthPercent;
                               carData2["heatTemperature1"] = carData->heatTemperature1;
                               carData2["lastUpdate"] = carData->carData2LastUpdate;
                           }

                           if (carData->carData3Valid)
                           {

                               JsonObject carData3 = car.createNestedObject("220100");
                               carData3["indoorTemperature"] = carData->indoorTemperature;
                               carData3["outdoorTemperature"] = carData->outdoorTemperature;
                               carData3["lastUpdate"] = carData->carData3LastUpdate;
                           }

                           if (carData->carData4Valid)
                           {

                               JsonObject carData4 = car.createNestedObject("22B002");
                               carData4["odometer"] = carData->odometer;
                               carData4["lastUpdate"] = carData->carData4LastUpdate;
                           }

                           if (carData->carData5Valid)
                           {

                               JsonObject carData5 = car.createNestedObject("22C00B");
                               carData5["frontLeftTyrePressure"] = carData->frontLeftTyrePressure;
                               carData5["frontLeftTyreTemperature"] = carData->frontLeftTyreTemperature;
                               carData5["frontRightTyrePressure"] = carData->frontRightTyrePressure;
                               carData5["frontRightTyreTemperature"] = carData->frontRightTyreTemperature;
                               carData5["rearLeftTyrePressure"] = carData->rearLeftTyrePressure;
                               carData5["rearLeftTyreTemperature"] = carData->rearLeftTyreTemperature;
                               carData5["rearRightTyrePressure"] = carData->rearRightTyrePressure;
                               carData5["rearRightTyreTemperature"] = carData->rearRightTyreTemperature;
                               carData5["lastUpdate"] = carData->carData5LastUpdate;
                           }
                           //                       doc["battPower"] = carData->battPower;
                           //                       doc["battTemperature"] = carData->battTemperature;
                           //                       doc["coolTemperature"] = carData->coolTemperature;
                       }

                       JsonObject gsm = doc.createNestedObject("gsm");
                       gsm["gsmOperator"] = gsmData->gsmOperator;
                       gsm["imei"] = gsmData->imei;
                       gsm["imsi"] = gsmData->imsi;
                       gsm["isConnected"] = gsmData->isConnected;
                       gsm["localIP"] = gsmData->localIP;
                       gsm["modemInfo"] = gsmData->modemInfo;
                       gsm["modemName"] = gsmData->modemName;
                       gsm["registrationStatus"] = gsmData->registrationStatus;
                       gsm["signalQuality"] = gsmData->signalQuality;
                       gsm["simCCID"] = gsmData->simCCID;
                       gsm["simStatus"] = gsmData->simStatus;

                       JsonObject chip = doc.createNestedObject("chip");
                       chip["freeHeap"] = chipData->freeHeap;
                       chip["freePsram"] = chipData->freePsram;

                       response->setLength();
                       request->send(response);
                   });

        server->on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                       request->send(200, "text/plain", "Restarting card...");
                       ESP.restart();
                   });

        server->on("/unmoutSDCard", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                       if (sdCardData->isMounted)
                       {
                           sdCardData->isMounted = false;

                           SD.end();
                           request->send(200);
                           return;
                       }

                       request->send(405);
                   });

        server->on("/moutSDCard", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                       if (!sdCardData->isMounted)
                       {

                           if (SD.begin())
                           {
                               sdCardData->isMounted = true;
                               sdCardData->cardType = SD.cardType();
                               sdCardData->cardSize = SD.cardSize() / (1024 * 1024);

                               request->send(200);

                               return;
                           }
                           else
                           {
                               request->send(500);
                           }
                       }

                       request->send(405);

                       return;
                   });

        server->on("/getFile", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                       if (request->hasParam("file"))
                       {
                           AsyncWebParameter *p = request->getParam("file");
                           CarAnalyzerLog_d("file param received %s", p->value());

                           request->send(SD, "/" + p->value() + ".log", "text/plain");
                           return;
                       }

                       request->send(405);
                   });

        server->onNotFound([](AsyncWebServerRequest *request)
                           { request->send(404, "text/plain", "Page Not Found"); });

        server->begin();

        CarAnalyzerLog_d("After Web Server started:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    if (config->bluetoothUse)
    {
        bluetoothData->isConnected = false;

        carData->carData1Valid = false;
        carData->carData2Valid = false;
        carData->carData3Valid = false;
        carData->carData4Valid = false;
        carData->carData5Valid = false;
        carData->carUpdate = config->carDataUpdateOnUse;

        btSerial = new BluetoothSerial();
        elm = new ELM327();

        btSerial->begin(config->bluetoothName, true);
        btSerial->disconnect();
        btSerial->unpairDevice(config->macAddress);

        CarAnalyzerLog_d("After Bluetooth Initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

        btSerial->setPin(config->bluetoothPin);

        if (!btSerial->connect(config->macAddress))
        {
            CarAnalyzerLog_e("Connection to BT device failed.");
            bluetoothData->isConnected = false;
        }
        else
        {
            if (!elm->begin(*btSerial, false, 5000, '6', 400))
            {
                CarAnalyzerLog_e("ELM327 initializeation failed.");
                bluetoothData->isConnected = false;
            }
            else
            {
                CarAnalyzerLog_d("After ELM327 Initialization success:");
                CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
                CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

                bluetoothData->isConnected = true;
                bluetoothData->currentFrame = 0;
            }
        }
    }
}

void loop(void)
{
    if (millis() - updateTimers->gsmTimer > 5000)
    {
        updateTimers->gsmTimer = millis();

        CarAnalyzerLog_d("GPRS Status: %d", gsmModem->isGprsConnected());
        if (!gsmModem->isGprsConnected())
        {
            CarAnalyzerLog_w("GPRS is not connected");

            if (!gsmModem->init(config->gsmSimPin))
            {
                CarAnalyzerLog_w("Init cannot be done!");

                if (!gsmModem->restart(config->gsmSimPin))
                {
                    CarAnalyzerLog_e("GSM cannot be started...");
                }
            }
        }

        CarAnalyzerLog_d("GSM Sim Status: %d", gsmModem->getSimStatus());

        if (gsmModem->getSimStatus() != 1 && gsmModem->getSimStatus() != 3)
        {
            if (!gsmModem->simUnlock(config->gsmSimPin))
            {
                CarAnalyzerLog_e("GSM cannot be unlocked... [%s]", config->gsmSimPin);
            }
        }

        if (!gsmModem->isGprsConnected())
        {
            if (!gsmModem->gprsConnect(config->gsmApn, config->gsmUser, config->gsmPassword))
            {
                CarAnalyzerLog_e("GSM network connection cannot be started... [APN: %s, user: %s, pass: %s]", config->gsmApn, config->gsmUser, config->gsmPassword);
            }
        }

        strlcpy(gsmData->modemInfo, gsmModem->getModemInfo().c_str(), sizeof(gsmData->modemInfo));
        strlcpy(gsmData->modemName, gsmModem->getModemName().c_str(), sizeof(gsmData->modemName));
        strlcpy(gsmData->imei, gsmModem->getIMEI().c_str(), sizeof(gsmData->imei));
        strlcpy(gsmData->imsi, gsmModem->getIMSI().c_str(), sizeof(gsmData->imsi));
        strlcpy(gsmData->localIP, gsmModem->getLocalIP().c_str(), sizeof(gsmData->localIP));
        strlcpy(gsmData->simCCID, gsmModem->getSimCCID().c_str(), sizeof(gsmData->simCCID));
        strlcpy(gsmData->gsmOperator, gsmModem->getOperator().c_str(), sizeof(gsmData->gsmOperator));
        gsmData->simStatus = gsmModem->getSimStatus();
        gsmData->signalQuality = gsmModem->getSignalQuality();
        gsmData->registrationStatus = gsmModem->getRegistrationStatus();
        gsmData->isConnected = gsmModem->isGprsConnected();
    }

    if (millis() - updateTimers->gpsTimer > 5000)
    {
        updateTimers->gpsTimer = millis();

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

        if (gsmModem->getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second))
        {
            rtc.setTime(second, minute, hour, day, month, year);
            gpsData->epoch = rtc.getEpoch();

            gpsData->latitude = latitude;
            gpsData->longitude = longitude;
            gpsData->accuracy = accuracy;
            gpsData->altitude = altitude;
            gpsData->speed = speed;
            gpsData->usedSat = usedSat;
            gpsData->visibleSat = visibleSat;
            gpsData->lastUpdate = millis() / 1000;

            gpsData->valid = true;

            if (gpsData->speed > 0) {
                // Cela signifie que le véhicule est en mouvement. Par conséquent, on force le rafraichissement de lecture de l'OBD
                carData->carUpdate = config->carDataUpdateOnUse;
            }

            CarAnalyzerLog_d("After GPS Update:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
        }
        else
        {
            CarAnalyzerLog_w("No data from GPS: %s", gsmModem->getGPSraw());

            gpsData->valid = false;

            gsmModem->enableGPS();
        }
    }

    if (millis() - updateTimers->bluetoothTimer > 1000)
    {
        updateTimers->bluetoothTimer = millis();

        CarAnalyzerLog_d("ELM Connection status: %d", btSerial->connected());
        if (!btSerial->connected())
        {
            CarAnalyzerLog_w("Connection to ELM device has been lost!");
            btSerial->end();

            bluetoothData->isConnected = false;

            btSerial->begin(config->bluetoothName, true);
            btSerial->disconnect();
            btSerial->unpairDevice(config->macAddress);

            CarAnalyzerLog_d("After Bluetooth Initialization:");
            CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
            CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

            btSerial->setPin(config->bluetoothPin);
            if (!btSerial->connect(config->macAddress))
            {
                CarAnalyzerLog_e("Connection to BT device failed.");
            }
            else
            {
                if (!elm->begin(*btSerial, false, 5000, '6', 400))
                {
                    CarAnalyzerLog_e("ELM327 initialization failed.");
                }
                else
                {
                    CarAnalyzerLog_d("After ELM327 Initialization success:");
                    bluetoothData->isConnected = true;
                }
            }
        }
    }

    if (millis() - updateTimers->carTimer > carData->carUpdate && bluetoothData->isConnected)
    {
        updateTimers->carTimer = millis();

        // On peut lire toutes les données depuis l'ELM327.
        if (bluetoothData->currentFrame == 0 && readCarData("AT SH 7E4", "220101"))
        {
            // Si on arrive à lire ce sdonnées, c'est que le véhicule est utilisé (en charge, décharge, etc.)
            CarAnalyzerLog_d("Car Data Update %d", carData->carUpdate);

            // Lecture des données.
            long socReal = extractCarData(elm->payload, 21, 2);
            if (socReal > -999)
            {
                carData->stateOfChargeRealPercent = socReal / 2.0;
            }

            long batteryCurrent1 = extractCarData(elm->payload, 35, 2);
            long batteryCurrent2 = extractCarData(elm->payload, 37, 2);
            if (batteryCurrent1 > -999 && batteryCurrent2 > -999)
            {
                if (batteryCurrent1 > 200)
                {
                    carData->batteryCurrent = (255 * (255 - batteryCurrent1) + 255 - batteryCurrent2) / 10.0;
                }
                else
                {
                    carData->batteryCurrent = (256 * batteryCurrent1 + batteryCurrent2) / 10.0;
                }
            }

            long batteryVoltage = extractCarData(elm->payload, 39, 4);
            if (batteryVoltage > -999)
            {
                carData->batteryVoltage = batteryVoltage / 10.0;
            }

            long auxBatteryVoltage = extractCarData(elm->payload, 77, 2);
            if (auxBatteryVoltage > -999)
            {
                carData->auxBattVoltage = auxBatteryVoltage / 10.0;
            }

            long cec = extractCarData(elm->payload, 99, 8);
            if (cec > -999)
            {
                carData->cecValue = cec / 10.0;
            }

            long ced = extractCarData(elm->payload, 107, 6) * 256 + extractCarData(elm->payload, 116, 2);
            if (ced > -999)
            {
                carData->cedValue = ced / 10.0;
            }

            long opTime = extractCarData(elm->payload, 117, 8);
            if (opTime > -999)
            {
                carData->operatingTime = opTime / 3600.0;
            }

            long battMaxTemp = extractCarData(elm->payload, 43, 2);
            if (battMaxTemp > -999)
            {
                if (battMaxTemp > 200)
                {
                    carData->batteryMaxTemperature = battMaxTemp - 255;
                }
                else
                {
                    carData->batteryMaxTemperature = battMaxTemp;
                }
            }

            long battMinTemp = extractCarData(elm->payload, 45, 2);
            if (battMinTemp > -999)
            {
                if (battMinTemp > 200)
                {
                    carData->batteryMinTemperature = battMinTemp - 255;
                }
                else
                {
                    carData->batteryMinTemperature = battMinTemp;
                }
            }

            long battTemp1 = extractCarData(elm->payload, 47, 2);
            if (battTemp1 > -999)
            {
                if (battTemp1 > 200)
                {
                    carData->batteryTemperature1 = battTemp1 - 255;
                }
                else
                {
                    carData->batteryTemperature1 = battTemp1;
                }
            }

            long battTemp2 = extractCarData(elm->payload, 51, 2);
            if (battTemp2 > -999)
            {
                if (battTemp2 > 200)
                {
                    carData->batteryTemperature2 = battTemp2 - 255;
                }
                else
                {
                    carData->batteryTemperature2 = battTemp2;
                }
            }

            long battTemp3 = extractCarData(elm->payload, 53, 2);
            if (battTemp3 > -999)
            {
                if (battTemp3 > 200)
                {
                    carData->batteryTemperature3 = battTemp3 - 255;
                }
                else
                {
                    carData->batteryTemperature3 = battTemp3;
                }
            }

            long battTemp4 = extractCarData(elm->payload, 55, 2);
            if (battTemp4 > -999)
            {
                if (battTemp4 > 200)
                {
                    carData->batteryTemperature4 = battTemp4 - 255;
                }
                else
                {
                    carData->batteryTemperature4 = battTemp4;
                }
            }

            long battTemp5 = extractCarData(elm->payload, 57, 2);
            if (battTemp5 > -999)
            {
                if (battTemp5 > 200)
                {
                    carData->batteryTemperature5 = battTemp5 - 255;
                }
                else
                {
                    carData->batteryTemperature5 = battTemp5;
                }
            }

            long battInletTemp = extractCarData(elm->payload, 61, 2);
            if (battInletTemp > -999)
            {
                if (battInletTemp > 200)
                {
                    carData->battInletTemperature = battInletTemp - 255;
                }
                else
                {
                    carData->battInletTemperature = battInletTemp;
                }
            }

            long maxCellVoltage = extractCarData(elm->payload, 63, 2);
            if (maxCellVoltage > -999)
            {
                carData->maxCellVolt = maxCellVoltage / 50.0;
            }

            long maxCellNumber = extractCarData(elm->payload, 67, 2);
            if (maxCellNumber > -999)
            {
                carData->maxCellVoltNumber = maxCellNumber;
            }

            long minCellVoltage = extractCarData(elm->payload, 69, 2);
            if (minCellVoltage > -999)
            {
                carData->minCellVolt = minCellVoltage / 50.0;
            }

            long minCellNumber = extractCarData(elm->payload, 71, 2);
            if (minCellNumber > -999)
            {
                carData->minCellVoltNumber = minCellNumber;
            }

            long battFanSpeed = extractCarData(elm->payload, 75, 2);
            if (battFanSpeed > -999)
            {
                carData->battFanSpeed = battFanSpeed;
            }

            carData->carData1Valid = true;

            carData->carData1LastUpdate = millis() / 1000;
        }

        if (bluetoothData->currentFrame == 1 && readCarData("AT SH 7E4", "220105"))
        {
            // Lecture des données.
            float soh = extractCarData(elm->payload, 69, 4);
            if (soh > -999)
            {
                carData->stateOfHealthPercent = soh * 0.1;
            }

            float socDisplay = extractCarData(elm->payload, 83, 2);
            if (socDisplay > -999)
            {
                carData->stateOfChargePercent = socDisplay * 0.5;
            }

            float heater1Temperature = extractCarData(elm->payload, 63, 2);
            if (heater1Temperature > -999)
            {
                if (heater1Temperature > 200)
                {
                    carData->heatTemperature1 = heater1Temperature - 255;
                }
                else
                {
                    carData->heatTemperature1 = heater1Temperature;
                }
            }

            carData->carData2Valid = true;
            carData->carData2LastUpdate = millis() / 1000;
        }

        if (bluetoothData->currentFrame == 2 && readCarData("AT SH 7B3", "220100"))
        {
            // Lecture des données.

            long indoorTemp = extractCarData(elm->payload, 23, 2);
            if (indoorTemp > -999)
            {
                carData->indoorTemperature = indoorTemp / 2.0 - 40.0;
            }

            long outdoorTemp = extractCarData(elm->payload, 27, 2);
            if (outdoorTemp > -999)
            {
                carData->outdoorTemperature = outdoorTemp / 2.0 - 40.0;
            }

            carData->carData3Valid = true;
            carData->carData3LastUpdate = millis() / 1000;
        }
        else
        {
            if (bluetoothData->currentFrame == 2)
            {
                // Si on n'arrive pas à récupérer cette trame, c'est que la voiture n'est pas utilisée.
                // On réduit la fréquence de récupération des données.
                carData->carUpdate = config->carDataUpdateOnSleep;
            }
        }

        if (bluetoothData->currentFrame == 3 && readCarData("AT SH 7C6", "22B002"))
        {
            // Lecture des données.

            long odometer = extractCarData(elm->payload, 25, 6);

            if (odometer > -999)
            {
                carData->odometer = odometer;
            }

            carData->carData4Valid = true;
            carData->carData4LastUpdate = millis() / 1000;
        }

        if (bluetoothData->currentFrame == 4 && readCarData("AT SH 7A0", "22C00B"))
        {
            // Lecture des données.
            long fltPress = extractCarData(elm->payload, 21, 2);
            if (fltPress > -999)
            {
                carData->frontLeftTyrePressure = fltPress / 5 / 14.504;
            }

            long fltTemp = extractCarData(elm->payload, 23, 2);
            if (fltTemp > -999)
            {
                carData->frontLeftTyreTemperature = fltTemp - 50;
            }

            long frtPress = extractCarData(elm->payload, 31, 2);
            if (frtPress > -999)
            {
                carData->frontRightTyrePressure = frtPress / 5 / 14.504;
            }

            long frtTemp = extractCarData(elm->payload, 35, 2);
            if (frtTemp > -999)
            {
                carData->frontRightTyreTemperature = frtTemp - 50;
            }

            long rrtPress = extractCarData(elm->payload, 43, 2);
            if (rrtPress > -999)
            {
                carData->rearRightTyrePressure = rrtPress / 5 / 14.504;
            }

            long rrtTemp = extractCarData(elm->payload, 45, 2);
            if (rrtTemp > -999)
            {
                carData->rearRightTyreTemperature = rrtTemp - 50;
            }

            long rltPress = extractCarData(elm->payload, 55, 2);
            if (rltPress > -999)
            {
                carData->rearLeftTyrePressure = rltPress / 5 / 14.504;
            }

            long rltTemp = extractCarData(elm->payload, 57, 2);
            if (rltTemp > -999)
            {
                carData->rearLeftTyreTemperature = rltTemp - 50;
            }

            carData->carData5Valid = true;
            carData->carData5LastUpdate = millis() / 1000;
        }

        bluetoothData->currentFrame = (bluetoothData->currentFrame + 1) % 5;

        CarAnalyzerLog_d("current Frame: %d", bluetoothData->currentFrame);
    }

    if (millis() - updateTimers->chipTimer > 5000)
    {
        updateTimers->chipTimer = millis();

        chipData->freeHeap = ESP.getFreeHeap();
        chipData->freePsram = ESP.getFreePsram();
        /*
        writeData("/data_heap.csv", String(ESP.getFreeHeap()).c_str());
        writeData("/data_psram.csv", String(ESP.getFreePsram()).c_str());
        writeData("/bluetooth_state.csv", String(btSerial->connected()).c_str());
   */
    }

    if (millis() - updateTimers->abrpTimer > config->abrpUpdate && gsmData->isConnected)
    {
        updateTimers->abrpTimer = millis();

        SpiRamJsonDocument doc(1000);

        if (carData->carData2Valid)
        {
            doc["soc"] = carData->stateOfChargePercent;
            doc["soh"] = carData->stateOfHealthPercent;
            doc["speed"] = carData->speed;
        }

        if (gpsData->valid)
        {
            doc["utc"] = gpsData->epoch;
            doc["speed"] = gpsData->speed;
            doc["lat"] = String(gpsData->latitude, config->gpsPrecision);
            doc["lon"] = String(gpsData->longitude, config->gpsPrecision);
            doc["elevation"] = gpsData->altitude;
        }

        doc["car_model"] = config->abrpCarModel;

        char *serializedDoc = (char *)ps_malloc(measureJson(doc) + 1);
        serializeJson(doc, serializedDoc, measureJson(doc) + 1);
        CarAnalyzerLog_d("Data for ABRP: %s", serializedDoc);

        HttpClient abrpHttpClient(*abrpGsmClient, "api.iternio.com", 80);
        String r = String("/1/tlm/send") + "?api_key=" + String(config->abrpApiKey) + "&token=" + String(config->abrpToken) + "&tlm=" + serializedDoc;
        CarAnalyzerLog_d("URL ABRP: %s", r);

        abrpHttpClient.get(r);

        CarAnalyzerLog_d("result code form ABRP: %d", abrpHttpClient.responseStatusCode());

        CarAnalyzerLog_d("After Abrp Update:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }
}
