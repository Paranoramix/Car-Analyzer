#ifndef _CAR_CONFIG_FILE_H
#define _CAR_CONFIG_FILE_H

#include <Arduino.h>
#include <SD.h>

class CarConfigFile {
    public:

        boolean begin(char *);
        
        void end(void);

        void getParameter(const char*, char*&);
        void getParameter(const char*, int&);
        void getParameter(const char*, float&);
        void getParameter(const char*, bool&);
        
        boolean readNextSetting();
        boolean nameIs(const char *);

        const char *getName();
        const char *getValue();
        int getIntValue();
        boolean getBooleanValue();
        
        char *copyValue();

    private:
        File _file;
        boolean *_atEnd = (boolean *)ps_malloc(sizeof(boolean));
        char *_line;
                                
        uint8_t *_lineSize = (uint8_t *)ps_malloc(sizeof(uint8_t));
        uint8_t *_lineLength = (uint8_t *)ps_malloc(sizeof(uint8_t));
        uint8_t *_valueIdx = (uint8_t *)ps_malloc(sizeof(uint8_t));
};

#endif