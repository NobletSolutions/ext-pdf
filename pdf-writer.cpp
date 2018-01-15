/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include "pdf-writer.h"

#include <fontconfig/fontconfig.h>
#include <PDFWriter/EPDFVersion.h>
#include <PDFWriter/PDFUsedFont.h>
#include <sys/stat.h>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "pdf-text.h"

std::map<std::string,std::string> allFonts;
std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

void initializeFonts() {
    if(allFonts.size()>0) {
	    return;
    }
    const FcChar8 *format = NULL;
    FcObjectSet   *os = NULL;
    FcFontSet     *fs = NULL;
    FcPattern     *pat = NULL;
    int j;

    std::string tmp;
    std::vector<std::string> tokens;
    std::vector<std::string> fontNames;

    pat     = FcNameParse ((FcChar8 *) ":style=Regular");
    os      = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, (char *) 0);
    format  = (const FcChar8 *) "%{+family,file{%{=unparse}}}";
    fs      = FcFontList (0, pat, os);

    if (os) {
        FcObjectSetDestroy (os);
    }

    if (pat) {
        FcPatternDestroy (pat);
    }

    for (j = 0; j < fs->nfont; j++)
    {
        FcChar8 *s;

        s = FcPatternFormat (fs->fonts[j], format);
        if (s) {
            tmp.assign((char*)s);
            FcStrFree (s);

            tokens = split((const std::string) tmp,':');
            if(tokens[1].find("php-pdf-fonts") != std::string::npos) {
                fontNames = split((const std::string)tokens[0],',');
                tokens[1].erase(tokens[1].begin(), tokens[1].begin()+5);
                allFonts.insert(std::make_pair(fontNames[0], tokens[1]));
            }
        }
    }

    FcFini ();
}

std::vector<std::string> getFonts(){
    if (allFonts.empty()) {
        initializeFonts();
    }

    std::vector<std::string> fonts;
    for(std::map<std::string,std::string>::iterator it = allFonts.begin(); it != allFonts.end(); ++it) {
      fonts.push_back(it->first);
    }

    return fonts;
}

Php::Value fonts() {
    return getFonts();
}

PdfWriter::PdfWriter() = default;

void PdfWriter::__construct(Php::Parameters &params) {
    struct stat buffer;
    if (stat (params[0], &buffer) != 0) {
        throw Php::Exception("File doesn't exist?");
    }

    writer.ModifyPDF(params[0], ePDFVersion14, params[1]);

    initializeFonts();
}

AbstractContentContext::TextOptions * PdfWriter::getFont(std::string requestedFont, double inFontSize)
{
    AbstractContentContext::TextOptions * options;
    std::map<std::string,std::string>::iterator it;
    struct stat buffer;
    PDFUsedFont * _font;

    it = allFonts.find(requestedFont);

    if (it != allFonts.end()) {
        // Check that the font file exists
        if (stat (it->second.c_str(), &buffer) == -1) {
            throw Php::Exception("Font doesn't exist");
        }

        _font = writer.GetFontForFile(it->second.c_str(), 0);
        if (!_font) {
            throw Php::Exception("Unable to locate font");
        }

        options = new AbstractContentContext::TextOptions(_font, inFontSize, AbstractContentContext::eRGB, 0);

        return options;
    }

    Php::out << "Tried to get font: "<< requestedFont << std::endl;
    throw Php::Exception("No such Font");
}

void PdfWriter::setFont(Php::Parameters &params) {
    defaultFontName.assign(params[0].stringValue());
    if(params.size() == 2) {
        defaultText = this->getFont(params[0].stringValue(), (double)params[1]);
    } else if (params.size() == 1) {
        defaultText = this->getFont(params[0].stringValue(), 10);
    }
}

void PdfWriter::writeTextToPage(Php::Parameters &params) {
    if (params[0] < 0) {
        throw Php::Exception("Cannot write to a negative page");
    }

    if (params[1].size() > 0) {
        PDFModifiedPage thePage(&writer, static_cast<double>(params[0]), true);
        AbstractContentContext* contentContext = thePage.StartContentContext();

        if (!defaultText) {
            throw Php::Exception("No font set!");
        }

        AbstractContentContext::TextOptions * options;

        // iterate over each PdfText point
        for (auto &iter : params[1]) {
            PdfText *obj = (PdfText *) iter.second.implementation();

            if (obj->getFontSize() || obj->getFont() ) {
                int _fontSize = 10;
                std::string fontName(obj->getFont().stringValue());
                if(obj->getFontSize()) {
                    _fontSize = (int)obj->getFontSize();
                }

                if(fontName.empty()){
                    fontName.assign(defaultFontName);
                }

                options = this->getFont(fontName, _fontSize);
            } else {
                options = defaultText;
            }

            if (obj->getText().stringValue().find("\n") != std::string::npos) {
                std::vector<std::string> tokens = split(obj->getText().stringValue(),'\n');
                double lineHeight = 0;
                PDFUsedFont::TextMeasures textDimensions = options->font->CalculateTextDimensions("H",14);

                for (std::vector<std::string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
                    contentContext->WriteText((double) obj->getX(), (double) obj->getY()-lineHeight, *it, *options);
                    lineHeight += textDimensions.height;
                }
                continue;
            }

            contentContext->WriteText((double) obj->getX(), (double) obj->getY(), obj->getText(), *options);
        }

        thePage.EndContentContext();
        thePage.WritePage();
    }

    return;
}

void PdfWriter::writePdf() {
    writer.EndPDF();
}

Php::Value PdfWriter::getAllFonts() {
    return getFonts();
}

