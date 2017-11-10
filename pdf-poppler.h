/*
 * pdf-poppler.h
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#ifndef PDF_POPPLER_H_
#define PDF_POPPLER_H_

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-rectangle.h>
#include <poppler-global.h>
#include <poppler-page-renderer.h>

class PdfImageResult: public Php::Base {
private:
    int imageWidth;
    int imageHeight;
    int pageWidth;
    int pageHeight;
    std::vector<std::string> pages;
public:
    PdfImageResult(int inImageWidth, int inImageHeight, int inPageWidth,
            int inPageHeight, std::vector<std::string> inPages);
    void __construct();
    Php::Value getImageWidth();
    Php::Value getImageHeight();
    Php::Value getPageWidth();
    Php::Value getPageHeight();
    Php::Value getNumberOfPages();
    Php::Value getPages();
};

class PdfImageFormat {
private:
    char format[5];
    char ext[4];
public:
    PdfImageFormat(const char * inFormat, const char * inExt);
    const char * getFormat();
    const char * getExtension();
};

class PdfDocument: public Php::Base {
private:
    int _major;
    int _minor;
    poppler::document *_document;
    PdfImageFormat * jpeg;
    PdfImageFormat * png;
    PdfImageFormat * tiff;
public:
    PdfDocument();
    void __construct(Php::Parameters &params);
    Php::Value getMajorVersion();
    Php::Value getMinorVersion();
    Php::Value hasEmbeddedFiles();
    Php::Value isEncrypted();
    Php::Value isLinear();
    Php::Value isLocked();
    Php::Value numberOfPages();
    Php::Value asString();
    Php::Value toImage(Php::Parameters &params);

private:
    PdfImageFormat * getImageFormat(int inFormat);
};

#endif /* PDF_POPPLER_H_ */
