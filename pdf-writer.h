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

class PdfText : public Php::Base {
private:
    Php::Value x;
    Php::Value y;
    Php::Value text;
public:
    PdfText();
    void __construct(Php::Parameters &params);
    Php::Value getX();
    Php::Value getY();
    Php::Value getText();
};

class PdfWriter : public Php::Base {
private:
    PDFWriter writer;
    PDFModifiedPage * modifiedPage;
    PDFObject * page;
    AbstractContentContext * contentContext;
    AbstractContentContext::TextOptions * textOptions;

    int64_t pageNum;
public:
    PdfWriter();
    void __construct(Php::Parameters &params);
    void writeTextToPage(Php::Parameters &params);
    void writePdf();
};

#endif /* PDF_WRITER_H_ */
