#include "CarAnalyzerSdCard.h"

CarAnalyzerSdCard::CarAnalyzerSdCard(uint8_t sclk, int8_t miso, int8_t mosi, int8_t cs)
{
    SPI.begin(sclk, miso, mosi, cs);

    this->_data = new SpiRamJsonDocument(200);

    if (!SD.begin(cs))
    {
        CarAnalyzerLog_d("Cart mount failed!");
        (*this->_data)["mounted"] = false;

        return;
    }

    (*this->_data)["mounted"] = true;
    (*this->_data)["cardType"] = SD.cardType();
    (*this->_data)["cardSize"] = SD.cardSize() / (1024 * 1024);

    this->_lastUpdate = millis();
}

bool CarAnalyzerSdCard::isMounted(void)
{
    return (*this->_data)["mounted"].as<bool>();
}

uint32_t CarAnalyzerSdCard::getLastUpdate(void)
{
    return this->_lastUpdate;
}

JsonObject CarAnalyzerSdCard::readJsonFile(const char* filename) {
    SpiRamJsonDocument* document = new SpiRamJsonDocument(5000);


    if (!this->isMounted()) {
        CarAnalyzerLog_w("read file is not possible: SD Card is not mounted");

        return document->as<JsonObject>();
    }

    File file = SD.open(filename);

    deserializeJson((*document), file);

    return document->as<JsonObject>();
}

bool CarAnalyzerSdCard::saveFile(JsonObject document, const char* filename, bool backup = false) {
    if (!this->isMounted()) {
        CarAnalyzerLog_w("save file is not possible: SD Card is not mounted");

        return false;
    }

    
    // Delete existing file, otherwise the configuration is appended to the file

    if (backup) {
        File myFileIn = SD.open(filename, FILE_READ);
        File myFileOut = SD.open(String(filename) + ".bkp", FILE_WRITE);
        while (myFileIn.available())
        {
            myFileOut.write(myFileIn.read());
        }

        myFileIn.close();
        myFileOut.close();
    }

    // Open file for writing
    File file = SD.open(filename, FILE_WRITE);
    if (!file)
    {
        CarAnalyzerLog_e("Failed to create file");
        return false;
    }

    if (serializeJsonPretty(document, file) == 0)
    {
        CarAnalyzerLog_e("Failed to write to file");
    }

    CarAnalyzerLog_d("File saved");

    // Close the file
    file.close();

}