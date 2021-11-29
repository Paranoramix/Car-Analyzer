#include "config.h"

///////////////////////////////////////////////////////////////////
// OTA Update
///////////////////////////////////////////////////////////////////

#include "elegantWebpage.h"
#include "webserialWebpage.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WebSerial.h>

const char* ssid = "Car-Analyzer";
const char* password = "123456789";

AsyncWebServer server(80);

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
}

void setupWiFi(void) {
  WiFi.softAP(ssid, password);

  AsyncElegantOTA.begin(&server);

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  /* Attach Message Callback */
  WebSerial.msgCallback(recvMsg);


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
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

void updateGsmData(void) {

  // L'initialisation de la carte GSM est terminée.
  // On affiche toutes les informations !
  SerialMon.println("------------------------------------------");
  SerialMon.print("--- Modem Name: ");
  SerialMon.println(gsmModem.getModemName());

  SerialMon.print("--- Modem Info: ");
  SerialMon.println(gsmModem.getModemInfo());

  SerialMon.print("--- SIM Status: ");
  SerialMon.println(gsmModem.getSimStatus());

  SerialMon.print("--- GPRS status: ");
  SerialMon.println(gsmModem.isGprsConnected());

  SerialMon.print("--- CCID: ");
  SerialMon.println(gsmModem.getSimCCID());

  SerialMon.print("--- IMEI: ");
  SerialMon.println(gsmModem.getIMEI());

  SerialMon.print("--- IMSI: ");
  SerialMon.println(gsmModem.getIMSI());

  SerialMon.print("--- Operator: ");
  SerialMon.println(gsmModem.getOperator());

  SerialMon.print("--- Local IP: ");
  SerialMon.println(gsmModem.localIP());

  SerialMon.print("--- Signal quality: ");
  SerialMon.println(gsmModem.getSignalQuality()); 
  // Signal Quality: 
  //    0       -115 dBm ou moins         *
  //    1       -111  dBm                 **
  //    2...30  -110...-54 dBm            ***
  //    31      -52 dBm ou plus           ****
  //    99      Inconnu ou non détectable - 

  SerialMon.print("--- GSM Location: ");
  SerialMon.println(gsmModem.getGsmLocation());

  SerialMon.print("--- Network Mode: ");
  SerialMon.println(gsmModem.getNetworkMode());

  SerialMon.print("--- Registration Status: ");
  SerialMon.println(gsmModem.getRegistrationStatus());
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
  if (gsmModem.getGPS(&latitude, &longitude, &speed, &altitude, &visibleSat, &usedSat, &accuracy, &year, &month, &day, &hour, &minute, &second)) {
    SerialMon.print("--- Latitude, Longitude: ");
    SerialMon.println(String(latitude, 8) + String(", ") + String(longitude, 8));

    SerialMon.print("--- Speed: ");
    SerialMon.println(speed);

    SerialMon.print("--- Altitude: ");
    SerialMon.println(altitude);

    SerialMon.print("--- Visible Sat: ");
    SerialMon.println(visibleSat);

    SerialMon.print("--- Used Sat: ");
    SerialMon.println(usedSat);

    SerialMon.print("--- Accuracy: ");
    SerialMon.println(accuracy);

    SerialMon.print("--- Date & Time: ");
    SerialMon.println(year + String("/") + month + String("/") + day + String(" ") + hour + String(":") + minute + String(":") + second);
  } else {
    SerialMon.println("Couldn't get GPS/GNSS/GLONASS location");
    SerialMon.println(gsmModem.getGPSraw());

  }
}



void setup(void) {
  Serial.begin(9600);

  Serial.println("");
  
  setupWiFi();
  
  setupGsm();
  updateGsmData();

  setupGps();
  updateGpsData();
    

}

long t = 0;
void loop(void) {
  if (millis() - t > 10000) {
    t = millis();
    updateGpsData();
    updateGsmData();
  }
}