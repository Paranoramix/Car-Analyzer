// #include "config.h"
// #include <ESP32Time.h>

// #include <CarConfigFile.h>

// boolean readConfiguration() {
//   /*
//    * Length of the longest line expected in the config file.
//    * The larger this number, the more memory is used
//    * to read the file.
//    * You probably won't need to change this number.
//    */
//   const uint8_t CONFIG_LINE_LENGTH = 127;
  
//   // The open configuration file.
//   CarConfigFile cfg;
  
//   // Open the configuration file.
//   if (!cfg.begin(configFile, CONFIG_LINE_LENGTH)) {
//     Serial.print("Failed to open configuration file: ");
//     Serial.println(configFile);
//     return false;
//   }
  
//   // Read each setting from the file.
//   while (cfg.readNextSetting()) {
    
//     // Put a nameIs() block here for each setting you have.
    
//     if (cfg.nameIs("logToFile")) {
//       configLogToFile = cfg.getBooleanValue();
//       Serial.print("Read logToFile: ");
//       Serial.println(configLogToFile);

//     } else if (cfg.nameIs("simPin")) {
//       simPin = cfg.copyValue();
//       Serial.print("Read simPin: ");
//       Serial.println(simPin);
    
//     } else if (cfg.nameIs("gsmApn")) {
//       gsmApn = cfg.copyValue();
//       Serial.print("Read gsmApn: ");
//       Serial.println(gsmApn);

//     } else if (cfg.nameIs("gsmGprsUser")) {
//       gsmGprsUser = cfg.copyValue();
//       Serial.print("Read gsmGprsUser: ");
//       Serial.println(gsmGprsUser);

//     } else if (cfg.nameIs("gsmGprsPass")) {
//       gsmGprsPass = cfg.copyValue();
//       Serial.print("Read gsmGprsPass: ");
//       Serial.println(gsmGprsPass);

//     } else if (cfg.nameIs("abrpSendData")) {
//       abrpSendData = cfg.getBooleanValue();
//       Serial.print("Read abrpSendData: ");
//       Serial.println(abrpSendData);

//     } else if (cfg.nameIs("abrpUpdatePeriod")) {
//       abrpUpdatePeriod = cfg.getIntValue();
//       Serial.print("Read abrpUpdatePeriod: ");
//       Serial.println(abrpUpdatePeriod);

//     } else if (cfg.nameIs("abrpApiKey")) {
//       abrpApiKey = cfg.copyValue();
//       Serial.print("Read abrpApiKey: ");
//       Serial.println(abrpApiKey);

//     } else if (cfg.nameIs("abrpToken")) {
//       abrpToken = cfg.copyValue();
//       Serial.print("Read abrpToken: ");
//       Serial.println(abrpToken);

//     } else if (cfg.nameIs("abrpCarModel")) {
//       abrpCarModel = cfg.copyValue();
//       Serial.print("Read abrpCarModel: ");
//       Serial.println(abrpCarModel);

//     } else {
//       // report unrecognized names.
//       Serial.print("Unknown name in config: ");
//       Serial.println(cfg.getName());
//     }
//   }

//   // clean up
//   cfg.end();
  
//   return true;
// }

// void appendFile(fs::FS &fs, const char * path, const char * message){
//   File file = fs.open(path, FILE_APPEND);
//   if(!file){
//     return;
//   }
//   if(file.print(message)){
//     file.print("\n");
//   }

//   file.close();
// }

// void deleteFile(fs::FS &fs, const char * path){
//   Serial.printf("Deleting file: %s\n", path);
//   if(fs.remove(path)){
//     Serial.println("File deleted");
//   } else {
//     Serial.println("Delete failed");
//   }
// }

// ///////////////////////////////////////////////////////////////////
// // Bluetooth
// ///////////////////////////////////////////////////////////////////
// //#include <BluetoothSerial.h>
// //#include <ELMduino.h>

// //BluetoothSerial SerialBT;
// //ELM327 elm327;

// uint8_t address[6]  = {0x13, 0x31, 0x18, 0x05, 0x88, 0x0A8};
// const char *pin = "1234";

// void setupElm(void) {
// /*  SerialBT.begin("ArduHUD", true);
//   if (!SerialBT.connect(address)) {
//     Serial.println("Couldn't connect to OBD scanner - Phase 1");
//   } else {
//     Serial.println("Connected to OBD scanner - Phase 1");
//   }
// /*
//   if (!elm327.begin(SerialBT, true, 2000, 0)) {
//     Serial.println("Couldn't connect to OBD scanner - Phase 2");
//   } else {
//     Serial.println("Connected to OBD scanner - Phase 2");
//   }
// */
//   Serial.println("Connected to ELM327");
// }

