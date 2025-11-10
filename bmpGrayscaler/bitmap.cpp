#include "bitmap.h"


BMP::Bitmap::Bitmap(char *data, int size)
{
    fileHeader = reinterpret_cast<FileHeader*>(data);
    infoHeader = reinterpret_cast<InfoHeader*>(data + sizeof(FileHeader));
    lines = new uint8*[infoHeader->biHeight];
    int lineSize = infoHeader->biWidth * 3.;
    if(lineSize % 4)
        lineSize = lineSize % 4 + 4;
    for(int i = 0; i < infoHeader->biHeight; i++){
        lines[i] = reinterpret_cast<uint8*>(&data[fileHeader->bfOffBits + i * lineSize]);
    }
}

void BMP::Bitmap::getPx(int x, int y, uint8 &R, uint8 &G, uint8 &B)
{
    R = lines[y][x * 3];
    G = lines[y][x * 3 + 1];
    B = lines[y][x * 3 + 2];
}

void BMP::Bitmap::getPxBrt(int x, int y, uint8 &S)
{
    uint8 &R = lines[y][x * 3];
    uint8 &G = lines[y][x * 3 + 1];
    uint8 &B = lines[y][x * 3 + 2];
    S = 0.299 * R + 0.587 * G + 0.114 * B;
}

void BMP::Bitmap::makeBW()
{
    for(int y = 0; y < infoHeader->biHeight; y++){
        for(int x = 0; x < infoHeader->biWidth; x++){
            uint8 &R = lines[y][x * 3];
            uint8 &G = lines[y][x * 3 + 1];
            uint8 &B = lines[y][x * 3 + 2];
            R = G = B = 0.299 * R + 0.587 * G + 0.114 * B;
        }
    }

}
