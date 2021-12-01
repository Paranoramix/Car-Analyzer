#include "config.h"
#include <ESP32Time.h>

///////////////////////////////////////////////////////////////////
// Web Server & WiFi
///////////////////////////////////////////////////////////////////
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);


///////////////////////////////////////////////////////////////////
// Data definitions
///////////////////////////////////////////////////////////////////
#include <CarAnalyzerData.h>
CarAnalyzerDataClass CarAnalyzerData;

///////////////////////////////////////////////////////////////////
// Sockets definitions
///////////////////////////////////////////////////////////////////
#include <CarAnalyzerSocket.h>

// LogsSocket est utilisé pour les messages de journalisation.
CarAnalyzerSocketClass LogsSocket;

// ChipSocket est utilisé pour les données de la puce ESP.
CarAnalyzerSocketClass ChipSocket;

// GsmSocket est utilisé pour les données de la puce GSM.
CarAnalyzerSocketClass GsmSocket;

// GpsSocket est utilisé pour les données GPS.
CarAnalyzerSocketClass GpsSocket;

void setupCarAnalyzerSockets(void) {
  LogsSocket.begin(&server, "log");
  ChipSocket.begin(&server, "chip");
  GsmSocket.begin(&server, "gsm");
  GpsSocket.begin(&server, "gps");
}





///////////////////////////////////////////////////////////////////
// OTA Update
///////////////////////////////////////////////////////////////////

#include "elegantWebpage.h"

#include <AsyncElegantOTA.h>

const char* ssid = "Car-Analyzer";
const char* password = "123456789";

void setupWiFi(void) {
  WiFi.softAP(ssid, password);

  AsyncElegantOTA.begin(&server);

  // WebSerial is accessible at "<IP Address>/webserial" in browser


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(Serial, "text/plain", 12);
    response->addHeader("Server","ESP Async Web Server");
    request->send(response);
  });


  server.begin();

  delay(3000);
  SerialMon.println("Car Analyzer v0.0.1");
  SerialMon.println("...");
  delay(1000);
  SerialMon.println("--- Started! ");
  SerialMon.println("");
  SerialMon.println("");
  SerialMon.println("");
}


///////////////////////////////////////////////////////////////////
// GSM
///////////////////////////////////////////////////////////////////
#include <TinyGSMClient.h>
#include <ArduinoHttpClient.h>

TinyGsm gsmModem(GsmSerial);
TinyGsmClient gsmClientHomeAssistant(gsmModem, 0);
TinyGsmClient gsmClientABRP(gsmModem, 1);

void setupGsm(void) {
  SerialMon.println("GSM Initialization...");

  GsmSerial.begin(9600, SERIAL_8N1, GSM_MODEM_RX, GSM_MODEM_TX);
  delay(100);

  // Afin d'optimiser les temps de redémarrage, on vérifie l'état de la carte.
  // En cas de plantage de l'ESP, la carte SIM7000E peut être déjà démarrée.
  // Il n'est donc pas nécessaire de redémarrer le modem.

  SerialMon.print("--- Checking modem status...  ");

  if (!gsmModem.isGprsConnected()) {
    SerialMon.println("KO [Not connected to network]");
    SerialMon.print("   => Modem reset...                    ");

    if (!gsmModem.init(simPin)) {
      SerialMon.println("KO");
      SerialMon.print("   => Restarting modem...               ");

      if (!gsmModem.restart(simPin)) {
        SerialMon.println("KO. Please unplug / plug the device!");
        while(true) { }
      }
    }
  }

  SerialMon.println("OK :)");

  // Ensuite, quelquesoit la situation, on essayer de déverrouiller la carte SIM.
  if (gsmModem.getSimStatus() != 1 && gsmModem.getSimStatus() != 3) {
    SerialMon.println("--- Unlocking SIM Card...             ");

    if (!gsmModem.simUnlock(simPin)) {
        SerialMon.println("KO");
    } else {
      SerialMon.println("OK :)");
    }
  }

  // Ensuite, si on n'est pas connecté, alors on se connecte.
  if (!gsmModem.isGprsConnected()) {
    SerialMon.print("--- Connecting to GPRS network...       ");

    if(!gsmModem.gprsConnect(gsmApn, gsmGprsUser, gsmGprsPass)) {
      SerialMon.println("KO");
    }

    SerialMon.println("OK :)");
  }

  // Quand le GSM est connecté, alors on initialise le CarAnalyzerData.
  CarAnalyzerData.setModemName(gsmModem.getModemName());
  CarAnalyzerData.setModemInfo(gsmModem.getModemInfo());
  CarAnalyzerData.setSimStatus(gsmModem.getSimStatus());
  CarAnalyzerData.setSimCCID(gsmModem.getSimCCID());
  CarAnalyzerData.setImei(gsmModem.getIMEI());
  CarAnalyzerData.setImsi(gsmModem.getIMSI());
  CarAnalyzerData.setGsmOperator(gsmModem.getOperator());
  CarAnalyzerData.setSignalQuality(gsmModem.getSignalQuality());
  CarAnalyzerData.setRegistrationStatus(gsmModem.getRegistrationStatus());
  CarAnalyzerData.setLocalIP(gsmModem.getLocalIP());
}
/*
void abrpUpdate(void) {
  DynamicJsonDocument  doc(1000);
  doc["utc"] = epoch;
  doc["soc"] = 29;
  doc["soh"] = 100;
  doc["speed"] = speed;
  doc["lat"] = latitude;
  doc["lon"] = longitude;
  doc["elevation"] = altitude;
  doc["car_model"] = String(abrpCarModel);

  String data;
  serializeJson(doc, data);  
  HttpClient httpClienABRP(gsmClientABRP, "api.iternio.com", 80);
  String r = "/1/tlm/send?api_key=" + String(abrpApiKey) + "&token=" + String(abrpToken) + "&tlm=" + data;
  Serial.println(r);
  Serial.println(httpClienABRP.get(r));
  int status = httpClienABRP.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
}
*/

