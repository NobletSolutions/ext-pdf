/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include "pdf-writer.h"

#include <fontconfig/fontconfig.h>
#include <PDFWriter/EStatusCode.h>
#include <PDFWriter/EPDFVersion.h>
#include <PDFWriter/PDFUsedFont.h>
#include <PDFWriter/PDFDocumentCopyingContext.h>
#include <PDFWriter/PageContentContext.h>
#include <PDFWriter/PDFPageInput.h>
#include <phpcpp.h>
#include <sys/stat.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "pdf-text.h"
#include "pdf-image.h"

inline bool file_exists(const std::string &filename ) {
    struct stat statBuffer;
    return (stat(filename.c_str(), &statBuffer) == 0);
}

std::map<std::string,std::string> allFonts;
std::vector<std::string> split(const std::string& s, char delimiter) {
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);

   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }

   return tokens;
}

void initializeFonts() {
    if (allFonts.size() > 0) {
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
            if (tokens[1].find("php-pdf-fonts") != std::string::npos) {
                fontNames = split((const std::string)tokens[0],',');
                // This is a hack since our Regular style filter is returning multiple fonts.
                // So if the font name has only one name we expect its the 'regular' typeface as well
                // I need a better way to use fc-list / library to only get the Regular style typeface
                if (fontNames.size() == 1) {
                    tokens[1].erase(tokens[1].begin(), tokens[1].begin()+5);
                    allFonts.insert(std::make_pair(fontNames[0], tokens[1]));
                }
            }
        }
    }

    FcFini ();
}

std::vector<std::string> getFonts() {
    if (allFonts.empty()) {
        initializeFonts();
    }

    std::vector<std::string> fonts;
    for (std::map<std::string,std::string>::iterator it = allFonts.begin(); it != allFonts.end(); ++it) {
      fonts.push_back(it->first);
    }

    return fonts;
}

Php::Value fonts() {
    return getFonts();
}

PdfWriter::PdfWriter() = default;

void PdfWriter::__construct(Php::Parameters &params) {
    if (!file_exists(params[0])) {
        Php::warning << "File " << params[0] << " doesn't exist" << std::flush;
        throw Php::Exception("File doesn't exist?");
    }

    _inputFileName.assign(params[0].stringValue());
    _outputFileName.assign(params[1].stringValue());
    EStatusCode status = writer.ModifyPDF(_inputFileName, ePDFVersion14, _outputFileName);
    if (status != eSuccess) {
        Php::warning << "Unable to open " << _inputFileName << std::flush;
        throw Php::Exception("Unable to open file");
    }

    initializeFonts();
}

AbstractContentContext::TextOptions * PdfWriter::getFont(std::string requestedFont, double inFontSize, int64_t colorValue) {
    std::map<std::string,std::string>::iterator it;
    PDFUsedFont * _font;

    it = allFonts.find(requestedFont);
    if (it != allFonts.end()) {
        // Check that the font file exists
        if (!file_exists(it->second.c_str())) {
            Php::warning << "Font " << requestedFont << " does not exist" << std::flush;
            throw Php::Exception("Font doesn't exist");
        }

        _font = writer.GetFontForFile(it->second.c_str(), 0);
        if (!_font) {
            Php::warning << "Unable to locate font " << requestedFont << std::flush;
            throw Php::Exception("Unable to locate font");
        }

        return new AbstractContentContext::TextOptions(_font, inFontSize, AbstractContentContext::eRGB, colorValue);
    }

    Php::warning << "No such font: " << requestedFont << std::flush;
    throw Php::Exception("No such font " + requestedFont);
}

void PdfWriter::setFont(Php::Parameters &params) {
    defaultFontName.assign(params[0].stringValue());
    if (params.size() == 2) {
        defaultText = this->getFont(params[0].stringValue(), (double)params[1], 0);
    } else if (params.size() == 1) {
        defaultText = this->getFont(params[0].stringValue(), 10, 0);
    }
}

