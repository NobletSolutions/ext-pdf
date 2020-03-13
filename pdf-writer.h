/*
 * pdf-writer.h
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#ifndef PDF_WRITER_H_
#define PDF_WRITER_H_

#include <phpcpp.h>
#include <PDFWriter/PDFWriter.h>
#include <PDFWriter/PDFModifiedPage.h>
#include <PDFWriter/PDFPage.h>
#include <PDFWriter/PDFObject.h>
#include <PDFWriter/AbstractContentContext.h>
#include <unordered_map>
#include "pdf-image.h"
#include "pdf-text.h"
#include "pdf-rectangle.h"
#include "pdf-line.h"

void initializeFonts();
std::vector<std::string> getFonts();
Php::Value fonts();

class PdfWriter : public Php::Base {
private:
    PDFWriter writer;
    PDFModifiedPage * modifiedPage = NULL;
    PDFObject * page;// = NULL;
    AbstractContentContext * contentContext;// = NULL;
    AbstractContentContext::TextOptions * textOptions;// = NULL;
    std::string defaultFontName;
    PDFUsedFont * defaultFont;// = NULL;
    AbstractContentContext::TextOptions * defaultText;// = NULL;
    int64_t pageNum = -1;
    AbstractContentContext::TextOptions * getFont(std::string requestedFont, double inFontSize = 10, int64_t color = 0);
    std::string _inputFileName;
    std::string _outputFileName;
    std::unordered_map<double,std::vector<PdfImage*>> pageImages;
    std::unordered_map<double,std::vector<PdfText*>> pageText;
    std::unordered_map<double,std::vector<PdfRectangle*>> pageRectangles;
    std::unordered_map<double,std::vector<PdfLine*>> pageLines;

    void writeText(PdfText *obj, int pageRotation, const PDFRectangle &mediaBox, AbstractContentContext *contentContext);
    void writeImage(PdfImage *image, AbstractContentContext *contentContext);
    void writeRectangle(PdfRectangle *rect, AbstractContentContext *contentContext);
    void writeLine(PdfLine *line, AbstractContentContext *contentContext);
public:
    PdfWriter();
    void __construct(Php::Parameters &params);
    void writeTextToPage(Php::Parameters &params);
    void drawRectangleToPage(Php::Parameters &params);
    void drawLineToPage(Php::Parameters &params);
    void writeImageToPage(Php::Parameters &params);
    void writePdf(Php::Parameters &params);
    void setFont(Php::Parameters &params);
    Php::Value getAllFonts();
    static Php::Value combine(Php::Parameters &params);
};

#endif /* PDF_WRITER_H_ */
