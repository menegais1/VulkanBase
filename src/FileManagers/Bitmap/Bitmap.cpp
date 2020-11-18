#include "Bitmap.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstring>

using namespace std;

Bitmap::Bitmap(const string fileName) {
    this->fileName = fileName;
    this->imageRotation = 0;
    fstream file;
    openFile(fileName, file);
    if (!file.is_open())
        return;
    loadFileHeader(file);
    loadBitmapHeader(file);
    colorPalleteExists = checkColorPallete(file);
    if (colorPalleteExists) {
        loadColorPallete(file);
    }
    loadImage(file);
    closeFile(file);
    width = bitmapHeader.BiWidth;
    height = bitmapHeader.BiHeight;
    lastScale = 1;
    lastRotation = 0;
    fileHeader.print();
    bitmapHeader.print();
}

void Bitmap::openFile(const string filename, fstream &file) {
    file.open(filename, fstream::in | fstream::binary);
}

void Bitmap::closeFile(fstream &file) {
    file.close();
}

void Bitmap::loadFileHeader(fstream &file) {
    file.read(reinterpret_cast<char *>(&fileHeader.BfType), 2);
    file.read(reinterpret_cast<char *>(&fileHeader.BfSize), 4);
    file.read(reinterpret_cast<char *>(&fileHeader.BfReser1), 2);
    file.read(reinterpret_cast<char *>(&fileHeader.BfReser2), 2);
    file.read(reinterpret_cast<char *>(&fileHeader.BfOffSetBits), 4);
}

void Bitmap::loadBitmapHeader(fstream &file) {
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiSize), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiWidth), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiHeight), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiPlanes), 2);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiBitCount), 2);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiCompress), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiSizeImag), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiXPPMeter), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiYPPMeter), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiClrUsed), 4);
    file.read(reinterpret_cast<char *>(&bitmapHeader.BiClrImpor), 4);
}

bool Bitmap::checkColorPallete(fstream &file) {
    if (bitmapHeader.BiBitCount < 24)
        return true;
    return false;
}

void Bitmap::loadColorPallete(fstream &file) {
    int palleteCount = pow(2, bitmapHeader.BiBitCount);
    colorPallete = new glm::vec4[palleteCount];
    unsigned char pallete[4];
    for (int i = 0; i < palleteCount; i++) {
        file.read(reinterpret_cast<char *>(&pallete[2]), 1);
        file.read(reinterpret_cast<char *>(&pallete[1]), 1);
        file.read(reinterpret_cast<char *>(&pallete[0]), 1);
        file.read(reinterpret_cast<char *>(&pallete[3]), 1);
        colorPallete[i] = glm::vec4(pallete[0], pallete[1], pallete[2], pallete[3]);
    }
}

glm::vec4 Bitmap::getPixelFromPallete(const unsigned char pixelValue) {
    glm::vec4 p;
    p[0] = colorPallete[pixelValue][0];
    p[1] = colorPallete[pixelValue][1];
    p[2] = colorPallete[pixelValue][2];
    p[3] = colorPallete[pixelValue][3];
    return p;
}

void Bitmap::loadImage(fstream &file) {
    file.clear();
    file.seekg(fileHeader.BfOffSetBits, ios::beg);
    int rowSize = std::ceil((bitmapHeader.BiBitCount * bitmapHeader.BiWidth) / 32) * 4;
    int padding = rowSize - (bitmapHeader.BiBitCount / 8 * bitmapHeader.BiWidth);
    int bitmapSize = bitmapHeader.BiWidth * bitmapHeader.BiHeight;
    unsigned char *byteArray = new unsigned char[rowSize];

    bitmapArray = new glm::vec4[bitmapSize + 1];

    cout << "Bitmap size:" << bitmapSize << endl;
    cout << "Row Size:" << rowSize << endl;
    cout << "Padding:" << padding << endl;

    if (colorPalleteExists) {
        for (int l = 0; l < bitmapHeader.BiHeight; l++) {
            file.read(reinterpret_cast<char *>(byteArray), rowSize);

            for (int i = 0; i <= rowSize - padding; i++) {

                if (bitmapHeader.BiBitCount == 1) {
                    for (int j = 0; j < 8; j++) {

                        unsigned char pixelValue = (byteArray[i] << j) >> 7;

                        bitmapArray[l * bitmapHeader.BiWidth + i + j] = getPixelFromPallete(pixelValue) / 255.f;
                    }
                }
                if (bitmapHeader.BiBitCount == 4) {
                    for (int j = 0; j < 2; j++) {

                        unsigned char pixelValue = (byteArray[i] << j) >> 4;
                        bitmapArray[l * bitmapHeader.BiWidth + i + j] = getPixelFromPallete(pixelValue) / 255.f;
                    }
                }
                if (bitmapHeader.BiBitCount == 8) {

                    unsigned char pixelValue = byteArray[i];
                    bitmapArray[l * bitmapHeader.BiWidth + i] = getPixelFromPallete(pixelValue) / 255.f;
                }
            }
        }
    } else {

        for (int l = 0; l < bitmapHeader.BiHeight; l++) {
            file.read(reinterpret_cast<char *>(byteArray), rowSize);
            for (int i = 0, j = 0; i <= rowSize - padding - 3; i += (int) (bitmapHeader.BiBitCount / 8), j++) {
                glm::vec4 p;
                if (bitmapHeader.BiBitCount == 24) {
                    p[0] = byteArray[i + 2];
                    p[1] = byteArray[i + 1];
                    p[2] = byteArray[i + 0];
                    p[3] = 255;
                } else {
                    p[0] = byteArray[i + 2];
                    p[1] = byteArray[i + 1];
                    p[2] = byteArray[i + 0];
                    p[3] = byteArray[i + 3];
                }

                bitmapArray[l * bitmapHeader.BiWidth + j] = p / 255.f;
            }
        }
    }

    originalBitmapArray = new glm::vec4[bitmapSize + 1];
    std::memcpy(originalBitmapArray, bitmapArray, bitmapSize * sizeof(glm::vec4));
}