void PdfWriter::writeImageToPage(Php::Parameters &params) {
    if (params[0] < 0) {
        Php::warning << "Unable to write to a negative page number" << std::flush;
        throw Php::Exception("Cannot write to a negative page");
    }

    double pageIndex = params[0];

    PdfImage *image = new PdfImage(*(PdfImage *)params[1].implementation());

    auto search = pageImages.find(pageIndex);
    if (search != pageImages.end()) {
        search->second.push_back(image);
    } else {
        std::vector<PdfImage*> v = {image};
        pageImages.insert({pageIndex,v});
    }
}

void PdfWriter::drawLineToPage(Php::Parameters &params) 
{
    if (params[0] < 0) {
        Php::warning << "Unable to write to a negative page number" << std::flush;
        throw Php::Exception("Cannot write to a negative page");
    }

    if (params[1].size() > 0) {
        double pageIndex = params[0];

        auto search = pageLines.find(pageIndex);
        if(search != pageLines.end()) {
            for(auto &iter : params[1]) {
                PdfLine *obj = new PdfLine(*(PdfLine*) iter.second.implementation());
                search->second.push_back(obj);
            }
        } else {
            std::vector<PdfLine*> v;
            for(auto &iter : params[1]) {
                PdfLine *obj = new PdfLine(*(PdfLine*) iter.second.implementation());
                v.push_back(obj);
            }

            pageLines.insert({pageIndex, v});
        }
    }

    return;
}

void PdfWriter::drawRectangleToPage(Php::Parameters &params) 
{
    if (params[0] < 0) {
        Php::warning << "Unable to write to a negative page number" << std::flush;
        throw Php::Exception("Cannot write to a negative page");
    }

    if (params[1].size() > 0) {
        double pageIndex = params[0];

	auto search = pageRectangles.find(pageIndex);
	if(search != pageRectangles.end()) {
	    for(auto &iter : params[1]) {
                PdfRectangle *obj = new PdfRectangle(*(PdfRectangle*) iter.second.implementation());
		search->second.push_back(obj);
	    }
	} else {
	    std::vector<PdfRectangle*> v;
	    for(auto &iter : params[1]) {
                PdfRectangle *obj = new PdfRectangle(*(PdfRectangle*) iter.second.implementation());
                v.push_back(obj);
            }

	    pageRectangles.insert({pageIndex, v});
	}
    }

    return;
}

void PdfWriter::writeTextToPage(Php::Parameters &params)
{
    if (params[0] < 0) {
        Php::warning << "Unable to write to a negative page number" << std::flush;
        throw Php::Exception("Cannot write to a negative page");
    }

    if (!defaultText) {
        Php::warning << "No font set" << std::flush;
        throw Php::Exception("No font set!");
    }

    if (params[1].size() > 0) {
        double pageIndex = params[0];

        auto search = pageText.find(pageIndex);
        if (search != pageText.end()) 
        {
            for (auto &iter : params[1]) {
                PdfText *obj = new PdfText(*(PdfText *) iter.second.implementation());
                search->second.push_back(obj);
            }
        } else {
            std::vector<PdfText*> v;

            for (auto &iter : params[1]) {
                PdfText *obj = new PdfText(*(PdfText *) iter.second.implementation());
                v.push_back(obj);
            }

            pageText.insert({pageIndex,v});
        }
    }

    return;
}

const long double RADIAN_PER_DEGREE = 0.0174533;

std::vector<std::vector<double>> multiplyMatrices(std::vector<std::vector<double>> firstMatrix, std::vector<std::vector<double>> secondMatrix)
{
    std::vector<std::vector<double>> result(2, std::vector<double>(2, 0));

    result[0][0] = (firstMatrix[0][0]*secondMatrix[0][0])+(firstMatrix[0][1]*secondMatrix[1][0]);
    result[0][1] = (firstMatrix[0][0]*secondMatrix[0][1])+(firstMatrix[0][1]*secondMatrix[1][1]);

    result[1][0] = (firstMatrix[1][0]*secondMatrix[0][0])+(firstMatrix[1][1]*secondMatrix[1][0]);
    result[1][1] = (firstMatrix[1][0]*secondMatrix[1][0])+(firstMatrix[1][1]*secondMatrix[1][1]);

    return result;
}

