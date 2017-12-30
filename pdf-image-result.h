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

#endif /* PDF_POPPLER_H_ */
