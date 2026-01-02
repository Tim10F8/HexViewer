#ifndef HEXDATA_H
#define HEXDATA_H

#include <stdint.h>
#include <stddef.h>


struct SimpleString {
    char*  data;
    size_t length;
    size_t capacity;
};

struct ByteBuffer {
    uint8_t* data;
    size_t   size;
    size_t   capacity;
};

struct LineArray {
    SimpleString* lines;
    size_t        count;
    size_t        capacity;
};

class HexData {
public:
    HexData();
    ~HexData();

    bool loadFile(const char* filepath);
    bool saveFile(const char* filepath);
    void clear();

    const LineArray& getHexLines() const        { return hexLines; }
    const LineArray& getDisassemblyLines() const { return disassemblyLines; }
    const SimpleString& getHeaderLine() const   { return headerLine; }
    size_t getFileSize() const                  { return fileData.size; }
    bool isEmpty() const                        { return fileData.size == 0; }
    int getCurrentBytesPerLine() const          { return currentBytesPerLine; }

    bool editByte(size_t offset, uint8_t newValue);
    uint8_t getByte(size_t offset) const;
    uint8_t readByte(size_t offset) const       { return getByte(offset); }

    bool isModified() const                     { return modified; }
    void setModified(bool mod)                  { modified = mod; }

    void regenerateHexLines(int bytesPerLine);
    
    void setArchitecture(int arch, int mode);

private:
    void convertDataToHex(int bytesPerLine);
    void generateHeader(int bytesPerLine);
    void generateDisassembly(int bytesPerLine);
    void disassembleInstruction(size_t offset, int& instructionLength, SimpleString& outInstr);
    
    bool initializeCapstone();
    void cleanupCapstone();

private:
    ByteBuffer   fileData;
    LineArray    hexLines;
    LineArray    disassemblyLines;
    SimpleString headerLine;

    int   currentBytesPerLine;
    bool  modified;
    bool  capstoneInitialized;
    
    int currentArch;
    int currentMode;
    size_t csHandle;
};

#endif
