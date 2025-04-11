
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <zlib.h>

using namespace std;

#ifndef CREATEPNG_H
#define CREATEPNG_H

#ifdef CREATEPNG_EXPORT
#define CREATEPNG_API __declspec(dllexport)
#else
#define CREATEPNG_API __declspec(dllimport)
#endif

class CREATEPNG_API createPNG
{
private:
    
    // auxilary funtion which are help to create PNG file

    // write 4 bytes to file
    // this func write this hexa value 00 00 00 0D ---> 0D 00 00 00
    void write4bytes(ofstream& file, uint32_t value);

    // CRC calculating algorithem
    uint32_t calculateCRC(vector<uint8_t>& rawData);

    // Compress the raw pixel data using DEFLATE algorithem
    vector<unsigned char> compressData(vector<unsigned char>& uncompressedData);

    // The PNG format requires that each scanline (row of pixel data) starts with a filter type byte. The filter type tells the PNG decoder how the data in the scanline has been processed. For simplicity, you can use a filter type of 0 (no filtering).

    //     What to Add:
    //         Prepend each scanline (row of pixels) with a single byte 0x00.
    //         If your image is 300 pixels wide and has 3 channels (RGB), each scanline is 300 * 3 = 900 bytes. Add 1 byte at the start of each scanline.

    //     Modified Structure:

    // [Filter Type Byte][Scanline Data][Filter Type Byte][Scanline Data]...

    // this funtion add filer bytes to the uncompressed data
    void addFilterbytes(vector<unsigned char>& uncompressedData, int width, int height);

    // ---------------------------------end of auxilety func section ----------------------------------------

    const char* idatFileName;
    int width;
    int height;

public:

    createPNG(const char* rawDataFile, int width, int height, const char* outPutFile);

    // write the signature to the given out put stream
    void writeSignature(ofstream& file);

    // write IHDR chunck to the given out put stream
    // IHDR contain the besic data about the PNG file like width height
    void writeIHDR(ofstream& file);

    // write PLTE chunck to the given out put stream
    // PLTE chunck contain about color details
    void writePLTE(ofstream& file);

    // write IDAT chunck to the given out put stream
    // IDAT chunck contain the compressed pixel data
    void writeIDAT(ofstream& file);

    // write IEND chunck to the given out put stream
    // IEND chunck indicate that the PNG file end
    void writeIEND(ofstream& file);

};



#endif