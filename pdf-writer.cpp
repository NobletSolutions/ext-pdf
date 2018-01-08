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
#include "pdf-text.h"
#include <sys/stat.h>
#include <stdio.h>

PdfWriter::PdfWriter() = default;

void PdfWriter::__construct(Php::Parameters &params) {
    font = NULL;
    struct stat buffer;
    if (stat (params[0], &buffer) != 0) {
        throw Php::Exception("File doesn't exist?");
    }
    writer.ModifyPDF(params[0], ePDFVersion14, params[1]);
}

void PdfWriter::setFont(Php::Parameters &params) {
    struct stat buffer;
    char * temp = NULL;
    std::string fontDir = Php::ini_get("pdf.font_dir");

    if (fontDir.empty()) {
	    fontDir.assign(FONT_DIR);
    }

    if (fontDir.back() == '/') {
        asprintf(&temp, "%s.ttf", params[0].stringValue().c_str());
    } else {
        asprintf(&temp, "/%s.ttf", params[0].stringValue().c_str());
    }

    fontDir.append(temp);
    free(temp);

    // Check that the font file exists
    if (stat (fontDir.c_str(), &buffer) == -1) {
        fontDir.append(" - Doesn't exist");
        throw Php::Exception(fontDir);
    }

    font = writer.GetFontForFile(fontDir,0);

    if (!font) {
        throw Php::Exception("Unable to locate font");
    }
}

void PdfWriter::writeTextToPage(Php::Parameters &params) {
    if (params[0] < 0) {
        throw Php::Exception("Cannot write to a negative page");
    }

    if (params[1].size() > 0) {
        PDFModifiedPage thePage(&writer, static_cast<double>(params[0]), true);
        AbstractContentContext* contentContext = thePage.StartContentContext();

        if (font == NULL) {
            struct stat buffer;
            std::string fontDir = Php::ini_get("pdf.font_dir");
            if (fontDir.empty()) {
                fontDir.assign(FONT_DIR);
            }

            if (fontDir[fontDir.size()] == '/') {
                fontDir.append("arial.ttf");
            } else {
                fontDir.append("/arial.ttf");
            }

            // Check that the font file exists
            if (stat (fontDir.c_str(), &buffer) == -1) {
                fontDir.append(" - Doesn't exist");
                throw Php::Exception(fontDir);
            }

            font = writer.GetFontForFile(fontDir, 0);
        }

        if (!font) {
            throw Php::Exception("Unable to locate font");
        }

        AbstractContentContext::TextOptions defaultOptions(font, 10, AbstractContentContext::eRGB, 0);
        for (auto &iter : params[1]) {
            PdfText *obj = (PdfText *) iter.second.implementation();
            if (obj->getFontSize() || obj->getFont() ) {
                PDFUsedFont * customFont = NULL;
                if(obj->getFont()) {
                    struct stat buffer;
                    std::string fontDir = Php::ini_get("pdf.font_dir");
                    if (fontDir.empty()) {
                        fontDir.assign(FONT_DIR);
                    }
                    char * temp = NULL;

                    if (fontDir[fontDir.size()] == '/') {
                        asprintf(&temp,"%s.ttf",obj->getFont().buffer());
                    } else {
                        asprintf(&temp,"/%s.ttf",obj->getFont().buffer());
                    }

                    fontDir.append(temp);
                    free(temp);

                    // Check that the font file exists
                    if (stat (fontDir.c_str(), &buffer) == -1) {
                        fontDir.append(" - Doesn't exist");
                        throw Php::Exception(fontDir);
                    }

                    customFont = writer.GetFontForFile(fontDir, 0);
                    if (!customFont) {
                        throw Php::Exception("Unable to locate font");
                    }
                } else {
                    customFont = font;
                }

                int _fontSize =10;
                if(obj->getFontSize()) {
                    _fontSize = (int)obj->getFontSize();
                }

                AbstractContentContext::TextOptions options(customFont, _fontSize, AbstractContentContext::eRGB, 0);
                contentContext->WriteText((double) obj->getX(), (double) obj->getY(), obj->getText(), options);
            } else {
                contentContext->WriteText((double) obj->getX(), (double) obj->getY(), obj->getText(), defaultOptions);
            }
        }

        thePage.EndContentContext();
        thePage.WritePage();
    }

    return;
}

void PdfWriter::writePdf() {
    writer.EndPDF();
}