// ///////////////////////////////////////////////////////////////////
// // Web Server & WiFi
// ///////////////////////////////////////////////////////////////////
// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <ArduinoJson.h>

// AsyncWebServer server(80);


// ///////////////////////////////////////////////////////////////////
// // Data definitions
// ///////////////////////////////////////////////////////////////////
// #include <CarAnalyzerData.h>
// CarAnalyzerDataClass CarAnalyzerData;

// ///////////////////////////////////////////////////////////////////
// // Sockets definitions
// ///////////////////////////////////////////////////////////////////
// #include <CarAnalyzerSocket.h>

// // LogsSocket est utilisé pour les messages de journalisation.
// CarAnalyzerSocketClass LogsSocket;

// // ChipSocket est utilisé pour les données de la puce ESP.
// CarAnalyzerSocketClass ChipSocket;

// // GsmSocket est utilisé pour les données de la puce GSM.
// CarAnalyzerSocketClass GsmSocket;

// // GpsSocket est utilisé pour les données GPS.
// CarAnalyzerSocketClass GpsSocket;

// void setupCarAnalyzerSockets(void) {
//   LogsSocket.begin(&server, "log");
//   ChipSocket.begin(&server, "chip");
//   GsmSocket.begin(&server, "gsm");
//   GpsSocket.begin(&server, "gps");
// }







// ///////////////////////////////////////////////////////////////////
// // GSM
// ///////////////////////////////////////////////////////////////////
// #include <TinyGSMClient.h>
// #include <ArduinoHttpClient.h>
// #include <SoftwareSerial.h>

// SoftwareSerial mySerial;

// TinyGsm gsmModem(mySerial);
// TinyGsmClient gsmClientHomeAssistant(gsmModem, 0);
// TinyGsmClient gsmClientABRP(gsmModem, 1);

// void setupGsm(void) {
//   SerialMon.println("GSM Initialization...");
//   CarAnalyzerData.setModemName("");
//   CarAnalyzerData.setModemInfo("");
//   CarAnalyzerData.setSimStatus(-1);
//   CarAnalyzerData.setSimCCID("");
//   CarAnalyzerData.setImei("");
//   CarAnalyzerData.setImsi("");
//   CarAnalyzerData.setGsmOperator("");
//   CarAnalyzerData.setSignalQuality(99);
//   CarAnalyzerData.setRegistrationStatus(-1);
//   CarAnalyzerData.setLocalIP("");

//   mySerial.begin(9600, SWSERIAL_8N1, GSM_MODEM_RX, GSM_MODEM_TX, false);
//   delay(100);

//   // Afin d'optimiser les temps de redémarrage, on vérifie l'état de la carte.
//   // En cas de plantage de l'ESP, la carte SIM7000E peut être déjà démarrée.
//   // Il n'est donc pas nécessaire de redémarrer le modem.

//   SerialMon.print("--- Checking modem status...  ");

//   if (!gsmModem.isGprsConnected()) {
//     SerialMon.println("KO [Not connected to network]");
//     SerialMon.print("   => Modem reset...                    ");

//     if (!gsmModem.init(simPin)) {
//       SerialMon.println("KO");
//       SerialMon.print("   => Restarting modem...               ");

//       if (!gsmModem.restart(simPin)) {
//         SerialMon.println("KO. Please unplug / plug the device!");
//         return; 
//       }
//     }
//   }

//   // Ensuite, quelquesoit la situation, on essayer de déverrouiller la carte SIM.
//   if (gsmModem.getSimStatus() != 1 && gsmModem.getSimStatus() != 3) {
//     SerialMon.println("--- Unlocking SIM Card...             ");

//     if (!gsmModem.simUnlock(simPin)) {
//         SerialMon.println("KO");
//     } else {
//       SerialMon.println("OK :)");
//     }
//   }

//   // Ensuite, si on n'est pas connecté, alors on se connecte.
//   if (!gsmModem.isGprsConnected()) {
//     SerialMon.print("--- Connecting to GPRS network...       ");

//     if(!gsmModem.gprsConnect(gsmApn, gsmGprsUser, gsmGprsPass)) {
//       SerialMon.println("KO");
//     }

//     SerialMon.println("OK :)");
//   }