glm::vec4 Bitmap::getPixelColorAtPosition(const int l, const int c) const {

    int idx = l * width + c;
    return bitmapArray[idx];
}

glm::vec2 Bitmap::getPixelPositionOnScreen(const int l, const int c) const {
    float translatedC = c - width / 2;
    float translatedL = l - height / 2;
    float newC = (translatedC * cos(imageRotation) + translatedL * sin(imageRotation));
    float newL = ((-translatedC * sin(imageRotation)) + translatedL * cos(imageRotation));
    glm::vec2 pos = {newC + width / 2, newL + height / 2};
    return pos;
}

void Bitmap::scaleImage(const float scale) {
    lastScale = scale;
}

void Bitmap::_scaleImage(const float scale) {
    int oldHeight = this->bitmapHeader.BiHeight;
    int oldWidth = this->bitmapHeader.BiWidth;
    this->height = oldHeight * scale;
    this->width = oldWidth * scale;
    glm::vec4 *newBitmapArray = new glm::vec4[height * width];
    for (int l = 0; l < height; l++) {
        for (int c = 0; c < width; c++) {
            int oldL = floor(l * (1 / scale));
            int oldC = floor(c * (1 / scale));
            int oldIdx = oldL * oldWidth + oldC;
            glm::vec4 color = originalBitmapArray[oldIdx];

            int idx = l * width + c;
            newBitmapArray[idx] = color;
        }
    }
    if (bitmapArray != NULL) {
        delete[] bitmapArray;
        bitmapArray = NULL;
    }

    bitmapArray = newBitmapArray;
    lastScale = scale;
}

void Bitmap::flipImageInX() {
    filters.push_back(Filter::FlipX);
}

void Bitmap::_flipImageInX() {
    for (int l = 0; l < height; l++) {
        for (int c = 0; c < width / 2; c++) {
            int newIdx = l * width + (width - 1 - c);
            int idx = l * width + c;
            glm::vec4 tmp = bitmapArray[idx];
            bitmapArray[idx] = bitmapArray[newIdx];
            bitmapArray[newIdx] = tmp;
        }
    }
}

void Bitmap::flipImageInY() {
    filters.push_back(Filter::FlipY);
}

void Bitmap::_flipImageInY() {
    for (int l = 0; l < height / 2; l++) {
        for (int c = 0; c < width; c++) {
            int newIdx = (height - 1 - l) * width + c;
            int idx = l * width + c;
            glm::vec4 tmp = bitmapArray[idx];
            bitmapArray[idx] = bitmapArray[newIdx];
            bitmapArray[newIdx] = tmp;
        }
    }
}

void Bitmap::rotateImage(const float angle) {
    this->imageRotation = angle;
}

void Bitmap::nearestNeighbourRotation(const float angle) {
    lastRotation = angle;
}

