/*
 * pdf-poppler.h
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#ifndef PDF_IMAGE_FORMAT_H_
#define PDF_IMAGE_FORMAT_H_

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-rectangle.h>
#include <poppler-global.h>
#include <poppler-page-renderer.h>

class PdfImageFormat {
private:
    char format[5];
    char ext[4];
public:
    PdfImageFormat(const char * inFormat, const char * inExt);
    const char * getFormat();
    const char * getExtension();
};

#endif /* PDF_IMAGE_FORMAT_H_ */