//   // Quand le GSM est connecté, alors on initialise le CarAnalyzerData.
//   CarAnalyzerData.setModemName(gsmModem.getModemName());
//   CarAnalyzerData.setModemInfo(gsmModem.getModemInfo());
//   CarAnalyzerData.setSimStatus(gsmModem.getSimStatus());
//   CarAnalyzerData.setSimCCID(gsmModem.getSimCCID());
//   CarAnalyzerData.setImei(gsmModem.getIMEI());
//   CarAnalyzerData.setImsi(gsmModem.getIMSI());
//   CarAnalyzerData.setGsmOperator(gsmModem.getOperator());
//   CarAnalyzerData.setSignalQuality(gsmModem.getSignalQuality());
//   CarAnalyzerData.setRegistrationStatus(gsmModem.getRegistrationStatus());
//   CarAnalyzerData.setLocalIP(gsmModem.getLocalIP());
// }

// void abrpUpdate(void) {
//   DynamicJsonDocument  doc(1000);
//   doc["utc"] = CarAnalyzerData.getEpoch();
//   doc["soc"] = 29;
//   doc["soh"] = 100;
//   doc["speed"] = CarAnalyzerData.getSpeed();
//   doc["lat"] = CarAnalyzerData.getLatitude();
//   doc["lon"] = CarAnalyzerData.getLongitude();
//   doc["elevation"] = CarAnalyzerData.getAltitude();
//   doc["car_model"] = String(abrpCarModel);

//   String data;
//   serializeJson(doc, data);  

//   HttpClient httpClienABRP(gsmClientABRP, abrpApiHostname, abrpApiPort);
//   String r = String(abrUri) + "?api_key=" + String(abrpApiKey) + "&token=" + String(abrpToken) + "&tlm=" + data;

//   int status = httpClienABRP.responseStatusCode();
//   Serial.print(F("Response status code: "));
//   Serial.println(status);
// }


// void updateGsmData(bool init = false) {
//   if (gsmModem.isGprsConnected()) {
//     if (init) {
//       Serial.println("INIT GSM");

//       CarAnalyzerData.setModemName(gsmModem.getModemName());
//       CarAnalyzerData.setModemInfo(gsmModem.getModemInfo());
//       CarAnalyzerData.setSimCCID(gsmModem.getSimCCID());
//       CarAnalyzerData.setImei(gsmModem.getIMEI());
//       CarAnalyzerData.setImsi(gsmModem.getIMSI());
//     }

//     CarAnalyzerData.setSimStatus(gsmModem.getSimStatus());
//     CarAnalyzerData.setGsmOperator(gsmModem.getOperator());
//     CarAnalyzerData.setSignalQuality(gsmModem.getSignalQuality());
//     CarAnalyzerData.setRegistrationStatus(gsmModem.getRegistrationStatus());
//     CarAnalyzerData.setLocalIP(gsmModem.getLocalIP());
//   }
// }

// ///////////////////////////////////////////////////////////////////
// // GPS
// ///////////////////////////////////////////////////////////////////
// void setupGps(void) {
//   SerialMon.println("GPS Initialization...");

//   SerialMon.print("--- Starting GPS...                     ");

//   if (gsmModem.enableGPS()) {
//     SerialMon.println("OK :)");
//   } else {
//     SerialMon.println("KO");
//   }
// }

// void updateGpsData(bool init = false) {
//   float latitude          = 0;
//   float longitude         = 0;
//   float speed             = 0;
//   float altitude          = 0;
//   int   visibleSat        = 0;
//   int   usedSat           = 0;
//   float accuracy          = 0;

//   int   year              = 0;
//   int   month             = 0;
//   int   day               = 0;
//   int   hour              = 0;
//   int   minute            = 0;
//   int   second            = 0;
  
//   ESP32Time rtc;

//   if (gsmModem.getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
//     rtc.setTime(second, minute, hour, day, month, year);

//     CarAnalyzerData.setLatitude(latitude);
//     CarAnalyzerData.setLongitude(longitude);
//     CarAnalyzerData.setSpeed(speed);
//     CarAnalyzerData.setAltitude(altitude);
//     CarAnalyzerData.setVisibleSat(visibleSat);
//     CarAnalyzerData.setUsedSat(usedSat);
//     CarAnalyzerData.setAccuracy(accuracy);
//     CarAnalyzerData.setEpoch(rtc.getEpoch());
//   } else {
//     // On essaye de redémarrer le module GPS
//     setupGps();

//     // CarAnalyzerData.setLatitude(0);
//     // CarAnalyzerData.setLongitude(0);
//     // CarAnalyzerData.setSpeed(0);
//     // CarAnalyzerData.setAltitude(0);
//     // CarAnalyzerData.setVisibleSat(0);
//     // CarAnalyzerData.setUsedSat(0);
//     // CarAnalyzerData.setAccuracy(0);
//     CarAnalyzerData.setEpoch(0);
//   }
// }


// ///////////////////////////////////////////////////////////////////
// // OTA Update
// ///////////////////////////////////////////////////////////////////


// const char* ssid = "Car-Analyzer";
// const char* password = "123456789";

