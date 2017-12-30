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
        int inPageWidth, int inPageHeight, std::vector<std::string> inPages) {
    imageWidth = inImageWidth;
    imageHeight = inImageHeight;
    pageWidth = inPageWidth;
    pageHeight = inPageHeight;
    pages = inPages;
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

Php::Value PdfImageResult::getNumberOfPages() {
    return (int) pages.size();
}

Php::Value PdfImageResult::getPages() {
    return pages;
}