void positionText(int pageRotation, PDFRectangle mediaBox, AbstractContentContext *contentContext, double long scale, double long x, double long y)
{
    double a = 1;
    double b = 0;
    double c = 0;
    double d = 1;
    double e = 0;
    double f = 0;

    switch (pageRotation) {
        case 0:
            e = x;
            f = y;

            break;
        case 90:
            // Use UpperRightX because the page was rotated so Y is the width
            if (mediaBox.UpperRightX > mediaBox.UpperRightY) {
                std::vector<std::vector<double>> scaling(2, std::vector<double>(2, 0));
                std::vector<std::vector<double>> rotation(2, std::vector<double>(2, 0));

                scaling[0][0]  = scale;
                scaling[1][1]  = scale;

                rotation[0][0] = cosl(RADIAN_PER_DEGREE*pageRotation);
                rotation[0][1] = sinl(RADIAN_PER_DEGREE*pageRotation);
                rotation[1][0] = -1*rotation[0][1];
                rotation[1][1] = rotation[0][0];

                std::vector<std::vector<double>> result = multiplyMatrices(scaling, rotation);
                a = result[0][0];
                b = result[0][1];
                c = result[1][0];
                d = result[1][1];
                e = mediaBox.UpperRightX-y;
                f = x;
            } else {
                e = x;
                f = y;
            }

            break;
        case 180:
            a = -1;
            d = -1;
            e = mediaBox.UpperRightX-x;
            f = mediaBox.UpperRightY-y;

            break;
        case 270:
            // Rotated 270 degrees into portrait mode
            if (mediaBox.UpperRightX > mediaBox.UpperRightY) {
                std::vector<std::vector<double>> scaling(2, std::vector<double>(2, 0));
                std::vector<std::vector<double>> rotation(2, std::vector<double>(2, 0));

                scaling[0][0]  = scale;
                scaling[1][1]  = scale;

                rotation[0][0] = cosl(RADIAN_PER_DEGREE*pageRotation);
                rotation[0][1] = sinl(RADIAN_PER_DEGREE*pageRotation);
                rotation[1][0] = -1*rotation[0][1];
                rotation[1][1] = rotation[0][0];

                std::vector<std::vector<double>> result = multiplyMatrices(scaling, rotation);
                a = result[0][0];
                b = result[0][1];
                c = result[1][0];
                d = result[1][1];
                e = y;
                f = mediaBox.UpperRightY-x;
            } else {
                e = x;
                f = y;
            }

            break;
    }

    contentContext->Tm(a, b, c, d, e, f);
}

