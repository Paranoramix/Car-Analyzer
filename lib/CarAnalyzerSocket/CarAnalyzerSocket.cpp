#include "CarAnalyzerSocket.h"


void CarAnalyzerSocketClass::begin(AsyncWebServer *server, const char* name) {
    this->_name = String(name);

    this->_server = server;
    this->_webSocket = new AsyncWebSocket("/" + this->_name + "_ws");

    this->_webSocket->onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) -> void {
        Serial.print(millis());
        if(type == WS_EVT_CONNECT){
            Serial.println(" Client connection received [socket: " + this->_name + "]");
        } else if(type == WS_EVT_DISCONNECT){
            Serial.println(" Client disconnected [socket: " + this->_name + "]");
        } else if(type == WS_EVT_DATA){
            Serial.println(" Received Websocket Data [socket: " + this->_name + "]");
        }
    });

    this->_server->addHandler(this->_webSocket);
}

void CarAnalyzerSocketClass::send(JsonDocument& document) {
    String message;
    serializeJson(document, message);
    this->send(message);
}

void CarAnalyzerSocketClass::send(String message) {
    this->_webSocket->textAll(message);
}