/*
 * pdf-poppler.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include "pdf-image-result.h"

PdfImageResult::PdfImageResult(int inImageWidth, int inImageHeight,
        int inPageWidth, int inPageHeight, std::string inPage) {
    imageWidth = inImageWidth;
    imageHeight = inImageHeight;
    pageWidth = inPageWidth;
    pageHeight = inPageHeight;
    page = inPage;
}

void PdfImageResult::__construct() {
}

Php::Value PdfImageResult::getImageWidth() {
    return imageWidth;
}

Php::Value PdfImageResult::getImageHeight() {
    return imageHeight;
}

Php::Value PdfImageResult::getPageWidth() {
    return pageWidth;
}

Php::Value PdfImageResult::getPageHeight() {
    return pageHeight;
}

Php::Value PdfImageResult::getPage() {
    return page;
}

Php::Value PdfImageResult::getRelativePage(Php::Parameters &params) {
    std::string rootPath = params[0].stringValue();
    size_t pagePos = page.find(rootPath);
    if (pagePos == std::string::npos) {
        throw Php::Exception(rootPath + " not found within page: " + page);
    }

    return page.substr(+rootPath.length());
}
