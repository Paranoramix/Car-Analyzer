#define TINY_GSM_MODEM_SIM7600
//#define DUMP_AT_COMMANDS

#include <Arduino.h>

#include "CarAnalyzerArduinoJson.h"
#include "CarAnalyzerConstants.h"
#include "CarAnalyzerData.h"
#include "CarAnalyzerLog.h"
#include "CarAnalyzerVersion.h"

#include "CarAnalyzerABRP.h"
#include "CarAnalyzerChip.h"
#include "CarAnalyzerConfig.h"
#include "CarAnalyzerGps.h"
#include "CarAnalyzerGsm.h"
#include "CarAnalyzerHomeAssistant.h"
#include "CarAnalyzerObd.h"
#include "CarAnalyzerSdCard.h"

CarAnalyzerConfig *carAnalyzerConfig;
CarAnalyzerChip *carAnalyzerChip;
CarAnalyzerSdCard *carAnalyzerSdCard;
CarAnalyzerObd *carAnalyzerObd;
CarAnalyzerGsm *carAnalyzerGsm;
CarAnalyzerGps *carAnalyzerGps;

CarAnalyzerHomeAssistant *carAnalyzerHomeAssistant;
CarAnalyzerABRP *carAnalyzerABRP;

uint32_t t = 0;

