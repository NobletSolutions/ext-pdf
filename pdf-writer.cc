/*
 * pdf.cc
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <iostream>
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
    PdfText() = default;

    void __construct(Php::Parameters &params)
    {
//        Php::out << "x: " << params[0] << std::endl;
//        Php::out << "y: " << params[1] << std::endl;
//        Php::out << "Text: " << params[2] << std::endl;
        x = params[0];
        y = params[1];
        text = params[2];
    }

    Php::Value getX() {
        return x;
    }

    Php::Value getY() {
        return y;
    }

    Php::Value getText() {
        return text;
    }
};

class PdfWriter : public Php::Base {
private:
    PDFWriter writer;
    PDFModifiedPage * modifiedPage = NULL;
    PDFObject * page = NULL;
    AbstractContentContext * contentContext = NULL;
    AbstractContentContext::TextOptions * textOptions = NULL;

    int64_t pageNum = -1;
public:
    PdfWriter() = default;

    void __construct(Php::Parameters &params)
    {
//        Php::out << "InputFile" << params[0] << std::endl;
//        Php::out << "OutputFile" << params[1] << std::endl;

        writer.ModifyPDF(params[0], ePDFVersion14, params[1]);
    }

    void writeTextToPage(Php::Parameters &params)
    {
        if (params[0] < 0) {
//            Php::out << "Page Is Negative!" << std::endl;
            return;
        }

//        Php::out << "Page: " << params[0] << std::endl;
//        Php::out << "Count: " << params[1].size() << std::endl;
//        for (auto &iter : params[1]) {
//            PdfText *obj = (PdfText *)iter.second.implementation();
//
//            Php::out << "Loop: " << iter.first << std::endl;
//            Php::out << "   x: " << obj->getX() << std::endl;
//            Php::out << "   y: " << obj->getY() << std::endl;
//            Php::out << "   t: " << obj->getText() << std::endl;
//        }

        if (params[1].size() > 0) {
            PDFModifiedPage thePage(&writer, static_cast<double>(params[0]), true);
            AbstractContentContext* contentContext = thePage.StartContentContext();

            PDFUsedFont * font = writer.GetFontForFile("./arial.ttf",0);
            if (!font) {
//                Php::out << "Failed to retrieve font ./arial.ttf" << std::endl;
                return;
            }

            AbstractContentContext::TextOptions opt(font,12,AbstractContentContext::eRGB, 0);
            for (auto &iter : params[1]) {
                PdfText *obj = (PdfText *)iter.second.implementation();
                contentContext->WriteText((double)obj->getX(), (double)obj->getY(), obj->getText(), opt);
            }

            thePage.EndContentContext();
            thePage.WritePage();
        }

        return;
    }

    void writePdf()
    {
        writer.EndPDF();
    }
};
