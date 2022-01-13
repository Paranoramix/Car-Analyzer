#include "CarAnalyzerObd.h"

/**
 * @brief Construct a new Car Analyzer Obd:: Car Analyzer Obd object.
 * 
 */
CarAnalyzerObd::CarAnalyzerObd(uint8_t pinControl)
{
    psramInit();

    this->_btSerial = (BluetoothSerial *)ps_malloc(sizeof(BluetoothSerial));
    this->_btSerial = new BluetoothSerial();
    this->_elm = (ELM327 *)ps_malloc(sizeof(ELM327));
    this->_elm = new ELM327();
    this->_lastUpdate = millis();

    this->_data = new SpiRamJsonDocument(10000);
    this->_dataRaw = new SpiRamJsonDocument(10000);

    this->_configuration = new SpiRamJsonDocument(50000);

    this->_elmPinControl = pinControl;

    // Set pin as output.
    pinMode(this->_elmPinControl, OUTPUT);
}

/**
 * @brief Initialize internal variables.
 * 
 * @param pinControl GPIO to switch ON/OFF the device.
 * @param macAddress Mac Address to connect to the device.
 * @param pin Pin code to connect to the device.
 * @param name A name for bluetooth connection.
 */
bool CarAnalyzerObd::begin(const char *macAddress, const char *pin = "1234", const char *name = "Car-Analyzer")
{
    this->_name = name;
    this->_pin = pin;

    sscanf(macAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &this->_macAddress[0], &this->_macAddress[1], &this->_macAddress[2], &this->_macAddress[3], &this->_macAddress[4], &this->_macAddress[5]);

    return this->switchOn() && this->_btSerial->begin(this->_name, true);
}

/**
 * @brief Turn on the device.
 * 
 */
bool CarAnalyzerObd::switchOn(void)
{
    digitalWrite(this->_elmPinControl, HIGH);

    return digitalRead(this->_elmPinControl);
}

/**
 * @brief Turn off the device.
 * 
 */
bool CarAnalyzerObd::switchOff(void)
{
    digitalWrite(this->_elmPinControl, LOW);

    return digitalRead(this->_elmPinControl);
}

/**
 * @brief Connect to the OBD device.
 * 
 * @return true if the device is totally connected and initialized.
 * @return false if the device is not connected and/or initialized.
 */
