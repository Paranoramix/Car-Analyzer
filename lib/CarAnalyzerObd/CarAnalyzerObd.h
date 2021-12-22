#ifndef _CAR_ANALYZER_OBD_H
#define _CAR_ANALYZER_OBD_H

#include <BluetoothSerial.h>
#include <ELMduino.h>

class CarAnalyzerObd {
    private:
        BluetoothSerial *_btSerial;
        ELM327 *_elm;

        uint8_t _elmPinControl;
        uint8_t _elmPinState;
        uint8_t _macAddress[6];

        const char* _name;
        const char* _pin;
        
    public:
        CarAnalyzerObd(void);

        void begin(uint8_t, uint8_t, const char* macAddress, const char*, const char*);
        bool connect(void);
        bool disconnect(void);

        void switchOff(void);
        void switchOn(void);

        void end(void);
};

#endif