#include "config.h"

#include <Arduino.h>

///////////////////////////////////////////////////////////////////
// OTA Update
///////////////////////////////////////////////////////////////////
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "Car-Analyzer";
const char* password = "123456789";

AsyncWebServer server(80);


///////////////////////////////////////////////////////////////////
// GSM
///////////////////////////////////////////////////////////////////
#include <TinyGSMClient.h>
TinyGsm gsmModem(GsmSerial);
TinyGsmClient gsmClientHomeAssistant(gsmModem, 0);
TinyGsmClient gsmClientABRP(gsmModem, 1);

void setupGsm(void) {
  GsmSerial.begin(9600, SERIAL_8N1, GSM_MODEM_RX, GSM_MODEM_TX);
  delay(100);

  // Afin d'optimiser les temps de redémarrage, on vérifie l'état de la carte.
  // En cas de plantage de l'ESP, la carte SIM7000E peut être déjà démarrée.
  // Il n'est donc pas nécessaire de redémarrer le modem.

  SerialMon.print("--- Vérification de l'état du Modem...  ");

  if (!gsmModem.isGprsConnected()) {
    SerialMon.println("KO [réseau non connecté]");
    SerialMon.print("   => Réinitialisation du modem...      ");

    if (!gsmModem.init(simPin)) {
      SerialMon.println("KO");
      SerialMon.print("   => Redémarrage du modem...           ");

      if (!gsmModem.restart(simPin)) {
        SerialMon.println("KO. Veuillez débrancher / rebrancher tout le système.");
        while(true) { }
      }
    }
  }

  SerialMon.println("Succès :)");

  // Ensuite, quelquesoit la situation, on essayer de déverrouiller la carte SIM.
  if (gsmModem.getSimStatus() != 1 && gsmModem.getSimStatus() != 3) {
    SerialMon.println("--- Déverrouillage de la carte SIM...   ");

    if (!gsmModem.simUnlock(simPin)) {
        SerialMon.println("KO");
    } else {
      SerialMon.println("Succès :)");
    }
  }

  // Ensuite, si on n'est pas connecté, alors on se connecte.
  if (!gsmModem.isGprsConnected()) {
    SerialMon.print("--- Connexion au réseau GSM...          ");

    if(!gsmModem.gprsConnect(gsmApn, gsmGprsUser, gsmGprsPass)) {
      SerialMon.println("KO");
    }

    SerialMon.println("Succès :)");
  }

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

void setupWiFi(void) {
  WiFi.softAP(ssid, password);

  AsyncElegantOTA.begin(&server);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  server.begin();
}

void setup(void) {
  SerialMon.begin(9600);

  Serial.println("");
  Serial.println(ESP.getFlashChipSize());
  Serial.println("");

  setupGsm();
  
  setupWiFi();
}

void loop(void) {
  // put your main code here, to run repeatedly:
}