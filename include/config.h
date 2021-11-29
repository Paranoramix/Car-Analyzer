
#include <Arduino.h>


#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_USE_GPRS true
#define GSM_MODEM_RX 16
#define GSM_MODEM_TX 17


#define SerialMon WebSerial


/////////////////////////////////////////////////////////////////////
// Variables for GSM
#define GsmSerial Serial2

const char* simPin        = "0000";

const char gsmApn[]       = "orange.fr";
const char gsmGprsUser[]  = "orange";
const char gsmGprsPass[]  = "orange";

String modemName          = "";
String modemInfo          = "";
String simStatus          = "";
String gprsStatus         = "";
String ccid               = "";
String imei               = "";
String imsi               = "";
String gsmOperator        = "";
String signalQuality      = "";
String networkMode        = "";
String registrationStatus = "";

/////////////////////////////////////////////////////////////////////
// Variables for GPS
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
