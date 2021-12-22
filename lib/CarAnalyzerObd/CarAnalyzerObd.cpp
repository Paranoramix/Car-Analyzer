#include "CarAnalyzerObd.h"

/**
 * @brief Construct a new Car Analyzer Obd:: Car Analyzer Obd object.
 * 
 */
CarAnalyzerObd::CarAnalyzerObd(void)
{
    psramInit();

    this->_btSerial = (BluetoothSerial *)ps_malloc(sizeof(BluetoothSerial));
    this->_btSerial = new BluetoothSerial();
    this->_elm = (ELM327 *)ps_malloc(sizeof(ELM327));
    this->_elm = new ELM327();
}

/**
 * @brief Initialize internal variables.
 * 
 * @param pinControl GPIO to switch ON/OFF the device.
 * @param statePin GPIO to detect if device is connected or not.
 * @param macAddress Mac Address to connect to the device.
 * @param pin Pin code to connect to the device.
 * @param name A name for bluetooth connection.
 */
void CarAnalyzerObd::begin(uint8_t pinControl, uint8_t statePin, const char *macAddress, const char *pin = "1234", const char *name = "Car-Analyzer")
{
    this->_elmPinControl = pinControl;
    this->_elmPinState = statePin;
    this->_name = name;
    this->_pin = pin;

    sscanf(macAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &this->_macAddress[0], &this->_macAddress[1], &this->_macAddress[2], &this->_macAddress[3], &this->_macAddress[4], &this->_macAddress[5]);

    // Set pin as output.
    pinMode(this->_elmPinControl, OUTPUT);
    pinMode(this->_elmPinState, INPUT);

    digitalWrite(this->_elmPinControl, HIGH);

    this->_btSerial->begin(this->_name, true);

    // We try to connect component.
    if (!digitalRead(this->_elmPinState))
    {
        digitalWrite(this->_elmPinControl, LOW);
        delay(100);
        digitalWrite(this->_elmPinControl, HIGH);
        delay(500);
    }

    return;
}

/**
 * @brief Turn on the device.
 * 
 */
void CarAnalyzerObd::switchOn(void)
{
    if (!digitalRead(this->_elmPinState))
    {
        digitalWrite(this->_elmPinControl, LOW);
        delay(100);
        digitalWrite(this->_elmPinControl, HIGH);
        delay(500);
    }
}

/**
 * @brief Turn off the device.
 * 
 */
void CarAnalyzerObd::switchOff(void)
{
    if (digitalRead(this->_elmPinState))
    {
        digitalWrite(this->_elmPinControl, LOW);
        delay(100);
        digitalWrite(this->_elmPinControl, HIGH);
        delay(500);
    }
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
            if (!this->_elm->begin(*this->_btSerial, true, 5000, '6', 500))
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

    return !digitalRead(this->_elmPinState);
}

void CarAnalyzerObd::end(void)
{
    this->disconnect();

    this->_btSerial->end();

}