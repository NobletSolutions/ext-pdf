/*
 * pdf-poppler.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include "pdf-document.h"
#include "pdf-image-result.h"
#include "pdf-image-format.h"

PdfDocument::PdfDocument() {
    jpeg = new PdfImageFormat("jpeg", "jpg");
    png = new PdfImageFormat("png", "png");
    tiff = new PdfImageFormat("tiff", "tif");
}

void PdfDocument::__construct(Php::Parameters &params) {
    _document = poppler::document::load_from_file(params[0]); //,params[1],params[2]);
}

Php::Value PdfDocument::getMajorVersion() {
    if (_major == 0) {
        _document->get_pdf_version(&_major, &_minor);
    }

    return _major;
}

Php::Value PdfDocument::getMinorVersion() {
    if (_major == 0) {
        _document->get_pdf_version(&_major, &_minor);
    }

    return _minor;
}

Php::Value PdfDocument::hasEmbeddedFiles() {
    return _document->has_embedded_files();
}

Php::Value PdfDocument::isEncrypted() {
    return _document->is_encrypted();
}

Php::Value PdfDocument::isLinear() {
    return _document->is_linearized();
}

Php::Value PdfDocument::isLocked() {
    return _document->is_locked();
}

Php::Value PdfDocument::numberOfPages() {
    return _document->pages();
}

Php::Value PdfDocument::asString() {
    int firstPage;
    int lastPage;
    int x;
    poppler::page *page;
    poppler::ustring pageData;
    std::string resultData;
    poppler::byte_array arr;
    char *c_str;

    firstPage = 0;
    lastPage = _document->pages();

    for (x = firstPage; x < lastPage; x++) {
        page = _document->create_page(x);
        pageData = page->text(page->page_rect(poppler::media_box));
        arr = pageData.to_utf8();
        c_str = &arr[0];
        resultData.append(std::string(c_str, arr.size()));
    }

    return resultData;
}

Php::Value PdfDocument::toImage(Php::Parameters &params) {
    int firstPage;
    int lastPage = _document->pages();
    int resolution = 75;
    int x;
    int imageWidth = 0;
    int imageHeight = 0;
    int pageWidth = 0;
    int pageHeight = 0;
    std::vector<std::string> pages;

    char * pattern = NULL;
    PdfImageFormat * format = NULL;
    poppler::page *page;
    poppler::page_renderer *renderer;
    poppler::image image;

    format = this->getImageFormat(params[0]);
    if (format == NULL) {
        Php::error << "Unable to determine type" << std::endl;
        return false;
    }
    pattern = params[1].buffer();

    if (strlen(pattern) + 10 > 255) {
        Php::error << "Path is larger than 255 chars - Unable to proceed"
                << std::endl;
        return false;
    }

    if (params.size() == 3) {
        resolution = params[2];
    }

    renderer = new poppler::page_renderer();
    firstPage = 0;

    for (x = firstPage; x < lastPage; x++) {
        char * outFile = new char[255];
        memset(outFile, 0, 255);
        sprintf(outFile, "%s-%d.%s", pattern, x, format->getExtension());

        page = _document->create_page(x);

        poppler::rectf pageDim = page->page_rect();
        pageWidth = pageDim.width();
        pageHeight = pageDim.height();

        image = renderer->render_page(page, resolution, resolution, 0, 0, -1,
                -1, poppler::rotation_enum::rotate_0);
        image.save(outFile, format->getFormat(), resolution);

        imageWidth = image.width();
        imageHeight = image.height();

        pages.push_back(outFile);
    }

    PdfImageResult * obj = new PdfImageResult(pageWidth, pageHeight, imageWidth,
            imageHeight, pages);
    return Php::Object("\\PDF\\PdfImageResult", obj);
}

PdfImageFormat * PdfDocument::getImageFormat(int inFormat) {
    switch (inFormat) {
    case 2:
        return png;
        break;
    case 3:
        return tiff;
    default:
    case 1:
        return jpeg;
    }

    return NULL;
}

