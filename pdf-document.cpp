/*
 * pdf-poppler.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
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

static int _mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    int status;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for(p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            status = mkdir(tmp, S_IRWXU);
            if(status != 0) {
                return -1;
            }
            *p = '/';
        }
    }

    mkdir(tmp, S_IRWXU);
    return 0;
}

Php::Value PdfDocument::toImage(Php::Parameters &params) {
    int firstPage = 0;
    int lastPage = _document->pages();
    int resolution = 75;
    int x;
    int imageWidth = 0;
    int imageHeight = 0;
    int pageWidth = 0;
    int pageHeight = 0;
    struct stat buffer;

    std::vector<std::string> pages;

    char pattern[300];
    PdfImageFormat * format = NULL;
    poppler::page *page;
    poppler::page_renderer *renderer;
    poppler::image image;

    format = this->getImageFormat(params[0]);
    if (format == NULL) {
        Php::error << "Unable to determine type" << std::endl;
        return false;
    }

    if (params[1].size() + 10 > 255) {
        Php::error << "Path is larger than 255 chars - Unable to proceed" << std::endl;
        return false;
    }

    if (params.size() == 3) {
        resolution = params[2];
    }

    renderer = new poppler::page_renderer();

    params[1].stringValue().copy(pattern,params[1].size(),0);

    char * _dirname = dirname(pattern);
    // Check that the directory exists
    if (stat (_dirname, &buffer) == -1) {
        if(_mkdir(_dirname) != 0) {
            throw Php::Exception("Directory doesn't exist - Unable to create");
        }
    }

    for (x = firstPage; x < lastPage; x++) {
        char outFile[255];// = new char[255];
        memset(&outFile, 0, 255);
        sprintf(&outFile[0], "%s-%d.%s", params[1].stringValue().c_str(), x, format->getExtension());

        page = _document->create_page(x);

        poppler::rectf pageDim = page->page_rect();
        pageWidth = pageDim.width();
        pageHeight = pageDim.height();

        image = renderer->render_page(page, resolution, resolution, 0, 0, -1, -1, poppler::rotation_enum::rotate_0);
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

