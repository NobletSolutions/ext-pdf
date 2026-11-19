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

void PdfDocument::__construct(Php::Parameters &params) {
    if (!file_exists(params[0])) {
        Php::warning << "File " << params[0] << " doesn't exist" << std::flush;
        throw Php::Exception("File doesn't exist?");
    }

    filePath = params[0].stringValue();
    const std::string ownerPassword = params.size() > 1 ? params[1].stringValue() : std::string();
    const std::string userPassword = params.size() > 2 ? params[2].stringValue() : std::string();
    _document.reset(poppler::document::load_from_file(filePath, ownerPassword, userPassword));

    if (!_document) {
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
    const std::string outputPattern = params[1].stringValue();
    if (outputPattern.size() + 10 > PATH_MAX) {
        return false;
    }

    const size_t slash = outputPattern.find_last_of('/');
    if (slash == std::string::npos) {
        throw Php::Exception("Unable to locate output directory");
    }

    const std::string outputDirectory = outputPattern.substr(0, slash == 0 ? 1 : slash);
    if (!file_exists(outputDirectory.c_str()) && _mkdir(outputDirectory.c_str()) != 0) {
        Php::warning << "PHP-PDF: Unable to create dir " << outputDirectory.c_str() << " because '" << strerror(errno) << "'" << std::flush;
        throw Php::Exception("Unable to create directory");
    }

    const int resolution = params.size() == 3 ? static_cast<int>(params[2]) : 75;
    PdfImageFormat format = getImageFormat(params[0]);
    poppler::page_renderer renderer;
    Php::Value returnValue;

    const int pageCount = _document->pages();
    for (int index = 0; index < pageCount; ++index) {
        const std::unique_ptr<poppler::page> page(_document->create_page(index));
        if (!page) {
            throw Php::Exception("Unable to read PDF page " + std::to_string(index + 1));
        }

        const poppler::rectf dimensions = page->page_rect();
        const auto orientation = page->orientation();
        const bool sideways = orientation == poppler::page::landscape || orientation == poppler::page::seascape;
        const int pageWidth = sideways ? dimensions.height() : dimensions.width();
        const int pageHeight = sideways ? dimensions.width() : dimensions.height();

        const auto image = renderer.render_page(page.get(), resolution, resolution, 0, 0, -1, -1, poppler::rotation_enum::rotate_0);
        if (!image.is_valid()) {
            throw Php::Exception("Unable to render PDF page " + std::to_string(index + 1));
        }

        const std::string outFile = outputPattern + '-' + std::to_string(index) + '.' + format.getExtension();
        if (!image.save(outFile, format.getFormat(), resolution)) {
            throw Php::Exception("Unable to write image: " + outFile);
        }
        returnValue[index] = Php::Object("\\PDF\\PdfImageResult",
            new PdfImageResult(image.width(), image.height(), pageWidth, pageHeight, outFile));
    }

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

PdfImageFormat PdfDocument::getImageFormat(int inFormat) {
    switch (inFormat) {
        case 2:
            return PdfImageFormat("png", "png");
        case 3:
            return PdfImageFormat("tiff", "tif");
        default:
        case 1:
            return PdfImageFormat("jpeg", "jpg");
    }

}

