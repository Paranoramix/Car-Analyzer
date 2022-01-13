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

JsonObject CarAnalyzerSdCard::readJsonFile(const char *filename)
{
    SpiRamJsonDocument *document = new SpiRamJsonDocument(50000);

    if (!this->isMounted())
    {
        CarAnalyzerLog_w("read file is not possible: SD Card is not mounted");

        return document->as<JsonObject>();
    }

    File file = SD.open(filename);

    deserializeJson((*document), file);

    return document->as<JsonObject>();
}


bool CarAnalyzerSdCard::saveFile(JsonObject document, const char *filename, bool backup = false)
{
    return updateFile(document, filename, "w", backup);
}

bool CarAnalyzerSdCard::updateFile(JsonObject document, const char *filename, const char * mode, bool backup = false)
{
    if (!this->isMounted())
    {
        CarAnalyzerLog_w("save file is not possible: SD Card is not mounted");

        return false;
    }

    // Delete existing file, otherwise the configuration is appended to the file

    if (backup)
    {
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
    File file = SD.open(filename, mode);
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

    return true;
}

void CarAnalyzerSdCard::checkForUpdate(void)
{
    File updateBin = SD.open("/firmware.bin");

    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            CarAnalyzerLog_e("Error, update.bin is not a file");
            updateBin.close();
            return;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0)
        {
            CarAnalyzerLog_d("Try to start update");
            if (Update.begin(updateSize))
            {
                size_t written = Update.writeStream(updateBin);
                if (written == updateSize)
                {
                    CarAnalyzerLog_d("Written : %s successfully", String(written).c_str());
                }
                else
                {
                    CarAnalyzerLog_e("Written only : %s/%s. Retry?", String(written).c_str(), String(updateSize).c_str());
                }
                if (Update.end())
                {
                    CarAnalyzerLog_d("OTA done!");
                    if (Update.isFinished())
                    {
                        CarAnalyzerLog_d("Update successfully completed. Rebooting.");
                    }
                    else
                    {
                        CarAnalyzerLog_e("Update not finished? Something went wrong!");
                    }
                }
                else
                {
                    CarAnalyzerLog_e("Error Occurred. Error #: %s", String(Update.getError()).c_str());
                }
            }
            else
            {
                CarAnalyzerLog_e("Not enough space to begin OTA");
            }
        }
        else
        {
            CarAnalyzerLog_e("Error, file is empty");
        }

        updateBin.close();

        // whe finished remove the binary from sd card to indicate end of the process
        SD.remove("/firmware.bin");

        delay(5000);
        ESP.restart();
    }
    else
    {
        CarAnalyzerLog_d("No update file... Continue!");
    }
}