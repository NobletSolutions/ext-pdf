/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <string.h>
#include "pdf-image.h"

PdfImage::PdfImage() = default;

PdfImage::PdfImage(PdfImage &obj) {
    index = 0;
    width = 0;
    height = 0;

    x = obj.x;
    y = obj.y;
    imagepath = obj.imagepath;

    if (obj.index > 0) {
        index = obj.index;
    }

    if (obj.width > 0) {
        width = obj.width;
        height = obj.height;
    }
}

void PdfImage::__construct(Php::Parameters &params) {
    x = params[0].numericValue();
    y = params[1].numericValue();
    imagepath = params[2].stringValue();

    if (params.size() > 3) {
        index = params[3].numericValue();
    }

    if (params.size() > 4) {

        if (params.size() != 6) {
            throw Php::Exception("When providing width/height parameters, both are required.");
        }

        width = params[4].numericValue();
        height = params[5].numericValue();
    }
}


Php::Value PdfImage::getX() {
    return x;
}

Php::Value PdfImage::getY() {
    return y;
}

Php::Value PdfImage::getIndex() {
    return index;
}

Php::Value PdfImage::getImagePath() {
    return imagepath;
}

Php::Value PdfImage::getWidth() {
    return width;
}

Php::Value PdfImage::getHeight() {
    return height;
}

