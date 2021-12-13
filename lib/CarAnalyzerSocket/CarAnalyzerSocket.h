#ifndef _CAR_ANALYZE_SOCKET_H
#define _CAR_ANALYZE_SOCKET_H

#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>

class CarAnalyzerSocketClass {
    public:
        void begin(AsyncWebServer *server, const char * name);

        void send(JsonDocument& doc);
        void send(String message);

        
    private:
        AsyncWebServer *_server;
        AsyncWebSocket *_webSocket;
        String _name;
        uint8_t _client;

};

#endif