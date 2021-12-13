#include <Arduino.h>
#include <ArduinoJson.h>
//#include <CarConfigFile.h>
//#include <ESP32Time.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <WiFi.h>


struct SpiRamAllocator {
        void* allocate(size_t size) {
                return ps_malloc(size);

        }
        void deallocate(void* pointer) {
                free(pointer);
        }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;


// Liste des variables de configuration
struct Config {
    char* gsmSimPin = (char *) ps_malloc(5);
    char* gsmApn = (char *) ps_malloc(65);
    char* gsmGprsUser = (char *) ps_malloc(65);
    char* gsmGprsPass = (char *) ps_malloc(65);
};

Config config;

uint32_t *configMillis = (uint32_t *)ps_malloc(sizeof(uint32_t));
uint32_t *configUpdatePeriod = (uint32_t *)ps_malloc(sizeof(uint32_t));

boolean *configLogToFile = (boolean *)ps_malloc(sizeof(boolean));
uint32_t *configLogMillis = (uint32_t *)ps_malloc(sizeof(uint32_t));
uint32_t *configLogUpdatePeriod = (uint32_t *)ps_malloc(sizeof(uint32_t));

char* simPin = (char *)ps_calloc(5, sizeof(char));

char* gsmApn = (char *)ps_calloc(50, sizeof(char));
char* gsmGprsUser = (char *)ps_calloc(50, sizeof(char));
char* gsmGprsPass = (char *)ps_calloc(50, sizeof(char));

boolean *abrpConnection = (boolean *)ps_malloc(sizeof(boolean));
boolean *homeAsisstantConnection = (boolean *)ps_malloc(sizeof(boolean));

uint32_t        *abrpMillis = (uint32_t *)ps_malloc(sizeof(uint32_t));
uint32_t        *abrpUpdatePeriod = (uint32_t *)ps_malloc(sizeof(uint32_t));
boolean        *abrpSendData = (boolean *)ps_malloc(sizeof(boolean));   // True if you want to send telemetry to ABRP.
char*       abrpApiKey = (char *)ps_calloc(40, sizeof(char));                 //"32b2162f-9599-4647-8139-66e9f9528370";
char*       abrpToken = (char *)ps_calloc(40, sizeof(char));                  //"68131ce8-a73d-4420-90f9-10a93ea30208";
char*       abrpCarModel = (char *)ps_calloc(50, sizeof(char));               //"hyundai:ioniq5:22:74";

const char* abrpApiHostname   = "api.iternio.com";
uint8_t     abrpApiPort       = 80;
const char* abrUri            = "/1/tlm/send";


boolean loadConfigurationFromSDCard(char*);
