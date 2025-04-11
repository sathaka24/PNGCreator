#include "createPNG.h"

void createPNG::write4bytes(ofstream &file, uint32_t value)
{
    file.put((value >> 24) & 0xFF);
    file.put((value >> 16) & 0xFF);
    file.put((value >> 8) & 0xFF);
    file.put(value & 0xFF);
}

uint32_t createPNG::calculateCRC(vector<uint8_t> &rawData)
{
    uint32_t CRC = 0xFFFFFFFF; // Initialize CRC. all bits are set to 1

    // CRC-32, the fixed polynomial is: 0x04C11DB7 (binary: 100110000010001110110110111)
    // But in actual calculations, we use its reflected version: 0xEDB88320 (binary: 11101101101110001000001100100000)

    const uint32_t polynomial = 0xEDB88320;

    for (uint8_t byte : rawData){
        CRC ^= byte;  // perform XOR with the current byte and CRC

        for (int i = 0; i < 8; i++){ // iterate through all 8 bits

            if (CRC & 1){ // check if LSB is 1 
                CRC = (CRC >> 1) ^ polynomial;  // if it is then shift 1 bit right and XOR with polynomial
            }
            else {
                CRC >>= 1; // if not just shift 1 bit right
            }
        }
        
    }

    return ~CRC; // reverse the calculated CRC and return it
}

vector<unsigned char> createPNG::compressData(vector<unsigned char> &uncompressedData)
{
    vector<unsigned char> compressedData;

    z_stream zs = {};

    zs.next_in = uncompressedData.data();
    zs.avail_in = uncompressedData.size();

    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
        throw runtime_error("z_stream compression initialize error");
    }

    int chunckSize = 4096;
    vector<unsigned char> buffer(chunckSize);

    int result;

    do{
        
        zs.next_out = buffer.data();
        zs.avail_out = buffer.size();

        result = deflate(&zs, Z_FINISH); // compress the data. Z_FINISH indicate the compression do to the whole data

        if(result == Z_STREAM_ERROR){
            deflateEnd(&zs);
            throw runtime_error("compression error!");
        }

        compressedData.insert(compressedData.end(), buffer.data(), buffer.data() + (buffer.size() - zs.avail_out)); // here is good explanation about this part https://chatgpt.com/share/6762db2b-d108-8008-911d-3923653c406f

    } while (zs.avail_out == 0);

    deflateEnd(&zs);

    cout << "compression sucessful" << endl;

    return compressedData;
}

void createPNG::addFilterbytes(vector<unsigned char> &uncompressedData, int width, int height)
{
    vector<unsigned char> tempvector;

    int rowSize = width * 3;

    for (int i = 0; i < height; i++)
    {
        tempvector.push_back(0x00);
        tempvector.insert(tempvector.end(), uncompressedData.begin() + (i * rowSize), uncompressedData.begin() + ((i + 1)* rowSize));
    }

    uncompressedData = move(tempvector); // Replace the original data with the modified data
}

createPNG::createPNG(const char *rawDataFile, int width, int height, const char* outPutFile)
{
    this->idatFileName = rawDataFile;
    this->width = width;
    this->height = height;

    ofstream file(outPutFile, ios::binary);

    writeSignature(file);
    cout << "signature done" << endl;
    writeIHDR(file);
    cout << "IHDR done" << endl;
    writePLTE(file);
    cout << "PLTE done" << endl;
    writeIDAT(file);
    cout << "IDAT done" << endl;
    writeIEND(file);
    cout << "IEND done" << endl;

    file.close();
    
}

void createPNG::writeSignature(ofstream &file)
{
    // PNG signature
    vector<uint8_t> signature = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
    };

    file.write(reinterpret_cast<char*>(signature.data()), signature.size()); // write PNG signature to file
}

