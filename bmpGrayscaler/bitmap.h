#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <map>
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
    enum Error:uint8{
        NoneError,
        WrongSize,
        WrongType,
        WrongPixelFormat,
        UnkonwnError
    }m_err{NoneError};
    Bitmap(char *data, int size);
    ~Bitmap();
    FileHeader *fileHeader{};
    InfoHeader *infoHeader{};
    uint8 **lines{};
    char *data{};
    uint32 lineSize;
    inline bool isValid()
    {
        return data != nullptr && fileHeader != nullptr && infoHeader != nullptr
               && lines != nullptr;
    }
    void getPx(int x, int y, uint8 &R, uint8 &G, uint8 &B);
    void getPxBrt(int x, int y, uint8 &S);
    void makeBW();
    inline static std::string errorString(Error const type)
    {
        auto it = m_errorStrings.find(type);
        return it != m_errorStrings.end() ? it->second : std::string{};
    }
    std::string errorString() const
    {
        return errorString(m_err);
    };
private:
    static std::map<Error, std::string> m_errorStrings;
};
} // namespace BMP
#endif // BITMAP_H