bool CarAnalyzerObd::connect(void)
{
    this->switchOn();

    if (!this->_btSerial->connected())
    {
        // Trying to connect.
        this->_btSerial->end();
        this->_btSerial->begin(this->_name, true);
        this->_btSerial->disconnect();
        this->_btSerial->unpairDevice(this->_macAddress);

        this->_btSerial->setPin(this->_pin);

        if (!this->_btSerial->connect(this->_macAddress))
        {
            return false;
        }
        else
        {
            if (!this->_elm->begin(*this->_btSerial, false, 1000, '6', 500))
            {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief Disconnect and switch off the OBD device.
 * 
 * @return true the device has been switched off.
 * @return false the device is always switched on. 
 */
bool CarAnalyzerObd::disconnect(void)
{
    this->switchOff();

    this->_btSerial->disconnect();
    this->_btSerial->unpairDevice(this->_macAddress);

    return !digitalRead(this->_elmPinControl);
}

void CarAnalyzerObd::end(void)
{
    this->disconnect();

    this->_btSerial->end();
}

uint32_t CarAnalyzerObd::getLastUpdate(void)
{
    return this->_lastUpdate;
}

bool CarAnalyzerObd::loadConfiguration(JsonObject configuration)
{
    String r;
    serializeJsonPretty(configuration, r);
    CarAnalyzerLog_d("%s", r.c_str());
    return this->_configuration->set(configuration);
}

bool CarAnalyzerObd::update(void)
{
    this->_data->clear();
    this->_dataRaw->clear();

    if (!this->_btSerial->connected())
    {
        return false;
    }

    CarAnalyzerLog_v("Updating Car OBD informations");

    this->_lastUpdate = millis();

    // A partir du fichier de configuration, nous allons extraire les données de l'OBD
    for (JsonPair kv : this->_configuration->as<JsonObject>())
    {
        if (String(kv.key().c_str()).equals("CALCULATED"))
        {
            // On traite des données qui sont calculées, et qui ne sont pas retournées par l'OBD.
            for (JsonPair parameter : kv.value().as<JsonObject>())
            {
                String formula = parameter.value().as<JsonObject>()["formula"].as<String>();
                JsonArray vars = parameter.value().as<JsonObject>()["vars"].as<JsonArray>();

                CarAnalyzerLog_d("Calcul %s: %s", parameter.key().c_str(), formula.c_str());

                String homeAssistantSuffix = "";
                if (parameter.value().as<JsonObject>().containsKey("HomeAssistantParameter"))
                {
                    homeAssistantSuffix = parameter.value().as<JsonObject>()["HomeAssistantParameter"].as<String>();
                }

                for (JsonVariant value : vars)
                {
                    if (this->_data->containsKey(value.as<String>()))
                    {
                        formula.replace(value.as<String>(), String((*this->_data)[value.as<String>()].as<double>()));
                    }
                }

                CarAnalyzerLog_d("Calcul: %s", formula.c_str());

                // Maintenant on lance le calcul
                int err;
                double result = te_interp(formula.c_str(), &err);

                if (err == 0)
                {
                    (*this->_data)[parameter.key().c_str() + homeAssistantSuffix] = result;
                }
            }
        }
        else
        {
            for (JsonPair kvc : kv.value().as<JsonObject>())
            {
                // Maintenant, on peut faire la requête auprès de l'ODB
                //               if (this->readCarData(kv.key().c_str(), kvc.key().c_str(), 0))
                {
                    //                    String data = String(this->_elm->payload);

                    String data = "";

                    if (String(kvc.key().c_str()).equals("220101"))
                    {
                        data = "03E0:620101EFFBE71:EFB500000000002:FF9D1D2A120C0E3:110D0E0D0036CF4:90CFA7000078005:008F5B00008C996:000064990000607:7D0050F67000008:06000000000BB8";
                    }

                    if (String(kvc.key().c_str()).equals("220105"))
                    {
                        data = "02E0:620105FFFB741:0F012C01012C0D2:0C0D0F0E0E0E163:AD62D40000640E4:0003E84484F9005:C80000000000006:000F0E1012AAAA";
                    }

                    if (String(kvc.key().c_str()).equals("220106"))
                    {
                        data = "0260:62010617F8111:000C000D0000002:000000000000003:0000000600EA004:000000000000005:00000000AAAAAA";
                    }

                    if (String(kvc.key().c_str()).equals("220100"))
                    {
                        data = "0270:6201007F94071:C8FF71606201EF2:8E01EFFFFF0FFF3:B3FFFFFFFFFFFF4:FF3255796F00FF5:FF00FFFFFFAAAA";
                    }

                    if (String(kvc.key().c_str()).equals("22C00B"))
                    {
                        data = "0240:62C00BFFFFFF1:80B638010000B42:37010000B639013:0000B7390100004:FF97B097B098B05:98B0AAAAAAAAAA";
                    }

                    if (String(kvc.key().c_str()).equals("22B002"))
                    {
                        data = "00F0:62B002E000001:00FFAF0025E9002:0000AAAAAAAAAA";
                    }

                    (*this->_dataRaw)[kvc.key().c_str()] = data;

                    // On va nettoyer la chaine de caractères pour traiter les données.
                    data.replace("0:", "");
                    data.replace("1:", "");
                    data.replace("2:", "");
                    data.replace("3:", "");
                    data.replace("4:", "");
                    data.replace("5:", "");
                    data.replace("6:", "");
                    data.replace("7:", "");
                    data.replace("8:", "");
                    data.replace("SEARCHING", "");

                    // La chaine est normalement nettoyée, on va pouvoir lancer le traitement des données.
                    for (JsonPair parameter : kvc.value().as<JsonObject>())
                    {
                        // Et voilà, on analyse les clés fournies dans le paramètre, et on en déduit les traitements à réaliser.
                        int startAt = parameter.value().as<JsonObject>()["start"].as<int>();
                        int length = parameter.value().as<JsonObject>()["length"].as<int>();
                        String formula = parameter.value().as<JsonObject>()["formula"].as<String>();
                        String homeAssistantSuffix = "";
                        if (parameter.value().as<JsonObject>().containsKey("HomeAssistantParameter"))
                        {
                            homeAssistantSuffix = parameter.value().as<JsonObject>()["HomeAssistantParameter"].as<String>();
                        }

                        double x;
                        int err;

                        te_variable vars[] = {{"x", &x}};
                        te_expr *expr = te_compile(formula.c_str(), vars, 1, &err);

                        x = this->extractCarData(data.c_str(), startAt, length);
                        if (expr)
                        {
                            // Si la donnée a été calculée, alors on peut mettre à jour notre _data
                            (*this->_data)[parameter.key().c_str() + homeAssistantSuffix] = te_eval(expr);
                        }

                        te_free(expr);
                    }
                    //                } else {
                    CarAnalyzerLog_w("Impossible to read data from OBD: %s, %s", kv.key().c_str(), kvc.key().c_str());
                }
            }
        }
    }
    return true;
}

bool CarAnalyzerObd::readCarData(const char *header, const char *query, uint8_t lengthPayloadAttempted = 0)
{

    int8_t statusHeader = this->_elm->sendCommand(header);
    if (statusHeader != ELM_SUCCESS)
    {
        CarAnalyzerLog_w("Error when sending header %s: %d", header, statusHeader);
        return false;
    }

    delay(10);

    int8_t statusQuery = this->_elm->sendCommand(query);
    if (statusQuery != ELM_SUCCESS)
    {
        CarAnalyzerLog_w("Error when sending query %s: %d", query, statusQuery);

        if (statusQuery == ELM_GENERAL_ERROR)
        {
            this->_btSerial->disconnect();
        }

        return false;
    }

    delay(500);

    if (lengthPayloadAttempted > 0 && strlen(this->_elm->payload) != lengthPayloadAttempted)
    {
        CarAnalyzerLog_w("Payload Length error: %d (attempted: %d)", strlen(this->_elm->payload), lengthPayloadAttempted);

        return false;
    }

    return true;
}

/**
 * @brief Extrait une valeur depuis une trame récupérée par l'ODB.
 * 
 * @param source trame OBD2
 * @param position position à partir de laquelle extraire la valeur.
 * @param length  longueur de la valeur.
 * @return long 
 */
double CarAnalyzerObd::extractCarData(const char *source, uint8_t position, uint8_t length)
{
    if (strlen(source) >= position + length + 1)
    {
        char *value = NULL;
        char *endPtr;

        value = (char *)ps_malloc((length + 1) * sizeof(char));

        memcpy(value, &source[position], length);
        value[length] = '\0';

        long r = strtol(value, &endPtr, HEX);
        ;
        free(value);
        return r;
    }
    else
    {
        return NAN;
    }
}

JsonObject CarAnalyzerObd::getData(void)
{
    return (*this->_data).as<JsonObject>();
}

JsonObject CarAnalyzerObd::getDataRaw(void)
{
    return (*this->_dataRaw).as<JsonObject>();
}
