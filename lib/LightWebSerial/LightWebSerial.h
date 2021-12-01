#ifndef _LIGHT_WEB_SERIAL_H
#define _LIGHT_WEB_SERIAL_H

#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

class LightWebSerialClass {
    public:
        void begin(AsyncWebServer *server, const char * separator = "\n", const char* url = "/logs");

        void print(String message);
        void print(const char* message);
        void print(char* message);
        void print(int message);
        void print(long message);
        void print(float message);
        void print(double message);
        void print(uint8_t message);
        void print(uint16_t message);
        void print(uint32_t message);

        void println(String message);
        void println(const char* message);
        void println(char* message);
        void println(int message);
        void println(long message);
        void println(float message);
        void println(double message);
        void println(uint8_t message);
        void println(uint16_t message);
        void println(uint32_t message);

    private:
        AsyncWebServer *_server;
        AsyncWebSocket *_webSocket;
        const char* _separator;
};

extern LightWebSerialClass LightWebSerial;

#endif