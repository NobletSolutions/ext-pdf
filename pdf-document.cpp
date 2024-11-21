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
#include "pdf-hasher.h"
#include <limits.h>
#include <poppler-page.h>
#include <openssl/sha.h>


inline bool file_exists(const std::string &filename ) {
    struct stat statBuffer;
    return (stat(filename.c_str(), &statBuffer) == 0);
}

static int _mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;

            if (!file_exists(tmp) && mkdir(tmp, S_IRWXU) != 0) {
                //Php::warning << "PHP-PDF: Unable to create dir " << tmp << " because '" << strerror(errno) << "'" << std::flush;
                return -1;
            }

            *p = '/';
        }
    }

    return mkdir(tmp, S_IRWXU);
}

PdfDocument::PdfDocument() {
    jpeg = new PdfImageFormat("jpeg", "jpg");
    png = new PdfImageFormat("png", "png");
    tiff = new PdfImageFormat("tiff", "tif");
}

void PdfDocument::__construct(Php::Parameters &params) {
    if (!file_exists(params[0])) {
        Php::warning << "File " << params[0] << " doesn't exist" << std::flush;
        throw Php::Exception("File doesn't exist?");
    }

    char *real_path = realpath(params[0].stringValue().c_str(), NULL);
    filePath = std::string(real_path);
    free(real_path);

    // Php::out << "ParamPath: " << params[0].stringValue() << "RealPath: " << filePath << std::endl;

    _document = poppler::document::load_from_file(filePath);

    if(_document == NULL) {
	    Php::warning << "Unable to open file as PDF" << std::flush;
	    throw Php::Exception("File is not a PDF");
    }
}

Php::Value PdfDocument::getCreator(){
    poppler::byte_array arr;
    char *c_str;

    arr = _document->info_key("Creator").to_utf8();
    c_str = &arr[0];
    return c_str;
}

Php::Value PdfDocument::getCreationDate(){
    char buffer[50];
#if POPPLER_HAS_NEW_DATE_T == 1
    time_t t = _document->info_date_t("CreationDate");
    if(t == -1) {
        return nullptr;
    }

    snprintf(buffer, 50, "@%lu", t);
#else 
    poppler::time_type t = _document->info_date("CreationDate");
    if(t == UINT_MAX) {
        return nullptr;
    }

    snprintf(buffer,50,"@%u",t);
#endif
    return Php::Object("DateTime", buffer);
}

