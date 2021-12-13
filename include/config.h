
#include <Arduino.h>

// GSM/GPS
// MODEM RX => 27
// MODEM TX => 25

// SDCARD
// CS       =>  5
// SCK      => 18
// MISO     => 19
// MOSI     => 23

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_USE_GPRS true
#define GSM_MODEM_RX 27
#define GSM_MODEM_TX 25

const char* configFile = "/config.cfg";

#define SerialMon Serial

long configMillis = 0;
long configUpdatePeriod = 60000;

bool configLogToFile = false;
long configLogMillis = 0;
long configLogUpdatePeriod = 5000;

/////////////////////////////////////////////////////////////////////
// Variables for GSM
#define GsmSerial Serial2

char* simPin;

char* gsmApn;
char* gsmGprsUser;
char* gsmGprsPass;

bool abrpConnection           = true;
bool homeAsisstantConnection  = true;

/////////////////////////////////////////////////////////////////////
// Variables for ABRP
long        abrpMillis        = 0;
long        abrpUpdatePeriod  = 60000;  // Every 60 seconds.
bool        abrpSendData      = true;   // True if you want to send telemetry to ABRP.
char*       abrpApiKey;                 //"32b2162f-9599-4647-8139-66e9f9528370";
char*       abrpToken;                  //"68131ce8-a73d-4420-90f9-10a93ea30208";
char*       abrpCarModel;               //"hyundai:ioniq5:22:74";

const char* abrpApiHostname   = "api.iternio.com";
uint8_t     abrpApiPort       = 80;
const char* abrUri            = "/1/tlm/send";
