#include "LightWebSerial.h"



void LightWebSerialClass::begin(AsyncWebServer *server, const char* separator, const char* url) {
    Serial.println(url);
    this->_server = server;
    this->_webSocket = new AsyncWebSocket("/logs_ws");
    this->_separator = separator;

    this->_webSocket->onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) -> void {
        Serial.print(millis());
        if(type == WS_EVT_CONNECT){
            Serial.println(" Client connection received");
        } else if(type == WS_EVT_DISCONNECT){
            Serial.println(" Client disconnected");
        } else if(type == WS_EVT_DATA){
            Serial.println(" Received Websocket Data");
        }
    });

    this->_server->addHandler(this->_webSocket);
}

void LightWebSerialClass::print(String message) {
    this->_webSocket->textAll(message);
}

void LightWebSerialClass::print(const char* message) {
    this->print(String(message));
}

void LightWebSerialClass::print(char* message) {
    this->print(String(message));
}

void LightWebSerialClass::print(int message) {
    this->print(String(message));
}

void LightWebSerialClass::print(long message) {
    this->print(String(message));
}

void LightWebSerialClass::print(float message) {
    this->print(String(message));
}

void LightWebSerialClass::print(double message) {
    this->print(String(message));
}

void LightWebSerialClass::print(uint8_t message) {
    this->print(String(message));
}

void LightWebSerialClass::print(uint16_t message) {
    this->print(String(message));
}

void LightWebSerialClass::print(uint32_t message) {
    this->print(String(message));
}

void LightWebSerialClass::println(String message) {
    this->print(message + String(this->_separator));
}

void LightWebSerialClass::println(const char* message) {
    this->println(String(message));
}

void LightWebSerialClass::println(char* message) {
    this->println(String(message));
}

void LightWebSerialClass::println(int message) {
    this->println(String(message));
}

void LightWebSerialClass::println(long message) {
    this->println(String(message));
}

void LightWebSerialClass::println(float message) {
    this->println(String(message));
}

void LightWebSerialClass::println(double message) {
    this->println(String(message));
}

void LightWebSerialClass::println(uint8_t message) {
    this->println(String(message));
}

void LightWebSerialClass::println(uint16_t message) {
    this->println(String(message));
}

void LightWebSerialClass::println(uint32_t message) {
    this->println(String(message));
}