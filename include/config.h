#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_USE_GPRS true
#define GSM_MODEM_RX 16
#define GSM_MODEM_TX 17


#define SerialMon Serial
#define GsmSerial Serial2

const char* simPin = "0000";

const char gsmApn[]      = "orange.fr";
const char gsmGprsUser[] = "orange";
const char gsmGprsPass[] = "orange";
