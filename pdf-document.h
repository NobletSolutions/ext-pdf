/*
 * pdf-poppler.h
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#ifndef PDF_DOCUMENT_H_
#define PDF_DOCUMENT_H_

#include <phpcpp.h>
#include <iostream>
#include <cstring>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-rectangle.h>
#include <poppler-global.h>
#include <poppler-page-renderer.h>
#include "pdf-image-format.h"
#include "pdf-image-result.h"

class PdfDocument: public Php::Base {
protected:
    int _major = 0;
    int _minor = 0;
    poppler::document *_document = NULL;
    PdfImageFormat * jpeg = NULL;
    PdfImageFormat * png = NULL;
    PdfImageFormat * tiff = NULL;
public:
    PdfDocument();
    void __construct(Php::Parameters &params);
    Php::Value getMajorVersion();
    Php::Value getMinorVersion();
    Php::Value getCreator();
    Php::Value getCreationDate();
    Php::Value getModifiedDate();
    Php::Value hasEmbeddedFiles();
    Php::Value isEncrypted();
    Php::Value isLinear();
    Php::Value isLocked();
    Php::Value numberOfPages();
    Php::Value asString();
    Php::Value toImage(Php::Parameters &params);
    Php::Value compare(Php::Parameters &params);
    Php::Value hash(Php::Parameters &params);
private:
    PdfImageFormat * getImageFormat(int inFormat);
};

#endif /* PDF_DOCUMENT_H_ */