// void setupWiFi(void) {
//   WiFi.softAP(ssid, password);

//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//     request->send(200, "text/plain", "Hi! I am ESP32.");
//   });

//   server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request) {
//     request->send(SD, "/history.csv", String(), true);
//   });

//   server.on("/clear_history", HTTP_GET, [](AsyncWebServerRequest *request) {
//     deleteFile(SD, "/history.csv");
//   });

//   server.on("/poweroff", HTTP_GET, [](AsyncWebServerRequest *request) {
//     gsmModem.gprsDisconnect();
//     delay(5000L);
//     gsmModem.poweroff();
//     delay(5000L);

//     esp_deep_sleep_start();
//   });

//   server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
//     AsyncWebServerResponse *response = request->beginResponse(Serial, "text/plain", 12);
//     response->addHeader("Server","ESP Async Web Server");
//     request->send(response);
//   });


//   server.begin();

//   delay(3000);
//   SerialMon.println("Car Analyzer v0.0.1");
//   SerialMon.println("...");
//   delay(1000);
//   SerialMon.println("--- Started! ");
//   SerialMon.println("");
//   SerialMon.println("");
//   SerialMon.println("");
// }

// ///////////////////////////////////////////////////////////////////
// // Chip
// ///////////////////////////////////////////////////////////////////


// void updateChipData(bool init = false) {
//   if (init) {
//     Serial.println("INIT CHIP");
//     uint32_t chipId = 0;
//       for(int i=0; i<17; i=i+8) {
//       chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
//     }
//     CarAnalyzerData.setChipId(chipId);
//     CarAnalyzerData.setChipRevision(ESP.getChipRevision());
//     CarAnalyzerData.setCpuFreqMHz(ESP.getCpuFreqMHz());
//     CarAnalyzerData.setFlashChipSize(ESP.getFlashChipSize());
//     CarAnalyzerData.setFlashChipSpeed(ESP.getFlashChipSpeed());
//     CarAnalyzerData.setHeapSize(ESP.getHeapSize());
//     CarAnalyzerData.setPsramSize(ESP.getPsramSize());
//     CarAnalyzerData.setSketchMD5(ESP.getSketchMD5());
//     CarAnalyzerData.setSketchSize(ESP.getSketchSize());
//   }
  
//   CarAnalyzerData.setFreeHeapSize(ESP.getFreeHeap());
//   CarAnalyzerData.setFreePsramSize(ESP.getFreePsram());
// }






// void setup(void) {
//   delay(500);
  
//   psramInit();


//   /////////pinMode(pinSelectSD, OUTPUT);
//   Serial.begin(9600);
//   Serial.println("");

//   // Chargement du fichier de configuration
//   Serial.println("Calling SD.begin()...");
//   if (!SD.begin()) {
//     Serial.println("SD.begin() failed. Check: ");
//     Serial.println("  card insertion,");
//     Serial.println("  SD shield I/O pins and chip select,");
//     Serial.println("  card formatting.");
//     return;
//   }
//   Serial.println("...succeeded.");

//   // Read our configuration from the SD card file.
//   readConfiguration();
//   setupElm();
//   delay(2000);

//   setupWiFi();
//   setupGsm();
//   setupGps();
//   setupCarAnalyzerSockets();

//   delay(2000);


//   updateChipData(true);
//   updateGsmData(true);
//   updateGpsData(true);
  
//   ChipSocket.send(CarAnalyzerData.getAllData());
// }


// void loop(void) {
//   Serial.print("FreeHeap: ");
//   Serial.println(ESP.getFreeHeap());
//   Serial.print("FreePSRam: ");
//   Serial.println(ESP.getFreePsram());

//   updateGpsData();
//   updateGsmData();
//   updateChipData();

//   // Redémarrage de la carte GSM.
//   if (!gsmModem.isGprsConnected()) {
//     setupGsm();
//     setupGps();
//   }

//   if (millis() - configMillis > configUpdatePeriod) {
//     configMillis = millis();
//     ChipSocket.send(CarAnalyzerData.getAllData());
//     Serial.println("ping");
//   } else {
//     ChipSocket.send(CarAnalyzerData.getChangedData());
//   }

//   // Demande de mise à jour de ABRP
//   if (abrpSendData && millis() - abrpMillis > abrpUpdatePeriod) {
//     abrpMillis = millis();
//     abrpUpdate();
//   }

//   if (configLogToFile && millis() - configLogMillis > configLogUpdatePeriod) {
//     configLogMillis = millis();
//     appendFile(SD, "/history.csv", CarAnalyzerData.getAllDataCsv().c_str());
//   }

//   delay(500);
// }