void writeTextToPdf(double long x, double long y, std::string text, int pageRotation, const PDFRectangle &mediaBox, AbstractContentContext *contentContext, AbstractContentContext::TextOptions *options)
{
    double r = (unsigned char)((options->colorValue >> 16) & 0xFF);
    double g = (unsigned char)((options->colorValue >> 8) & 0xFF);
    double b = (unsigned char)(options->colorValue & 0xFF);
    double fontSize = options->fontSize;

    contentContext->BT();
    contentContext->rg(r/255, g/255, b/255);

    if (options->font) {
        contentContext->Tf(options->font, options->fontSize);
        positionText(pageRotation, mediaBox, contentContext, 1, x, y);
    } else {
        positionText(pageRotation, mediaBox, contentContext, fontSize, x, y);
    }

    if (text.find("\n") != std::string::npos) {
        std::vector<std::string> tokens = split(text,'\n');
        for (std::vector<std::string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {
            contentContext->Tj(*it);
            contentContext->Td(0,-1.1*fontSize);
        }

        contentContext->ET();
        return;
    }

    contentContext->Tj(text);
    contentContext->ET();
}

void PdfWriter::writeText(PdfText *obj, int pageRotation, const PDFRectangle &mediaBox, AbstractContentContext *contentContext) {
    AbstractContentContext::TextOptions * options = NULL;

    if (obj->getFontSize() || obj->getFont() || obj->getColor() != 0) {
        int _fontSize = 10;
        std::string fontName(obj->getFont().stringValue());
        if (fontName.empty()) {
            fontName.assign(defaultFontName);
        }

        if (obj->getFontSize()) {
            _fontSize = (int)obj->getFontSize();
        }

        options = this->getFont(fontName, _fontSize, obj->getColor());
    } else {
        options = defaultText;
    }

    writeTextToPdf((double)obj->getX(), (double)obj->getY(), obj->getText(), pageRotation, mediaBox, contentContext, options);
}

void PdfWriter::writeImage(PdfImage *image, AbstractContentContext *contentContext) {
    if (image->getWidth() > 0 || image->getIndex() > 0) {
        AbstractContentContext::ImageOptions opt;
        opt.transformationMethod = AbstractContentContext::eFit;
        opt.fitProportional = true;

        if (image->getWidth() > 0) {
            opt.boundingBoxWidth = image->getWidth();
            opt.boundingBoxHeight = image->getHeight();
        }

        if (image->getIndex() > 0) {
            opt.imageIndex = (double)image->getIndex();
        }

        contentContext->DrawImage((double)image->getX(), (double)image->getY(), image->getImagePath(), opt);

        return;
    }

    contentContext->DrawImage((double)image->getX(), (double)image->getY(), image->getImagePath());
}

void PdfWriter::writeRectangle(PdfRectangle *rect, AbstractContentContext *contentContext) {
    AbstractContentContext::GraphicOptions options(AbstractContentContext::eFill, AbstractContentContext::eRGB, rect->getColor());

    contentContext->DrawRectangle((double)rect->getX(), (double)rect->getY(), (double)rect->getWidth(), (double)rect->getHeight(), options);
}

void PdfWriter::writeLine(PdfLine *line, AbstractContentContext *contentContext) {
    AbstractContentContext::GraphicOptions options(AbstractContentContext::eStroke, AbstractContentContext::eRGB, line->getColor(), line->getWidth());
    DoubleAndDoublePairList pathPoints;

    pathPoints.push_back(DoubleAndDoublePair(line->getX(), line->getY()));
    pathPoints.push_back(DoubleAndDoublePair(line->getX2(), line->getY2()));

    contentContext->DrawPath(pathPoints, options);
}

void PdfWriter::writePdf(Php::Parameters &params) {
    int pagePos = 0;
    while (true) {
        PDFModifiedPage thePage(&writer, pagePos, true);
        AbstractContentContext* contentContext = thePage.StartContentContext();

        if (!contentContext) {
            break;
        }

        PDFObject* page = writer.GetModifiedFileParser().ParsePage(pagePos);
        PDFPageInput pageInput(&writer.GetModifiedFileParser(), page);
        PDFRectangle mediaBox = pageInput.GetMediaBox();
        int pageRotation      = pageInput.GetRotate();

        // see if there are images destined for this page and write them at the same time
        auto images = pageImages.find(pagePos);

        if (images != pageImages.end()) {
            for (const auto& i : images->second) {
                this->writeImage(i, contentContext);
                delete i;
            }

            pageImages.erase(images);
        }

        auto rectangles = pageRectangles.find(pagePos);
        if (rectangles != pageRectangles.end()) {
            for (const auto &i : rectangles->second) {
                this->writeRectangle(i, contentContext);
                delete i;
            }
        }

        auto lines = pageLines.find(pagePos);
        if (lines != pageLines.end()) {
            for (const auto &i : lines->second) {
                this->writeLine(i, contentContext);
                delete i;
            }
        }

        // find text - written last to be on "top"
        auto textSearch = pageText.find(pagePos);
        if (textSearch != pageText.end()) {
            for (const auto &iter : textSearch->second) {
                this->writeText(iter, pageRotation, mediaBox, contentContext);
                delete iter;
            }
        }

        thePage.EndContentContext();
        thePage.WritePage();

        pagePos++;
    }

    pageText.clear();
    pageImages.clear();
    pageRectangles.clear();
    pageLines.clear();
    writer.EndPDF();

    // requesting only particular pages
    if (!params.empty()) {
        char tempFileName[] = "/tmp/fileXXXXXX";
        int fd;

        if ((fd = mkstemp(tempFileName)) == -1) {
            fprintf(stderr, "Failed with error %s\n", strerror(errno));
            return;
        }

        unlink(tempFileName);

        FILE *fh = fdopen(fd, "w");

        std::ifstream  src(_outputFileName.c_str(), std::ios::binary);
        std::ofstream  dst(tempFileName, std::ios::binary);

        dst << src.rdbuf();

        PDFPageRange pageRange;

        pageRange.mType = PDFPageRange::eRangeTypeSpecific;
        std::size_t found = std::string::npos;
        std::string::size_type sz;

        for (auto &iter : params[0]) {
            found = iter.second.stringValue().find('-');
            if(found != std::string::npos) {
                std::vector<std::string> v = split(iter.second.stringValue(), '-');
                pageRange.mSpecificRanges.push_back(ULongAndULong(std::stol (v[0],&sz)-1, std::stol (v[1],&sz)-1));
            } else {
                long li_dec = std::stol(iter.second.stringValue(),&sz) - 1;
                pageRange.mSpecificRanges.push_back(ULongAndULong(li_dec, li_dec));
            }
        }

        // Need a second instance of the writer for encrypted documents to work
        PDFWriter writer2;
        EStatusCode status;
        status = writer2.StartPDF(_outputFileName, ePDFVersion14);
        if (status != eSuccess) {
            Php::warning << "Unable to open " << _outputFileName << std::flush;
            throw Php::Exception("Unable to open file");
        }

        writer2.AppendPDFPagesFromPDF(tempFileName, pageRange);
        writer2.EndPDF();
        fclose(fh);
    }
}

Php::Value PdfWriter::getAllFonts() {
    return getFonts();
}

const int UNSUPPORTED_TYPE = 0;
const int PDF_TYPE = 1;
const int IMAGE_TYPE = 2;

int determineMimeType(const std::string &filename) {
    std::fstream file(filename, std::ios::in | std::ios::binary);
    char buffer[32];
    file.read(buffer, 31);
    file.close();

    if (strstr(buffer, "%PDF")) {
        return PDF_TYPE;
    }
    
    //jpg
    if (strstr(buffer, "\xFF\xD8\xFF\xE0") || 
        strstr(buffer, "\xFF\xD8\xFF\xE2") || 
        strstr(buffer, "\xFF\xD8\xFF\xE3") || 
        strstr(buffer, "\xFF\xD8\xFF\xE1")) {
        return IMAGE_TYPE;
    }
    
    //png
    if (strstr(buffer, "\x89PNG\r\n\x1A\n")) {
        return IMAGE_TYPE;
    }
    
    return UNSUPPORTED_TYPE;
}

Php::Value PdfWriter::combine(Php::Parameters &params) {
    std::string destinationFile = params[1];
    EStatusCode status;
    PDFWriter pdfWriter;
    PDFParser* parser = NULL;

    unsigned long pagesCount;
    double width;
    double height;

    const double PAGE_WIDTH = 612;
    const double PAGE_HEIGHT = 792;

    status = pdfWriter.StartPDF(destinationFile, ePDFVersion17);
    if (status != eSuccess) {
        Php::warning << "Unable to open " << destinationFile << std::flush;
        throw Php::Exception("Unable to open destination file");
    }

    PDFDocumentCopyingContext* copyingContext = NULL;

    for (auto &iter : params[0]) {
        if (file_exists(iter.second.stringValue()))
        {
            int mime = determineMimeType(iter.second.stringValue());
            if (UNSUPPORTED_TYPE == mime) {
                return false;
            }

            if (PDF_TYPE == mime) {
                copyingContext = pdfWriter.CreatePDFCopyingContext(iter.second.stringValue());
                if (copyingContext)
                {
                    parser = copyingContext->GetSourceDocumentParser();
                    pagesCount = parser->GetPagesCount();
                    for (unsigned long i=0; i < pagesCount; ++i) 
                    {
                        // parse dimensions
                        PDFPageInput pageInput(parser,parser->ParsePage(i));
                        PDFRectangle mediaBox =  pageInput.GetMediaBox();

                        // create form
                        EStatusCodeAndObjectIDType result = copyingContext->CreateFormXObjectFromPDFPage(i,ePDFPageBoxMediaBox);
                        ObjectIDType reusableObjectID = result.second;

                        // create target page
                        PDFPage* page = new PDFPage();
                        page->SetMediaBox(PDFRectangle(0, 0, PAGE_WIDTH, PAGE_HEIGHT));

                        PageContentContext* pageContent = pdfWriter.StartPageContentContext(page);

                        height = mediaBox.UpperRightY - mediaBox.LowerLeftY;
                        width  = mediaBox.UpperRightX - mediaBox.LowerLeftX;

                        if (width != 612 || height != 792)
                        {
                            double scaledWidth = (PAGE_WIDTH-40)/width;
                            double scaledHeight = (PAGE_HEIGHT-40)/height;

                            /* we scale the height to place the document in the middle of the page */
                            double heightPlacementAdjustment = 20;
                            double tmpAdjustment = ((PAGE_HEIGHT-height)*scaledHeight)/2;
                            if (height < PAGE_HEIGHT) {
                                heightPlacementAdjustment += tmpAdjustment;
                            }

                            // place scaled page
                            pageContent->q();
                            pageContent->cm(scaledWidth < scaledHeight ? scaledWidth:scaledHeight, 0, 0, scaledWidth < scaledHeight ? scaledWidth:scaledHeight, 20, heightPlacementAdjustment);
                            pageContent->Do(page->GetResourcesDictionary().AddFormXObjectMapping(reusableObjectID));
                            pageContent->Q();
                        } else {
                            pageContent->q();
                            pageContent->cm(1, 0, 0, 1, 0, 0);
                            pageContent->Do(page->GetResourcesDictionary().AddFormXObjectMapping(reusableObjectID));
                            pageContent->Q();
                        }

                        pdfWriter.EndPageContentContext(pageContent);
                        pdfWriter.WritePageAndRelease(page);
                    }

                    delete parser;
                }
            } else {
                AbstractContentContext::ImageOptions opt1;
                opt1.boundingBoxHeight = PAGE_HEIGHT-40;
                opt1.boundingBoxWidth = PAGE_WIDTH-40;
                opt1.transformationMethod = AbstractContentContext::eFit;
                opt1.fitProportional = true;

                PDFPage* page = new PDFPage();
                page->SetMediaBox(PDFRectangle(0, 0, PAGE_WIDTH, PAGE_HEIGHT));
                PageContentContext* cxt = pdfWriter.StartPageContentContext(page);
                cxt->DrawImage(10, 10, iter.second.stringValue(), opt1);
                pdfWriter.EndPageContentContext(cxt);
                pdfWriter.WritePageAndRelease(page);
            }
        } else {
            pdfWriter.EndPDF();
            return false;
        }
    }

    pdfWriter.EndPDF();

    return true;
}