Php::Value PdfDocument::getModifiedDate() {
    char buffer[50];
#if POPPLER_HAS_NEW_DATE_T == 1
    time_t t = _document->info_date_t("ModDate");
    if(t == -1) {
        return nullptr;
    }

    snprintf(buffer, 50, "@%lu", t);
#else
    poppler::time_type t = _document->info_date("ModDate");
    if(t == UINT_MAX) {
        return nullptr;
    }

    snprintf(buffer,50,"@%u",t);
#endif
    return Php::Object("DateTime", buffer);
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

#if POPPLER_HAS_JS == 1
Php::Value PdfDocument::hasJS() {
    return _document->has_js();
}

Php::Value PdfDocument::hasXFA() {
    return _document->has_xfa();
}
#endif

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
    int firstPage = 0;
    int lastPage = _document->pages();
    int resolution = 75;
    int x;
    Php::Value returnValue;

    PdfImageFormat * format = NULL;
    poppler::page *page;
    poppler::page_renderer *renderer;
    poppler::image image;

    format = this->getImageFormat(params[0]);
    if (format == NULL) {
        //Php::warning << "Unable to determine type" << std::flush;
        return false;
    }

    if (params[1].size() + 10 > PATH_MAX) {
        //Php::warning << "Path is larger than " << PATH_MAX << " chars - Unable to proceed" << std::flush;
        return false;
    }

    if (params.size() == 3) {
        resolution = params[2];
    }

    renderer                  = new poppler::page_renderer();
    std::string outputPattern = params[1].stringValue();
    size_t found              = outputPattern.find_last_of("/");

    if (found == std::string::npos) {
        throw Php::Exception("Unable to locate output directory");
    }

    char * outputDirectory = realpath(outputPattern.substr(0, found).c_str(), NULL);

    if (outputDirectory == NULL) {
        throw Php::Exception("Unable to file output directory pattern: "+ outputPattern.substr(0, found) );
    }

    std::string outputFile = outputPattern.substr(found+1);

    // Check that the directory exists
    if (!file_exists(outputDirectory) && _mkdir(outputDirectory) != 0) {
        Php::warning << "PHP-PDF: Unable to create dir " << outputDirectory << " because '" << strerror(errno) << "'" << std::flush;
        throw Php::Exception("Unable to create directory");
    }

    int imageWidth = 0;
    int imageHeight = 0;
    int pageWidth = 0;
    int pageHeight = 0;
    char *outFile = (char*)malloc(PATH_MAX);

    for (x = firstPage; x < lastPage; x++) {
        imageWidth = 0;
        imageHeight = 0;
        pageWidth = 0;
        pageHeight = 0;

        memset(outFile, 0, PATH_MAX);
        sprintf(outFile, "%s/%s-%d.%s", outputDirectory, outputFile.c_str(), x, format->getExtension());

        page = _document->create_page(x);

        poppler::rectf pageDim = page->page_rect();
        poppler::page::orientation_enum orientation = page->orientation();

        if (orientation == poppler::page::orientation_enum::landscape || orientation == poppler::page::orientation_enum::seascape) {
            pageWidth = pageDim.height();
            pageHeight = pageDim.width();
        } else {
            pageWidth = pageDim.width();
            pageHeight = pageDim.height();
        }

        image = renderer->render_page(page, resolution, resolution, 0, 0, -1, -1, poppler::rotation_enum::rotate_0);
        image.save(outFile, format->getFormat(), resolution);

        imageWidth = image.width();
        imageHeight = image.height();

        returnValue[x] = Php::Object("\\PDF\\PdfImageResult", new PdfImageResult(pageWidth, pageHeight, imageWidth, imageHeight, outFile));
    }

    free(outputDirectory);

    return returnValue;
}

Php::Value hashDocument(Php::Parameters &params) {
    return hashPdfFileContents(params[0].stringValue());
}

Php::Value PdfDocument::hash(Php::Parameters &params) {
    return hashPdfFileContents(filePath);
}

Php::Value PdfDocument::compare(Php::Parameters &params) {
    PdfDocument *document = (PdfDocument *)params[0].implementation();

    poppler::page *localPage;
    poppler::page *externalPage;
    poppler::ustring localPageData;
    poppler::ustring externalPageData;

    poppler::byte_array localPageText_arr;
    poppler::byte_array externalPageText_arr;

    char *localPageText_str;
    char *externalPageText_str;

    int textDifference = 0;

    int count = _document->pages();
    if (count != document->_document->pages()) {
        return 10;
    }

    for (int pageNum = 0; pageNum < count; ++pageNum) {
        localPage = _document->create_page(pageNum);
        if (!localPage) {
            //Php::warning << "PHP-PDF: Failed to read local page" << pageNum+1 << std::flush;

            return -1;
        }

        externalPage = document->_document->create_page(pageNum);
        if (!externalPage) {
            //Php::warning << "PHP-PDF: Failed to read " << pageNum+1 << std::flush;

            return -1;
        }

        localPageData = localPage->text();
        externalPageData = externalPage->text();

        if (localPageData.length() != externalPageData.length()) {
            return 12;
        }

        localPageText_arr = localPageData.to_utf8();
        localPageText_str = &localPageText_arr[0];

        externalPageText_arr = externalPageData.to_utf8();
        externalPageText_str = &externalPageText_arr[0];

        textDifference = std::memcmp(localPageText_str, externalPageText_str, localPageData.length());
        if (textDifference != 0) {
            return 13;
        }
    }

    return 0;
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

