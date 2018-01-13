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

class PdfWriter : public Php::Base {
private:
    PDFWriter writer;
    PDFModifiedPage * modifiedPage;
    PDFObject * page;
    AbstractContentContext * contentContext;
    AbstractContentContext::TextOptions * textOptions;
    std::string defaultFontName;
    PDFUsedFont * defaultFont = NULL;
    AbstractContentContext::TextOptions * defaultText;
    int64_t pageNum;
    std::map<std::string,std::string> allFonts;
    void initializeFontDir();
    AbstractContentContext::TextOptions * getFont(std::string requestedFont, double inFontSize = 10);
public:
    PdfWriter();
    void __construct(Php::Parameters &params);
    void writeTextToPage(Php::Parameters &params);
    void writePdf();
    void setFont(Php::Parameters &params);
    Php::Value getAllFonts();
};

#endif /* PDF_WRITER_H_ */
