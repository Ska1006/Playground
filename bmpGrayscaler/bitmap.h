#ifndef BITMAP_H
#define BITMAP_H

namespace BMP
{
    using uint8 = unsigned char;
    using uint16 = unsigned short;
    using uint32 = unsigned int;
    using int8 = char;
    using int16 = short;
    using int32 = int;
#pragma pack(push, 2)
    struct FileHeader
    {
        uint16 bfType;
        uint32 bfSize;
        uint16 bfReserved1;
        uint16 bfReserved2;
        uint32 bfOffBits;
    };
    struct InfoHeader
    {
        uint32 biSize;
        int32 biWidth;
        int32 biHeight;
        uint16 biPlanes;
        uint16 biBitCount;
        uint32 biCompression;
        uint32 biSizeImage;
        int32 biXPelsPerMeter;
        int32 biYPelsPerMeter;
        uint32 biClrUsed;
        uint32 biClrImportant;
    };
#pragma pack(pop)

    class Bitmap
    {
    public:
        Bitmap(char * data, int size);
        ~Bitmap();
        FileHeader * fileHeader{};
        InfoHeader * infoHeader{};
        uint8 ** lines;
        char * data;
        inline bool isValid() { return fileHeader != nullptr && infoHeader != nullptr; }
        void getPx(int x, int y, uint8 & R, uint8 & G, uint8 & B);
        void getPxBrt(int x, int y, uint8 & S);
        void makeBW();
    };
} // namespace BMP
#endif // BITMAP_H