void updateGsmData(bool init = false) {
  if (init) {
    Serial.println("INIT GSM");

    CarAnalyzerData.setModemName(gsmModem.getModemName());
    CarAnalyzerData.setModemInfo(gsmModem.getModemInfo());
    CarAnalyzerData.setSimCCID(gsmModem.getSimCCID());
    CarAnalyzerData.setImei(gsmModem.getIMEI());
    CarAnalyzerData.setImsi(gsmModem.getIMSI());
  }

  CarAnalyzerData.setSimStatus(gsmModem.getSimStatus());
  CarAnalyzerData.setGsmOperator(gsmModem.getOperator());
  CarAnalyzerData.setSignalQuality(gsmModem.getSignalQuality());
  CarAnalyzerData.setRegistrationStatus(gsmModem.getRegistrationStatus());
  CarAnalyzerData.setLocalIP(gsmModem.getLocalIP());
}

///////////////////////////////////////////////////////////////////
// GPS
///////////////////////////////////////////////////////////////////
void setupGps(void) {
  SerialMon.println("GPS Initialization...");

  SerialMon.print("--- Starting GPS...                     ");

  if (gsmModem.enableGPS()) {
    SerialMon.println("OK :)");
  } else {
    SerialMon.println("KO");
  }
}

void updateGpsData(bool init = false) {
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

  if (gsmModem.getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
    rtc.setTime(second, minute, hour, day, month, year);

    CarAnalyzerData.setLatitude(latitude);
    CarAnalyzerData.setLongitude(longitude);
    CarAnalyzerData.setSpeed(speed);
    CarAnalyzerData.setAltitude(altitude);
    CarAnalyzerData.setVisibleSat(visibleSat);
    CarAnalyzerData.setUsedSat(usedSat);
    CarAnalyzerData.setAccuracy(accuracy);
    CarAnalyzerData.setEpoch(rtc.getEpoch());
  }
}


///////////////////////////////////////////////////////////////////
// Chip
///////////////////////////////////////////////////////////////////


void updateChipData(bool init = false) {
  if (init) {
    Serial.println("INIT CHIP");
    uint32_t chipId = 0;
      for(int i=0; i<17; i=i+8) {
      chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    CarAnalyzerData.setChipId(chipId);
    CarAnalyzerData.setChipRevision(ESP.getChipRevision());
    CarAnalyzerData.setCpuFreqMHz(ESP.getCpuFreqMHz());
    CarAnalyzerData.setFlashChipSize(ESP.getFlashChipSize());
    CarAnalyzerData.setFlashChipSpeed(ESP.getFlashChipSpeed());
    CarAnalyzerData.setHeapSize(ESP.getHeapSize());
    CarAnalyzerData.setPsramSize(ESP.getPsramSize());
    CarAnalyzerData.setSketchMD5(ESP.getSketchMD5());
    CarAnalyzerData.setSketchSize(ESP.getSketchSize());
  }
  
  CarAnalyzerData.setFreeHeapSize(ESP.getFreeHeap());
  CarAnalyzerData.setFreePsramSize(ESP.getFreePsram());
}





void setup(void) {
  Serial.begin(9600);
  Serial.println("");
  
  setupWiFi();
  setupGsm();
  setupGps();
  setupCarAnalyzerSockets();
  
  delay(2000);

  updateChipData(true);
  updateGsmData(true);
  updateGpsData(true);
  
  ChipSocket.send(CarAnalyzerData.getAllData());
}


long t = 0;


void loop(void) {
  if (millis() - t > 1000) {
    t = millis();
    updateGpsData();
    updateGsmData();
    updateChipData();

  
  ChipSocket.send(CarAnalyzerData.getChangedData());

  }
}