void setup(void)
{
    psramInit();

    pinMode(19, INPUT_PULLUP);

    // Initialisation du port Série, pour accéder aux logs.
    Serial.begin(115200);
    Serial.println();

    CarAnalyzerLog_d("###################################");
    CarAnalyzerLog_d("Car-Analyzer - %s", VERSION);
    CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
    CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    CarAnalyzerLog_d("ChipRevision %d, Cpu Freq %d, SDK Version %s", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    CarAnalyzerLog_d("Flash Size %u, Flash Speed %u", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    CarAnalyzerLog_d("###################################");

    updateTimers = (Timers *)ps_malloc(sizeof(Timers));

    // Initialisation du composant permettant de tracer le comportement de l'ESP32.
    carAnalyzerChip = new CarAnalyzerChip(BATTERY_LEVEL_PIN, ALIM_LEVEL_PIN);
    carAnalyzerChip->update();

    // Initialisation du composant permettant la gestion de la carte SD.
    carAnalyzerSdCard = new CarAnalyzerSdCard(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

    // Si la carte SD n'a pas pu être monté, on passe en veille !
    if (!carAnalyzerSdCard->isMounted())
    {
        if (carAnalyzerChip->isBatteryPowered())
        {
            CarAnalyzerLog_e("Impossible to mount SD Card, going to sleep for 10 minutes...");

            esp_sleep_enable_timer_wakeup(600 * 1000000);
            delay(200);

            esp_deep_sleep_start();
        }
        else
        {
            CarAnalyzerLog_e("Impossible to mount SD Card, going to reset chip...");
            ESP.restart();
        }
    }

    carAnalyzerSdCard->checkForUpdate();

    // Initializing object allowing to manage configuration data.
    carAnalyzerConfig = new CarAnalyzerConfig();
    if (carAnalyzerConfig->loadConfiguration(carAnalyzerSdCard->readJsonFile("/config.json")))
    {
        carAnalyzerSdCard->saveFile(carAnalyzerConfig->getData(), "/config.json", true);
    }

    CarAnalyzerLog_d("After configuration Loading:");
    CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
    CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());

    // Initialisation du composant GSM. C'est le composant qui peut mettre le plus de temps à démarrer, alors
    // On l'initialise avant le bluetooth, pour éviter de consommer de la batterie 12V de la voiture pour pas grand chose.
    if (carAnalyzerConfig->getValue("gsmUse").equals("true"))
    {
        updateTimers->gsmTimer = carAnalyzerConfig->getValue("gsmUpdate").toInt();

        carAnalyzerGsm = new CarAnalyzerGsm(MODEM_PWRKEY, MODEM_FLIGHT);

        carAnalyzerGsm->begin(UART_BAUD, MODEM_RX, MODEM_TX);

        carAnalyzerGsm->connect(carAnalyzerConfig->getValue("gsmSimPin").c_str(), carAnalyzerConfig->getValue("gsmApn").c_str(), carAnalyzerConfig->getValue("gsmUser").c_str(), carAnalyzerConfig->getValue("gsmPassword").c_str());
        carAnalyzerGsm->update();

        CarAnalyzerLog_d("After GSM initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    // Il est possible d'utiliser une borne WiFi pour mettre à jour ABRP, HomeAssistant, ...
    // @to be implementing...
    if (carAnalyzerConfig->getValue("wifiUse").equals("true"))
    {
    }

    // On initialise le GPS, si besoin.
    if (carAnalyzerConfig->getValue("gpsUse").equals("true"))
    {
        updateTimers->gpsTimer = carAnalyzerConfig->getValue("gpsUpdate").toInt();

        carAnalyzerGps = new CarAnalyzerGps();

        carAnalyzerGps->begin(UART_BAUD, MODEM_RX, MODEM_TX);
        carAnalyzerGps->setPrecision(carAnalyzerConfig->getValue("gpsPrecision").toInt());
        carAnalyzerGps->connect();

        CarAnalyzerLog_d("After GPS initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    // Si l'utilisateur souhaite communiquer avec Home Assistant, alors on initialise le composant.
    if (carAnalyzerConfig->getValue("homeAssistantUse").equals("true"))
    {
        updateTimers->homeAssistantTimer = carAnalyzerConfig->getValue("homeAssistantUpdate").toInt();

        carAnalyzerHomeAssistant = new CarAnalyzerHomeAssistant(carAnalyzerGsm);
        carAnalyzerHomeAssistant->begin(carAnalyzerConfig->getValue("homeAssistantMqttServerHost").c_str(), carAnalyzerConfig->getValue("homeAssistantMqttServerPort").toInt());

        carAnalyzerHomeAssistant->connect("CarAnalyzer", carAnalyzerConfig->getValue("homeAssistantMqttUsername").c_str(), carAnalyzerConfig->getValue("homeAssistantMqttPassword").c_str());

        CarAnalyzerLog_d("After Home Assistant initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    // Si l'utilisateur veut envoyer les informations à ABRP, alors on initialise le composant.
    if (carAnalyzerConfig->getValue("abrpUse").equals("true"))
    {
        updateTimers->abrpTimer = carAnalyzerConfig->getValue("abrpUpdate").toInt();

        carAnalyzerABRP = new CarAnalyzerABRP(carAnalyzerGsm, carAnalyzerConfig->getValue("abrpServerHost").c_str(), carAnalyzerConfig->getValue("abrpServerPort").toInt());
        carAnalyzerABRP->begin(carAnalyzerConfig->getValue("abrpApiKey").c_str(), carAnalyzerConfig->getValue("abrpToken").c_str(), carAnalyzerConfig->getValue("abrpCarModel").c_str());

        CarAnalyzerLog_d("After ABRP initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }

    if (carAnalyzerConfig->getValue("bluetoothUse").equals("true"))
    {
        // If the user requested to use BT, we start bluetooth device.

        // First, we have to initialize dedicated component.
        // This component manages bluetoothSerial and ELMDuino objects.
        carAnalyzerObd = new CarAnalyzerObd(BT_PIN_CTRL);

        // Now, we can begin to use component.
        carAnalyzerObd->begin(carAnalyzerConfig->getValue("bluetoothAddress").c_str(), carAnalyzerConfig->getValue("bluetoothPin").c_str(), carAnalyzerConfig->getValue("bluetoothName").c_str());
        carAnalyzerObd->loadConfiguration(carAnalyzerSdCard->readJsonFile(carAnalyzerConfig->getValue("obdCarConfiguration").c_str()));

        // Because it's not possible to determine if bluetooth OBD is switched on,
        // we have to try to connect.
        if (!carAnalyzerObd->connect())
        {
            CarAnalyzerLog_e("Connection to OBD failed.");
        }

        CarAnalyzerLog_d("After OBD initialization:");
        CarAnalyzerLog_d("Internal Total heap %u, internal Free Heap %u", ESP.getHeapSize(), ESP.getFreeHeap());
        CarAnalyzerLog_d("PSRam Total heap %u, PSRam Free Heap %u", ESP.getPsramSize(), ESP.getFreePsram());
    }
}

void loop(void)
{
    carAnalyzerChip->update();

    if (carAnalyzerConfig->getValue("gsmUse").equals("true") && (carAnalyzerChip->isBatteryPowered() || millis() - carAnalyzerGsm->getLastUpdate() > updateTimers->gsmTimer))
    {
        carAnalyzerGsm->update();

        if (carAnalyzerConfig->getValue("homeAssistantUse").equals("true") && !carAnalyzerGsm->getData().isNull())
        {
            if (!carAnalyzerHomeAssistant->publish("GSM", carAnalyzerGsm->getData()))
            {
                CarAnalyzerLog_w("Home Assistant Connection lost.");
                carAnalyzerGsm->reconnect();
            }
        }
    }

    if (carAnalyzerConfig->getValue("bluetoothUse").equals("true") && (carAnalyzerChip->isBatteryPowered() || millis() - carAnalyzerObd->getLastUpdate() > 2000))
    {
        carAnalyzerObd->update();

        if (!carAnalyzerObd->getDataRaw().isNull()) {
            carAnalyzerSdCard->updateFile(carAnalyzerObd->getDataRaw(), "/OBD_data_raw.txt", FILE_APPEND, true);
        }

        if (!carAnalyzerObd->getData().isNull() && carAnalyzerConfig->getValue("homeAssistantUse").equals("true"))
        {
            if (!carAnalyzerHomeAssistant->publish("Car", carAnalyzerObd->getData()))
            {
                CarAnalyzerLog_w("Home Assistant Connection lost.");
                carAnalyzerGsm->reconnect();
            }
        }
    }

    if (carAnalyzerConfig->getValue("gpsUse").equals("true") && (carAnalyzerChip->isBatteryPowered() || millis() - carAnalyzerGps->getLastUpdate() > updateTimers->gpsTimer))
    {
        
        carAnalyzerGps->update();

        if (!carAnalyzerGps->getData().isNull() && carAnalyzerConfig->getValue("homeAssistantUse").equals("true"))
        {
            if (!carAnalyzerHomeAssistant->publish("GPS", carAnalyzerGps->getData()))
            {
                CarAnalyzerLog_w("Home Assistant Connection lost.");
                carAnalyzerGsm->reconnect();
            }
        }
        else
        {
            CarAnalyzerLog_i("GPS data is not valid, or Home Assistant is not used.");
        }
    }


    if (carAnalyzerConfig->getValue("homeAssistantUse").equals("true") && (carAnalyzerChip->isBatteryPowered() || millis() - t > updateTimers->homeAssistantTimer))
    {
        t = millis();
        if (!carAnalyzerHomeAssistant->publish("Chip", carAnalyzerChip->getData()))
        {
            CarAnalyzerLog_w("Home Assistant Connection lost.");
            carAnalyzerGsm->reconnect();
        }
    }

    if (carAnalyzerConfig->getValue("abrpUse").equals("true") && (carAnalyzerChip->isBatteryPowered() || millis() - carAnalyzerABRP->getLastUpdate() > updateTimers->abrpTimer))
    {
        JsonObject gpsData;
        JsonObject carData;

        if (carAnalyzerConfig->getValue("gpsUse").equals("true"))
        {
            gpsData = carAnalyzerGps->getData();
        }

        if (carAnalyzerConfig->getValue("bluetoothUse").equals("true"))
        {
            carData = carAnalyzerObd->getData();
        }

        if (!gpsData.isNull() && !carData.isNull())
        {
            carAnalyzerABRP->publish(gpsData, carData);
        }
    }

    if (carAnalyzerChip->isBatteryPowered())
    {
        // Si on est sur batterie, alors on s'assure d'éteindre l'adaptateur OBD.
        if (carAnalyzerConfig->getValue("bluetoothUse").equals("true"))
        {
            carAnalyzerObd->switchOff();
        }

        // Puis on coupe l'ESP32 pour la durée demandée.
        CarAnalyzerLog_d("Deep sleep mode for %ld seconds... Good night!", carAnalyzerConfig->getValue("chipDeepSleepDuration").toInt());

        esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 1);
        esp_sleep_enable_timer_wakeup(carAnalyzerConfig->getValue("chipDeepSleepDuration").toInt() * 1000000);
        delay(200);

        esp_deep_sleep_start();
    }
}