void createPNG::writeIHDR(ofstream &file)
{
    write4bytes(file, 13); // IHDR data length to file. this func write this hexa value 00 00 00 0D ---> 0D 00 00 00
    file.write("IHDR", 4); // Write chunk type. here we write IHDR chunk so we specify the type of it.

    // create width array. which contain the width in bytes(8bits). we can use this array in ihdrData
    vector<uint8_t> widthArr(4);

    for(int i = 0; i < 4; i++){

        widthArr[i] = width >> (i * 8) & 0xFF;
    }

    // create height array. which contain the height in bytes(8bits). we can use this array in ihdrData
    vector<uint8_t> heightArr(4);

    for(int i = 0; i < 4; i++){

        heightArr[i] = height >> (i * 8) & 0xFF;
    }

    vector<uint8_t> ihdrType = {'I', 'H', 'D', 'R'};
    // IHDR data
    vector<uint8_t> ihdrData = {
        widthArr[3], widthArr[2], widthArr[1], widthArr[0], // Width:
        heightArr[3], heightArr[2], heightArr[1], widthArr[0], // Height:
        0x08,                   // Bit depth: 8
        0x02,                   // Color type: RGB
        0x00,                   // Compression: Deflate
        0x00,                   // Filter method: 0
        0x00                    // Interlace: None
    };

    file.write(reinterpret_cast<char*>(ihdrData.data()), ihdrData.size()); // write IHDR data

    vector<uint8_t> combineData;

    combineData.insert(combineData.end(), ihdrType.begin(), ihdrType.end());
    combineData.insert(combineData.end(), ihdrData.begin(), ihdrData.end());

    uint32_t crc = calculateCRC(combineData); // get the crc for ihdr

    write4bytes(file, crc); // write ihdr chunk crc to png file
}

void createPNG::writePLTE(ofstream &file)
{
    // RGB color in hex
    vector<uint8_t> colorPalette = {
        0xFF, 0x00, 0x00,           // (255, 0, 0) Red
        0x00, 0xFF, 0x00,           // (0, 255, 0) Green
        0x00, 0x00, 0xFF            // (0, 0, 255) Blue
    };

    write4bytes(file, colorPalette.size()); // write PLTE chunk data size
    file.write("PLTE", 4);  // write chunk type

    vector<uint8_t> plteType = {'P', 'L', 'T', 'E'};


    vector<uint8_t> combineData;

    combineData.insert(combineData.end(), plteType.begin(), plteType.end());
    combineData.insert(combineData.end(), colorPalette.begin(), colorPalette.end());

    file.write(reinterpret_cast<char*>(colorPalette.data()), colorPalette.size()); // write the plte data

    uint32_t crc = calculateCRC(combineData); // calculate crc for PLTE chunk

    write4bytes(file, crc);
}

void createPNG::writeIDAT(ofstream &file)
{
    // -----------------reading image data from outside file-------------------
    
    ifstream idatfile(idatFileName, ios::binary);


    // get file size
    idatfile.seekg(0, ios::end);
    size_t fileSize = idatfile.tellg();
    idatfile.seekg(0, ios::beg);

    vector<unsigned char> unCompressedData(fileSize); // make vector to store uncompressed data

    // cout << fileSize << endl;

    idatfile.read(reinterpret_cast<char*>(unCompressedData.data()), fileSize); // read uncompressed data
    idatfile.close();

    addFilterbytes(unCompressedData, width, height);

    // cout << unCompressedData.size() << endl;

    vector<unsigned char> compressedData = compressData(unCompressedData);

    vector<uint8_t> imgData(compressedData.begin(), compressedData.end());
    // cout << imgData.size() << endl;

    //------------------------IDAT chunk-------------------

    write4bytes(file, fileSize); // write the length of IDAT chunk data
    file.write("IDAT", 4);  // write the type of the chunk

    vector<uint8_t> idatType = {'I', 'D', 'A', 'T'};

    vector<uint8_t> combineData;

    combineData.insert(combineData.end(), idatType.begin(), idatType.end());
    combineData.insert(combineData.end(), imgData.begin(), imgData.end());

    file.write(reinterpret_cast<char*>(compressedData.data()), compressedData.size()); // write compressed image data

    uint32_t crc = calculateCRC(combineData); // calculate crc for IDAT chunk

    write4bytes(file, crc);
}

void createPNG::writeIEND(ofstream &file)
{
    write4bytes(file, 0); // write the length of the IEND chunk data. (IEND chunk doesn't contain data)
    file.write("IEND", 4);

    vector<uint8_t> iendType = {'I', 'E', 'N', 'D'};

    vector<uint8_t> combineData;

    combineData.insert(combineData.end(), iendType.begin(), iendType.end());

    uint32_t crc = calculateCRC(combineData); // calculate crc for IEND chunk

    write4bytes(file, crc); 
}
