#ifndef _SD_CONFIG_FILE_H
#define _SD_CONFIG_FILE_H

class SdConfigfileClass {
  public:
    boolean begin(const char* configFileName);
    void end(void);
    boolean nameIs(const char name);
    
  
  private:
    File _file;
    boolean _atEnd;
    char* _line;
    
    uint8_t _lineSize;
    uint8_t _lineLength;
    uint8_t _valueIndex;
};

#endif