void Bitmap::_nearestNeighbourRotation(const float angle) {

    int diagonal = ceil(sqrt((width * width) + (height * height)));
    glm::vec4 *newBitmapArray = new glm::vec4[diagonal * diagonal];
    for (int l = 0; l < diagonal; l++) {
        for (int c = 0; c < diagonal; c++) {
            float translatedC = c - diagonal / 2;
            float translatedL = l - diagonal / 2;
            float newC = (translatedC * cos(-angle) + translatedL * sin(-angle));
            float newL = ((-translatedC * sin(-angle)) + translatedL * cos(-angle));
            glm::vec2 pos = {newC + width / 2, newL + height / 2};

            int idx = l * diagonal + c;
            glm::vec4 color;
            if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) {
                color[0] = 0;
                color[1] = 0;
                color[2] = 0;
                color[3] = 255;
                newBitmapArray[idx] = color;
                continue;
            }
            color = getPixelColorAtPosition(pos.y, pos.x);
            newBitmapArray[idx] = color;
        }
    }
    delete[] bitmapArray;
    width = diagonal;
    height = diagonal;
    bitmapArray = newBitmapArray;
    lastRotation = angle;
}

void Bitmap::convertImageToGrayScale() {
    filters.push_back(Filter::Greyscale);
}

void Bitmap::_convertImageToGrayScale() {
    for (int l = 0; l < height; l++) {
        for (int c = 0; c < width; c++) {
            int idx = l * width + c;

            float grayScaleColor =
                    bitmapArray[idx][0] * 0.299 + bitmapArray[idx][1] * 0.587 + bitmapArray[idx][2] * 0.114;
            glm::vec4 color;
            color[0] = grayScaleColor;
            color[1] = grayScaleColor;
            color[2] = grayScaleColor;
            bitmapArray[idx] = color;
        }
    }
}

//int *Bitmap::getHistogramForChannel(const Channel c) const {
//    int *histogram = new int[256];
//    for (int i = 0; i < 256; i++) {
//        histogram[i] = 0;
//    }
//    for (int i = 0; i < width * height; i++) {
//        histogram[(int) bitmapArray[i][c]] += 1;
//    }
//    return histogram;
//}
//
//void Bitmap::convertToSingleChannel(const Channel c) {
//    if (c == Channel::Red)
//        filters.push_back(Filter::RedC);
//    if (c == Channel::Green)
//        filters.push_back(Filter::GreenC);
//    if (c == Channel::Blue)
//        filters.push_back(Filter::BlueC);
//}
//
//void Bitmap::_convertToSingleChannel(const Channel c) {
//    for (int i = 0; i < width * height; i++) {
//        for (int curC = 0; curC < 3; curC++) {
//            if (c == curC)
//                continue;
//            bitmapArray[i][curC] = 0;
//        }
//    }
//}

//void Bitmap::applyTransformations(bool applyScale, bool applyRotation, bool applyFilters) {
//    resetImage();
//    if (applyScale)
//        _scaleImage(lastScale);
//    if (applyRotation) {
//        _nearestNeighbourRotation(lastRotation);
//    }
//    if (applyFilters) {
//        for (int i = 0; i < filters.size(); i++) {
//            Filter f = filters[i];
//
//            if (f == Filter::FlipX)
//                _flipImageInX();
//            if (f == Filter::FlipY)
//                _flipImageInY();
//            if (f == Filter::Greyscale)
//                _convertImageToGrayScale();
//            if (f == Filter::RedC)
//                _convertToSingleChannel(Channel::Red);
//            if (f == Filter::GreenC)
//                _convertToSingleChannel(Channel::Green);
//            if (f == Filter::BlueC)
//                _convertToSingleChannel(Channel::Blue);
//        }
//    }
//}

void Bitmap::resetImage() {
    if (bitmapArray != NULL) {
        delete[] bitmapArray;
        bitmapArray = NULL;
    }

    bitmapArray = new glm::vec4[bitmapHeader.BiHeight * bitmapHeader.BiWidth];
    memcpy(bitmapArray, originalBitmapArray, sizeof(glm::vec4) * bitmapHeader.BiHeight * bitmapHeader.BiWidth);
    this->height = bitmapHeader.BiHeight;
    this->width = bitmapHeader.BiWidth;
}

void Bitmap::resetImageToDefault() {
    if (bitmapArray != NULL) {
        delete[] bitmapArray;
        bitmapArray = NULL;
    }

    bitmapArray = new glm::vec4[bitmapHeader.BiHeight * bitmapHeader.BiWidth];
    memcpy(bitmapArray, originalBitmapArray, sizeof(glm::vec4) * bitmapHeader.BiHeight * bitmapHeader.BiWidth);
    this->height = bitmapHeader.BiHeight;
    this->width = bitmapHeader.BiWidth;
    this->imageRotation = 0;
    this->lastRotation = 0;
    this->lastScale = 1;
    this->filters.clear();
}

glm::vec4 Bitmap::sampleBitmao(const float u, const float v) const {
    return getPixelColorAtPosition(v * (height - 1), u * (width - 1));
}

