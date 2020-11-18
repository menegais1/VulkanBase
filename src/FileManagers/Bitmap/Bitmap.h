///Class responsible for loading and manipulating the .bmp
///It can load uncompressed .bmp of 1, 4, 8, 24 and 32 bits per pixel
///Meaning that the .bmp may contain a color pallete.
///The functions to manipulate the bitmap are:

///Scale
///Rotate - Nearest neighbour and default matrix rotation
///Generate Histogram per Channel
///Convert image to single channel
///Convert image to grayscale based on luminância
///Flip image in X plane
///Flip image in Y plane
///Reset image to default state - Image first load

#ifndef BITMAP_H
#define BITMAP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>


class FileHeader {
public:
    short BfType;
    int BfSize;
    short BfReser1;
    short BfReser2;
    int BfOffSetBits;

    void print() {
        std::cout << "BfType:" << BfType << std::endl;
        std::cout << "BfSize:" << BfSize << std::endl;
        std::cout << "BfReser1:" << BfReser1 << std::endl;
        std::cout << "BfReser1:" << BfReser1 << std::endl;
        std::cout << "BfOffSetBits:" << BfOffSetBits << std::endl;
    }
};

class BitmapHeader {
public:
    int BiSize;
    int BiWidth;
    int BiHeight;
    short BiPlanes;
    short BiBitCount;
    int BiCompress;
    int BiSizeImag;
    int BiXPPMeter;
    int BiYPPMeter;
    int BiClrUsed;
    int BiClrImpor;

    void print() {
        std::cout << "BiSize:" << BiSize << std::endl;
        std::cout << "BiWidth:" << BiWidth << std::endl;
        std::cout << "BiHeight:" << BiHeight << std::endl;
        std::cout << "BiPlanes:" << BiPlanes << std::endl;
        std::cout << "BiBitCount:" << BiBitCount << std::endl;
        std::cout << "BiCompress:" << BiCompress << std::endl;
        std::cout << "BiSizeImag:" << BiSizeImag << std::endl;
        std::cout << "BiXPPMeter:" << BiXPPMeter << std::endl;
        std::cout << "BiYPPMeter:" << BiYPPMeter << std::endl;
        std::cout << "BiClrUsed:" << BiClrUsed << std::endl;
        std::cout << "BiClrImpor:" << BiClrImpor << std::endl;
    }
};

enum Filter {
    FlipX,
    FlipY,
    Greyscale,
    RedC,
    GreenC,
    BlueC
};

class Bitmap {
public:
    std::string fileName;
    int width;
    int height;

    Bitmap(const std::string fileName);

    glm::vec4 sampleBitmao(const float u, const float v) const;

    glm::vec4 getPixelColorAtPosition(const int l, const int c) const;

    glm::vec2 getPixelPositionOnScreen(const int l, const int c) const;

    void flipImageInX();

    void flipImageInY();

    void convertImageToGrayScale();

//    void convertToSingleChannel(const Channel c);

    void rotateImage(const float angle);

    void nearestNeighbourRotation(const float angle);

    void scaleImage(const float scale);

    void applyTransformations(bool applyScale, bool applyRotation, bool applyFilters);

    void resetImageToDefault();

//    int *getHistogramForChannel(const Channel c) const;


    glm::vec4 *originalBitmapArray;
private:
    FileHeader fileHeader;
    BitmapHeader bitmapHeader;
    glm::vec4 *colorPallete;
    glm::vec4 *bitmapArray;
    bool colorPalleteExists;
    float imageRotation;

    std::vector<Filter> filters;
    float lastRotation;
    float lastScale;

    void _nearestNeighbourRotation(const float angle);

    void _scaleImage(const float scale);

    void _flipImageInX();

    void _flipImageInY();

    void _convertImageToGrayScale();

//    void _convertToSingleChannel(const Channel c);

    void resetImage();

    void openFile(const std::string filename, std::fstream &file);

    void closeFile(std::fstream &file);

    void loadFileHeader(std::fstream &file);

    void loadBitmapHeader(std::fstream &file);

    bool checkColorPallete(std::fstream &file);

    void loadColorPallete(std::fstream &file);

    void loadImage(std::fstream &file);

    glm::vec4 getPixelFromPallete(const unsigned char pixelValue);

    Bitmap(const int width, const int height);
};

#endif