#include "config.h"

///////////////////////////////////////////////////////////////////
// Web Server & WiFi
///////////////////////////////////////////////////////////////////
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);


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

  delay(10000);
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
}

void abrpUpdate(void) {
  const char* abrpApiKey = "32b2162f-9599-4647-8139-66e9f9528370";
  const char* abrpToken = "68131ce8-a73d-4420-90f9-10a93ea30208";
  const char* abrpapiUrl = "http://api.iternio.com/1/tlm/send";

  DynamicJsonDocument  doc(1000);
  doc["soc"] = 29;
  doc["soh"] = 100;
  doc["speed"] = speed;
  doc["lat"] = latitude;
  doc["lon"] = longitude;
  doc["elevation"] = altitude;
  doc["car_model"] = "hyundai:ioniq5:22:74";

  String data;
  serializeJson(doc, data);  
  HttpClient httpClienABRP(gsmClientABRP, "api.iternio.com", 80);
  String r = "/1/tlm/send?api_key=32b2162f-9599-4647-8139-66e9f9528370&token=68131ce8-a73d-4420-90f9-10a93ea30208&tlm=" + data;
  Serial.println(r);
  Serial.println(httpClienABRP.get(r));
  int status = httpClienABRP.responseStatusCode();
  Serial.print(F("Response status code: "));
  Serial.println(status);
}

void updateGsmData(void) {
  DynamicJsonDocument  doc(1000);

  doc["time"]               = millis();
  doc["modemName"]          = gsmModem.getModemName();
  doc["modemInfo"]          = gsmModem.getModemInfo();
  doc["simStatus"]          = gsmModem.getSimStatus();
  doc["gprsStatus"]         = gsmModem.isGprsConnected();
  doc["ccid"]               = gsmModem.getSimCCID();
  doc["imei"]               = gsmModem.getIMEI();
  doc["imsi"]               = gsmModem.getIMSI();
  doc["gsmOperator"]        = gsmModem.getOperator();
  doc["signalQuality"]      = gsmModem.getSignalQuality();
  doc["networkMode"]        = gsmModem.getNetworkMode();
  doc["registrationStatus"] = gsmModem.getRegistrationStatus();

  GsmSocket.send(doc);
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

void updateGpsData(void) {
  DynamicJsonDocument  doc(500);
  doc["time"]               = millis();

  if (gsmModem.getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
  
    doc["latitude"]           = latitude;
    doc["longitude"]          = longitude;
    doc["speed"]              = speed;
    doc["altitude"]           = altitude;
    doc["visibleSat"]         = visibleSat;
    doc["usedSat"]            = usedSat;
    doc["accuracy"]           = accuracy;
    doc["year"]               = year;
    doc["month"]              = month;
    doc["day"]                = day;
    doc["hour"]               = hour;
    doc["minute"]             = minute;
    doc["second"]             = second;
  } else {
    doc["latitude"]         = "";
    doc["longitude"]        = "";
    doc["speed"]            = "";
    doc["altitude"]         = "";
    doc["visibleSat"]       = "";
    doc["usedSat"]          = "";
    doc["accuracy"]         = "";
    doc["year"]             = "";
    doc["month"]            = "";
    doc["day"]              = "";
    doc["hour"]             = "";
    doc["minute"]           = "";
    doc["second"]           = "";
  }


  GpsSocket.send(doc);
}


///////////////////////////////////////////////////////////////////
// Chip
///////////////////////////////////////////////////////////////////
void setup(void) {
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}

  ChipRevision     = ESP.getChipRevision();
  CpuFreqMHz       = ESP.getCpuFreqMHz();
  FlashChipMode    = ESP.getFlashChipMode();
  FlashChipSize    = ESP.getFlashChipSize();
  FlashChipSpeed   = ESP.getFlashChipSpeed();
  FreeHeap         = ESP.getFreeHeap();
  FreePsram        = ESP.getFreePsram();
  FreeSketchSpace  = ESP.getFreeSketchSpace();
  HeapSize         = ESP.getHeapSize();
  MaxAllocHeap     = ESP.getMaxAllocHeap();
  MaxAllocPsram    = ESP.getMaxAllocPsram();
  MinFreeHeap      = ESP.getMinFreeHeap();
  MinFreePsram     = ESP.getMinFreePsram();
  PsramSize        = ESP.getPsramSize();
  SdkVersion       = ESP.getSdkVersion();
  SketchMD5        = ESP.getSketchMD5();
  SketchSize       = ESP.getSketchSize();

  Serial.begin(9600);
      
  setupCarAnalyzerSockets();

  Serial.println("");
  
  setupWiFi();

  
  setupGsm();
  updateGsmData();
  
  setupGps();
  updateGpsData();

  delay(10000);

  abrpUpdate();

  

}

void updateChip(void) {
  DynamicJsonDocument  doc(500);
  doc["time"]             = millis();
  doc["ChipId"]           = String(chipId, HEX);
  doc["ChipRevision"]     = ChipRevision;
  doc["CpuFreqMHz"]       = CpuFreqMHz;
  doc["FlashChipMode"]    = FlashChipMode;
  doc["FlashChipSize"]    = FlashChipSize;
  doc["FlashChipSpeed"]   = FlashChipSpeed;
  doc["FreeHeap"]         = ESP.getFreeHeap();
  doc["FreePsram"]        = ESP.getFreePsram();
  doc["FreeSketchSpace"]  = FreeSketchSpace;
  doc["HeapSize"]         = HeapSize;
  doc["MaxAllocHeap"]     = MaxAllocHeap;
  doc["MaxAllocPsram"]    = MaxAllocPsram;
  doc["MinFreeHeap"]      = MinFreeHeap;
  doc["MinFreePsram"]     = MinFreePsram;
  doc["PsramSize"]        = PsramSize;
  doc["SdkVersion"]       = SdkVersion;
  doc["SketchMD5"]        = SketchMD5;
  doc["SketchSize"]       = SketchSize;

  ChipSocket.send(doc);
}

long t = 0;
void loop(void) {
  if (millis() - t > 1000) {
    t = millis();
    updateGpsData();
    updateGsmData();
    updateChip();
    Serial.println("ping");
  }
}