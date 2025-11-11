#include "bitmap.h"

constexpr BMP::uint16 BMPMagic = 0x4d42;
std::map<BMP::Bitmap::Error, std::string> BMP::Bitmap::m_errorStrings = [] {
    std::map<BMP::Bitmap::Error, std::string> res;
    res.emplace(BMP::Bitmap::Error::NoneError, "");
    res.emplace(BMP::Bitmap::Error::WrongSize, "Not enoght size");
    res.emplace(BMP::Bitmap::Error::WrongType, "Data is not BMP format");
    res.emplace(BMP::Bitmap::Error::WrongPixelFormat, "Wrong pixel format or format not supported");
    res.emplace(BMP::Bitmap::Error::UnkonwnError, "Unknown error");
    return res;
}();

BMP::Bitmap::Bitmap(char * data, int size)
{
    this->data = data;
    if (size < sizeof(FileHeader) + sizeof(InfoHeader))
    {
        // Not enoght size
        m_err = WrongSize;
        return;
    }
    fileHeader = reinterpret_cast<FileHeader *>(this->data);
    if (fileHeader->bfType != BMPMagic)
    {
        // data is not BMP
        m_err = WrongType;
        return;
    }
    infoHeader = reinterpret_cast<InfoHeader *>(this->data + sizeof(FileHeader));
    if (infoHeader->biBitCount != 24)
    {
        // Another bitsPerPixel formats not supported
        m_err = WrongPixelFormat;
        return;
    }

    // Make lines pointers
    lineSize = infoHeader->biWidth * 3.;
    if (lineSize % 4 != 0)
    {
        lineSize = lineSize % 4 + 4;
    }
    if (size < fileHeader->bfOffBits + infoHeader->biHeight * lineSize)
    {
        // Not enoght size
        m_err = WrongSize;
        return;
    }
    lines = new uint8 *[infoHeader->biHeight];
    for (int i = 0; i < infoHeader->biHeight; i++)
    {
        lines[i] = reinterpret_cast<uint8 *>(&this->data[fileHeader->bfOffBits + i * lineSize]);
    }
}

BMP::Bitmap::~Bitmap()
{
    if (lines != nullptr)
    {
        delete[] lines;
        lines = nullptr;
    }

    if (data != nullptr)
    {
        delete[] data;
        data = nullptr;
    }
    fileHeader = nullptr;
    infoHeader = nullptr;
}

void BMP::Bitmap::getPx(int x, int y, uint8 & R, uint8 & G, uint8 & B)
{
    R = lines[y][x * 3];
    G = lines[y][x * 3 + 1];
    B = lines[y][x * 3 + 2];
}

void BMP::Bitmap::getPxBrt(int x, int y, uint8 & S)
{
    uint8 & R = lines[y][x * 3];
    uint8 & G = lines[y][x * 3 + 1];
    uint8 & B = lines[y][x * 3 + 2];
    S = 0.299 * R + 0.587 * G + 0.114 * B;
}

void BMP::Bitmap::makeBW()
{
    if (!isValid())
    {
        return;
    }
    for (int y = 0; y < infoHeader->biHeight; y++)
    {
        for (int x = 0; x < infoHeader->biWidth; x++)
        {
            uint8 & R = lines[y][x * 3];
            uint8 & G = lines[y][x * 3 + 1];
            uint8 & B = lines[y][x * 3 + 2];
            R = G = B = 0.299 * R + 0.587 * G + 0.114 * B;
        }
    }
}
