/*
 * poppler.cc
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-rectangle.h>
#include <poppler-global.h>
#include <poppler-page-renderer.h>

class PdfImageResult : public Php::Base {
private:
    int imageWidth  =-1;
    int imageHeight =-1;
    int pageWidth   =-1;
    int pageHeight  =-1;
    std::vector<std::string> pages;
public:
    PdfImageResult(int inImageWidth, int inImageHeight, int inPageWidth, int inPageHeight, std::vector<std::string> inPages) {
        imageWidth = inImageWidth;
        imageHeight = inImageHeight;
        pageWidth = inPageWidth;
        pageHeight = inPageHeight;
        pages = inPages;
    }

    void __construct() {}

    Php::Value getImageWidth() {
        return imageWidth;
    }

    Php::Value getImageHeight() {
        return imageHeight;
    }

    Php::Value getPageWidth() {
        return pageWidth;
    }

    Php::Value getPageHeight() {
        return pageHeight;
    }

    Php::Value getNumberOfPages() {
        return (int)pages.size();
    }

    Php::Value getPages() {
        return pages;
    }
};

class PdfImageFormat {
private:
    char format[5];
    char ext[4];
public:
    PdfImageFormat(const char * inFormat, const char * inExt) {
        memset(&format,0,5);
        memset(&ext,0,4);
        strncpy(&format[0],inFormat,4);
        strncpy(&ext[0],inExt,3);
    }

    const char * getFormat() {
        return &format[0];
    }

    const char * getExtension() {
        return &ext[0];
    }
};

class PdfDocument : public Php::Base {
  private:
    int _major = 0;
    int _minor = 0;
    poppler::document *_document = NULL;
    PdfImageFormat * jpeg = new PdfImageFormat("jpeg","jpg");
    PdfImageFormat * png = new PdfImageFormat("png","png");
    PdfImageFormat * tiff = new PdfImageFormat("tiff","tif");
  public:
    PdfDocument() = default;

    void __construct(Php::Parameters &params)
    {
       _document = poppler::document::load_from_file(params[0]);//,params[1],params[2]);
    }

    Php::Value getMajorVersion()
    {
        if(_major == 0) {
            _document->get_pdf_version(&_major, &_minor);
        }

        return _major;
    }

    Php::Value getMinorVersion()
    {
        if(_major == 0) {
            _document->get_pdf_version(&_major, &_minor);
        }

        return _minor;
    }

    Php::Value hasEmbeddedFiles()
    {
        return _document->has_embedded_files();
    }

    Php::Value isEncrypted()
    {
        return _document->is_encrypted();
    }

    Php::Value isLinear()
    {
        return _document->is_linearized();
    }

    Php::Value isLocked()
    {
        return _document->is_locked();
    }

    Php::Value numberOfPages()
    {
        return _document->pages();
    }

    Php::Value asString()
    {
        int firstPage;
        int lastPage;
        int x;
        poppler::page *page;
        Php::Value result;
        poppler::ustring pageData;
        std::string resultData;

        firstPage = 1;
        lastPage = _document->pages();

        for(x=firstPage;x<lastPage;x++) {
            page = _document->create_page(x);
            pageData = page->text();
            resultData.append(pageData.to_latin1());
        }

        result = resultData;
        return result;
    }

    Php::Value toImage(Php::Parameters &params)
    {
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
        if(format == NULL) {
            Php::error << "Unable to determine type" << std::endl;
            return false;
        }
        pattern = params[1].buffer();

        if(strlen(pattern)+10 > 255) {
            Php::error << "Path is larger than 255 chars - Unable to proceed" << std::endl;
            return false;
        }

        if (params.size() == 3) {
            resolution = params[2];
        }

        renderer = new poppler::page_renderer();
        firstPage = 0;

        for (x=firstPage; x<lastPage; x++) {
            char * outFile = new char [255];
            memset (outFile, 0, 255);
            sprintf(outFile, "%s-%d.%s", pattern, x, format->getExtension());

            page = _document->create_page(x);

            poppler::rectf pageDim = page->page_rect();
            pageWidth = pageDim.width();
            pageHeight = pageDim.height();

            image = renderer->render_page(page,resolution,resolution,0,0,-1,-1,poppler::rotation_enum::rotate_0);
            image.save(outFile,format->getFormat(),resolution);

            imageWidth = image.width();
            imageHeight = image.height();

            pages.push_back(outFile);
        }

        PdfImageResult * obj = new PdfImageResult(pageWidth,pageHeight,imageWidth,imageHeight,pages);
        return Php::Object("\\PDF\\PdfImageResult", obj);
    }

  private:
    PdfImageFormat * getImageFormat(int inFormat) {
        switch(inFormat) {
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
};
