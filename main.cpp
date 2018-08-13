#include <phpcpp.h>
#include <iostream>
#include "pdf-document.h"
#include "pdf-image-result.h"
#include "pdf-image-format.h"
#include "pdf-writer.h"
#include "pdf-text.h"
#include "pdf-image.h"

#ifndef VERSION
#define VERSION "0.10"
#endif

/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {
    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  structure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("pdf", VERSION);

        Php::Namespace myNamespace("PDF");

        // PdfImage Methods ========================
        Php::Class<PdfImage> pdfImage("PdfImage");
        pdfImage.method<&PdfImage::__construct>("__construct", {
            Php::ByVal("x", Php::Type::Numeric),
            Php::ByVal("y", Php::Type::Numeric),
            Php::ByVal("imagepath", Php::Type::String),
			Php::ByVal("index", Php::Type::Numeric, false),
            Php::ByVal("width", Php::Type::Numeric, false),
            Php::ByVal("height", Php::Type::Numeric, false)
        });
        pdfImage.method<&PdfImage::getX>("getX");
        pdfImage.method<&PdfImage::getY>("getY");
        pdfImage.method<&PdfImage::getIndex>("getIndex");
        pdfImage.method<&PdfImage::getImagePath>("getImagePath");
        pdfImage.method<&PdfImage::getWidth>("getWidth");
        pdfImage.method<&PdfImage::getHeight>("getHeight");

        // PdfText Methods =========================
        Php::Class<PdfText> pdfText("PdfText");
        pdfText.method<&PdfText::__construct>("__construct", {
            Php::ByVal("x", Php::Type::Numeric),
            Php::ByVal("y", Php::Type::Numeric),
            Php::ByVal("text", Php::Type::String),
            Php::ByVal("fontSize", Php::Type::Numeric, false),
            Php::ByVal("font", Php::Type::String, false)
        });

        pdfText.method<&PdfText::getX>("getX");
        pdfText.method<&PdfText::getY>("getY");
        pdfText.method<&PdfText::getText>("getText");
        pdfText.method<&PdfText::getText>("__toString");

        // PdfWriter Methods =========================
        Php::Class<PdfWriter> pdfWriter("PdfWriter");
        pdfWriter.method<&PdfWriter::__construct>("__construct", {
            Php::ByVal("inputFile", Php::Type::String),
            Php::ByVal("outputFile", Php::Type::String)
        });

        pdfWriter.method<&PdfWriter::getAllFonts>("getAllFonts");
        pdfWriter.method<&PdfWriter::setFont>("setFont", {
			Php::ByVal("font",Php::Type::String)
        });

        pdfWriter.method<&PdfWriter::writeTextToPage>("writeTextToPage",{
            Php::ByVal("page",Php::Type::Numeric),
            Php::ByVal("modifications", Php::Type::Array)
        });

        pdfWriter.method<&PdfWriter::writeImageToPage>("writeImageToPage",{
            Php::ByVal("page",Php::Type::Numeric),
            Php::ByVal("image","PDF\\PdfImage")
        });

        pdfWriter.method<&PdfWriter::writePdf>("save",{
                Php::ByVal("ranges", Php::Type::Array, false)
            });

        Php::Class<PdfImageResult> PdfImageResult("PdfImageResult");
        PdfImageResult.method<&PdfImageResult::__construct>("__construct", Php::Private);
        PdfImageResult.method<&PdfImageResult::getImageWidth>("getImageWidth");
        PdfImageResult.method<&PdfImageResult::getImageHeight>("getImageHeight");
        PdfImageResult.method<&PdfImageResult::getPageWidth>("getPageWidth");
        PdfImageResult.method<&PdfImageResult::getPageHeight>("getPageHeight");
        PdfImageResult.method<&PdfImageResult::getPage>("getPage");

        Php::Class<PdfDocument> pdfDocument("PdfDocument");
        pdfDocument.add(Php::Constant("IMAGE_JPEG",1));
        pdfDocument.add(Php::Constant("IMAGE_PNG",2));
        pdfDocument.add(Php::Constant("IMAGE_TIFF",3));

        pdfDocument.method<&PdfDocument::__construct>("__construct", {
            Php::ByVal("file", Php::Type::String),
            Php::ByVal("user", Php::Type::String, false),
            Php::ByVal("password", Php::Type::String, false)
        });

        pdfDocument.method<&PdfDocument::getMajorVersion>("getMajorVersion");
        pdfDocument.method<&PdfDocument::getMinorVersion>("getMinorVersion");
        pdfDocument.method<&PdfDocument::getCreationDate>("getCreationDate");
        pdfDocument.method<&PdfDocument::getModifiedDate>("getModifiedDate");
        pdfDocument.method<&PdfDocument::getCreator>("getCreator");
        pdfDocument.method<&PdfDocument::hasEmbeddedFiles>("hasEmbeddedFiles");
        pdfDocument.method<&PdfDocument::isLocked>("isLocked");
        pdfDocument.method<&PdfDocument::isEncrypted>("isEncrypted");
        pdfDocument.method<&PdfDocument::isLinear>("isLinear");
        pdfDocument.method<&PdfDocument::numberOfPages>("getNumberOfPages");
        pdfDocument.method<&PdfDocument::asString>("asString");
        pdfDocument.method<&PdfDocument::toImage>("toImage", {
            Php::ByVal("format", Php::Type::Constant),
            Php::ByVal("output", Php::Type::String),
            Php::ByVal("dpi", Php::Type::Numeric,false)
        });

        myNamespace.add<fonts>("getFonts");
        myNamespace.add(PdfImageResult);
        myNamespace.add(pdfDocument);
        myNamespace.add(pdfText);
        myNamespace.add(pdfImage);
        myNamespace.add(pdfWriter);
        extension.add(myNamespace);

        // return the extension
        return extension;
    }
}

