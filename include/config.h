
#include <Arduino.h>


#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_USE_GPRS true
#define GSM_MODEM_RX 16
#define GSM_MODEM_TX 17

#define SerialMon Serial

/////////////////////////////////////////////////////////////////////
// Variables for GSM
#define GsmSerial Serial2

const char* simPin            = "0000";

const char gsmApn[]           = "orange.fr";
const char gsmGprsUser[]      = "orange";
const char gsmGprsPass[]      = "orange";

bool abrpConnection           = true;
bool homeAsisstantConnection  = true;

/////////////////////////////////////////////////////////////////////
// Variables for ABRP
long        updatePeriod      = 60000;  // Every 60 seconds.
bool        sendDataToABRP    = true;   // True if you want to send telemetry to ABRP.
const char* abrpApiKey        = "32b2162f-9599-4647-8139-66e9f9528370";
const char* abrpToken         = "68131ce8-a73d-4420-90f9-10a93ea30208";
const char* abrpCarModel      = "hyundai:ioniq5:22:74";

