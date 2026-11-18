/*
 * pdf-poppler.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include "pdf-image-format.h"

PdfImageFormat::PdfImageFormat(const char * inFormat, const char * inExt) {
    memset(&format, 0, 5);
    memset(&ext, 0, 4);
    strncpy(&format[0], inFormat, 4);
    strncpy(&ext[0], inExt, 3);
}

const char * PdfImageFormat::getFormat() {
    return &format[0];
}

const char * PdfImageFormat::getExtension() {
    return &ext[0];
}


