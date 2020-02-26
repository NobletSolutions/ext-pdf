#include <phpcpp.h>
#include <iostream>
#include "pdf-document.h"
#include "pdf-image-result.h"
#include "pdf-image-format.h"
#include "pdf-writer.h"
#include "pdf-text.h"
#include "pdf-image.h"
#include "pdf-rectangle.h"
#include "pdf-line.h"

#ifndef VERSION
#define VERSION "0.11.5"
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
        Php::Class<PdfImage> pdfImage("PdfImage", 0);
        pdfImage.method<&PdfImage::__construct>("__construct", Php::Public, {
            Php::ByVal("x", Php::Type::Numeric),
            Php::ByVal("y", Php::Type::Numeric),
            Php::ByVal("imagepath", Php::Type::String),
			Php::ByVal("index", Php::Type::Numeric, false),
            Php::ByVal("width", Php::Type::Numeric, false),
            Php::ByVal("height", Php::Type::Numeric, false)
        });
        pdfImage.method<&PdfImage::getX>("getX", Php::Public);
        pdfImage.method<&PdfImage::getY>("getY", Php::Public);
        pdfImage.method<&PdfImage::getIndex>("getIndex", Php::Public);
        pdfImage.method<&PdfImage::getImagePath>("getImagePath", Php::Public);
        pdfImage.method<&PdfImage::getWidth>("getWidth", Php::Public);
        pdfImage.method<&PdfImage::getHeight>("getHeight", Php::Public);

        // PdfText Methods =========================
        Php::Class<PdfText> pdfText("PdfText");
        pdfText.method<&PdfText::__construct>("__construct", Php::Public, {
            Php::ByVal("x", Php::Type::Numeric),
            Php::ByVal("y", Php::Type::Numeric),
            Php::ByVal("text", Php::Type::String),
            Php::ByVal("fontSize", Php::Type::Numeric, false),
            Php::ByVal("font", Php::Type::String, false)
        });

        pdfText.method<&PdfText::getX>("getX", Php::Public);
        pdfText.method<&PdfText::getY>("getY", Php::Public);
        pdfText.method<&PdfText::getText>("getText", Php::Public);
        pdfText.method<&PdfText::getText>("__toString", Php::Public);

        // PdfRectangle Methods =========================
        Php::Class<PdfRectangle> pdfRectangle("PdfRectangle");
        pdfRectangle.method<&PdfRectangle::__construct>("__construct", Php::Public, {
            Php::ByVal("x", Php::Type::Numeric),
            Php::ByVal("y", Php::Type::Numeric),
            Php::ByVal("width", Php::Type::Numeric),
            Php::ByVal("height", Php::Type::Numeric),
	    Php::ByVal("red", Php::Type::Numeric,false),
	    Php::ByVal("green", Php::Type::Numeric,false),
	    Php::ByVal("blue", Php::Type::Numeric,false)
        });

        pdfRectangle.method<&PdfRectangle::getX>("getX", Php::Public);
        pdfRectangle.method<&PdfRectangle::getY>("getY", Php::Public);
        pdfRectangle.method<&PdfRectangle::getHeight>("getHeight", Php::Public);
        pdfRectangle.method<&PdfRectangle::getWidth>("getWidth", Php::Public);

	pdfRectangle.method<&PdfRectangle::getRed>("getRed", Php::Public);
        pdfRectangle.method<&PdfRectangle::getGreen>("getGreen", Php::Public);
        pdfRectangle.method<&PdfRectangle::getBlue>("getBlue", Php::Public);

        // PdfLine Methods =========================
        Php::Class<PdfLine> pdfLine("PdfLine");
        pdfLine.method<&PdfLine::__construct>("__construct", Php::Public, {
            Php::ByVal("x", Php::Type::Numeric),
            Php::ByVal("y", Php::Type::Numeric),
            Php::ByVal("x2", Php::Type::Numeric),
            Php::ByVal("y2", Php::Type::Numeric),
            Php::ByVal("red", Php::Type::Numeric,false),
            Php::ByVal("green", Php::Type::Numeric,false),
            Php::ByVal("blue", Php::Type::Numeric,false)
        });

	pdfLine.method<&PdfLine::setWidth>("setWidth", Php::Public, {
	    Php::ByVal("width", Php::Type::Numeric)
	});
	pdfLine.method<&PdfLine::getWidth>("getWidth", Php::Public);
        pdfLine.method<&PdfLine::getX>("getX", Php::Public);
        pdfLine.method<&PdfLine::getY>("getY", Php::Public);
        pdfLine.method<&PdfLine::getX2>("getX2", Php::Public);
        pdfLine.method<&PdfLine::getY2>("getY2", Php::Public);
        pdfLine.method<&PdfLine::getRed>("getRed", Php::Public);
        pdfLine.method<&PdfLine::getGreen>("getGreen", Php::Public);
        pdfLine.method<&PdfLine::getBlue>("getBlue", Php::Public);

        // PdfWriter Methods =========================
        Php::Class<PdfWriter> pdfWriter("PdfWriter");
        pdfWriter.method<&PdfWriter::__construct>("__construct", Php::Public, {
            Php::ByVal("inputFile", Php::Type::String),
            Php::ByVal("outputFile", Php::Type::String)
        });

        pdfWriter.method<&PdfWriter::getAllFonts>("getAllFonts", Php::Public);
        pdfWriter.method<&PdfWriter::setFont>("setFont", Php::Public, {
			Php::ByVal("font",Php::Type::String)
        });

        pdfWriter.method<&PdfWriter::writeTextToPage>("writeTextToPage", Php::Public, {
            Php::ByVal("page",Php::Type::Numeric),
            Php::ByVal("modifications", Php::Type::Array)
        });

        pdfWriter.method<&PdfWriter::drawRectangleToPage>("drawRectangleToPage", Php::Public, {
            Php::ByVal("page",Php::Type::Numeric),
            Php::ByVal("modifications", Php::Type::Array)
        });

        pdfWriter.method<&PdfWriter::drawLineToPage>("drawLineToPage", Php::Public, {
            Php::ByVal("page",Php::Type::Numeric),
            Php::ByVal("modifications", Php::Type::Array)
        });

        pdfWriter.method<&PdfWriter::writeImageToPage>("writeImageToPage", Php::Public, {
            Php::ByVal("page",Php::Type::Numeric),
            Php::ByVal("image","PDF\\PdfImage")
        });

        pdfWriter.method<&PdfWriter::writePdf>("save", Php::Public, {
            Php::ByVal("ranges", Php::Type::Array, false)
        });

        pdfWriter.method<&PdfWriter::combine>("combine", Php::Public, {
            Php::ByVal("documents", Php::Type::Array),
            Php::ByVal("destination", Php::Type::String),
        });
 
        Php::Class<PdfImageResult> PdfImageResult("PdfImageResult");
        PdfImageResult.method<&PdfImageResult::__construct>("__construct", Php::Public);
        PdfImageResult.method<&PdfImageResult::getImageWidth>("getImageWidth", Php::Public);
        PdfImageResult.method<&PdfImageResult::getImageHeight>("getImageHeight", Php::Public);
        PdfImageResult.method<&PdfImageResult::getPageWidth>("getPageWidth", Php::Public);
        PdfImageResult.method<&PdfImageResult::getPageHeight>("getPageHeight", Php::Public);
        PdfImageResult.method<&PdfImageResult::getPage>("getPage", Php::Public);

        Php::Class<PdfDocument> pdfDocument("PdfDocument");
        pdfDocument.constant("IMAGE_JPEG", 1);
        pdfDocument.constant("IMAGE_PNG", 2);
        pdfDocument.constant("IMAGE_TIFF", 3);

        pdfDocument.method<&PdfDocument::__construct>("__construct", Php::Public, {
            Php::ByVal("file", Php::Type::String),
            Php::ByVal("user", Php::Type::String, false),
            Php::ByVal("password", Php::Type::String, false)
        });

        pdfDocument.method<&PdfDocument::getMajorVersion>("getMajorVersion", Php::Public);
        pdfDocument.method<&PdfDocument::getMinorVersion>("getMinorVersion", Php::Public);
        pdfDocument.method<&PdfDocument::getCreationDate>("getCreationDate", Php::Public);
        pdfDocument.method<&PdfDocument::getModifiedDate>("getModifiedDate", Php::Public);
        pdfDocument.method<&PdfDocument::getCreator>("getCreator", Php::Public);
        pdfDocument.method<&PdfDocument::hasEmbeddedFiles>("hasEmbeddedFiles", Php::Public);
        pdfDocument.method<&PdfDocument::isLocked>("isLocked", Php::Public);
        pdfDocument.method<&PdfDocument::isEncrypted>("isEncrypted", Php::Public);
        pdfDocument.method<&PdfDocument::isLinear>("isLinear", Php::Public);

        pdfDocument.method<&PdfDocument::hasJS>("hasJS", Php::Public);
        pdfDocument.method<&PdfDocument::hasXFA>("hasXFA", Php::Public);

        pdfDocument.method<&PdfDocument::numberOfPages>("getNumberOfPages", Php::Public);
        pdfDocument.method<&PdfDocument::asString>("asString", Php::Public);
        pdfDocument.method<&PdfDocument::toImage>("toImage", Php::Public, {
            Php::ByVal("format", Php::Type::Constant),
            Php::ByVal("output", Php::Type::String),
            Php::ByVal("dpi", Php::Type::Numeric,false)
        });

        pdfDocument.method<&PdfDocument::compare>("compare", Php::Public, {
            Php::ByRef("document", "PDF\\PdfDocument", false)
        });

        pdfDocument.method<&PdfDocument::hash>("hashContent", Php::Public);
        pdfDocument.method<&hashDocument>("hashDocument", Php::Public, {Php::ByVal("file", Php::Type::String)});

        myNamespace.add<fonts>("getFonts");
        myNamespace.add(PdfImageResult);
        myNamespace.add(pdfDocument);
        myNamespace.add(pdfText);
	myNamespace.add(pdfRectangle);
	myNamespace.add(pdfLine);
        myNamespace.add(pdfImage);
        myNamespace.add(pdfWriter);
        extension.add(myNamespace);

        // return the extension
        return extension;
    }
}

