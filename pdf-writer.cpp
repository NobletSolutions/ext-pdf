/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <iostream>
#include <string.h>
#include "pdf-writer.h"

PdfText::PdfText() = default;

void PdfText::__construct(Php::Parameters &params) {
//        Php::out << "x: " << params[0] << std::endl;
//        Php::out << "y: " << params[1] << std::endl;
//        Php::out << "Text: " << params[2] << std::endl;
    x = params[0];
    y = params[1];
    text = params[2];
}

Php::Value PdfText::getX() {
    return x;
}

Php::Value PdfText::getY() {
    return y;
}

Php::Value PdfText::getText() {
    return text;
}

PdfWriter::PdfWriter() = default;

void PdfWriter::__construct(Php::Parameters &params) {
//        Php::out << "InputFile" << params[0] << std::endl;
//        Php::out << "OutputFile" << params[1] << std::endl;

    writer.ModifyPDF(params[0], ePDFVersion14, params[1]);
}

void PdfWriter::writeTextToPage(Php::Parameters &params) {
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

        PDFUsedFont * font = writer.GetFontForFile("./arial.ttf", 0);
        if (!font) {
//                Php::out << "Failed to retrieve font ./arial.ttf" << std::endl;
            return;
        }

        AbstractContentContext::TextOptions opt(font, 12,
                AbstractContentContext::eRGB, 0);
        for (auto &iter : params[1]) {
            PdfText *obj = (PdfText *) iter.second.implementation();
            contentContext->WriteText((double) obj->getX(),
                    (double) obj->getY(), obj->getText(), opt);
        }

        thePage.EndContentContext();
        thePage.WritePage();
    }

    return;
}

void PdfWriter::writePdf() {
    writer.EndPDF();
}

