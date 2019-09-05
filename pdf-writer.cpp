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
#include <iostream>
#include <fstream>
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

AbstractContentContext::TextOptions * PdfWriter::getFont(std::string requestedFont, double inFontSize) {
    AbstractContentContext::TextOptions * options;
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

        options = new AbstractContentContext::TextOptions(_font, inFontSize, AbstractContentContext::eRGB, 0);
        return options;
    }

    Php::warning << "No such font: " << requestedFont << std::flush;
    throw Php::Exception("No such font");
}

void PdfWriter::setFont(Php::Parameters &params) {
    defaultFontName.assign(params[0].stringValue());
    if (params.size() == 2) {
        defaultText = this->getFont(params[0].stringValue(), (double)params[1]);
    } else if (params.size() == 1) {
        defaultText = this->getFont(params[0].stringValue(), 10);
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
//        Php::out << "["<< pageIndex << "] Added (pushed) " << image->getImagePath() << std::endl;
        search->second.push_back(image);
    } else {
//        Php::out << "["<< pageIndex << "] Added (new) " << image->getImagePath() << std::endl;

        std::vector<PdfImage*> v = {image};
        pageImages.insert({pageIndex,v});
    }
}

void PdfWriter::writeTextToPage(Php::Parameters &params) {
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
        if (search != pageText.end()) {
//            Php::out <<"["<< pageIndex << "] Added (pushed) " << params[1].size() << " text(s)" << std::endl;

            for (auto &iter : params[1]) {
                PdfText *obj = new PdfText(*(PdfText *) iter.second.implementation());
//                Php::out << "\t" << obj->getText() << std::endl;
                search->second.push_back(obj);
            }
        } else {
            std::vector<PdfText*> v;
//            Php::out <<"["<< pageIndex << "] Added (new) " << params[1].size() << " text(s)" << std::endl;

            for (auto &iter : params[1]) {
                PdfText *obj = new PdfText(*(PdfText *) iter.second.implementation());
//                Php::out << "\t" << obj->getText() << std::endl;
                v.push_back(obj);
            }

            pageText.insert({pageIndex,v});
        }

        return;
    }

    return;
}

void PdfWriter::writeText(PdfText *obj, AbstractContentContext *contentContext) {
//    Php::out << "Get X" << obj->getX() << " " << obj->getText() << std::endl;
    AbstractContentContext::TextOptions * options;

    if (obj->getFontSize() || obj->getFont() ) {
        int _fontSize = 10;
        std::string fontName(obj->getFont().stringValue());
        if(obj->getFontSize()) {
            _fontSize = (int)obj->getFontSize();
        }

        if (fontName.empty()){
            fontName.assign(defaultFontName);
        }

//        Php::out << "Setting font " << fontName << std::endl;
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

        return;
    }

    contentContext->WriteText((double) obj->getX(), (double) obj->getY(), obj->getText(), *options);
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

void PdfWriter::writePdf(Php::Parameters &params) {
    // Iterate and print keys and values of unordered_map
    for (const auto &n : pageText ) {
//      Php::out << "PageText: [" << n.first << "] TextCount: [" << n.second.size() << "]\n";

        PDFModifiedPage thePage(&writer, n.first, true);
        AbstractContentContext* contentContext = thePage.StartContentContext();

        if (contentContext) {
            // iterate over each PdfText point
            for (const auto &iter : n.second) {
                this->writeText(iter, contentContext);
                //Php::out << "Delete iter" << std::endl;
                delete iter; //deleted because it was new PdfText() in our writeTextToPage calls
            }

            // see if there are images destined for this page and write them at the same time
            auto images = pageImages.find(n.first);

            if (images != pageImages.end()) {
                //Php::out << "Have Images: " << images->second.size() << std::endl;

                for (const auto& i : images->second) {
                    //Php::out << "\tImage: [" << i->getImagePath() << "]\n";
                    this->writeImage(i,contentContext);
                    delete i;
                }

                pageImages.erase(images);
            }

            thePage.EndContentContext();
        }

        thePage.WritePage();
    }

//    Php::out << "Image Pages Left: " << pageImages.size() << std::endl;
    for( const auto& i : pageImages ) {
        PDFModifiedPage thePage(&writer, i.first, true);
        AbstractContentContext* contentContext = thePage.StartContentContext();
        if (contentContext) {
            // Php::out << "PageImages:[" << i.first << "] NumImages: ["<< i.second.size() <<"]\n";
            for (const auto& image : i.second) {
                this->writeImage(image, contentContext);
                delete image;
            }
            thePage.EndContentContext();
        }

        thePage.WritePage();
    }

    pageText.clear();
    pageImages.clear();
    writer.EndPDF();

    // requesting only particular pages
    if (!params.empty()) {
        std::string tempfile = std::tmpnam(nullptr);

        std::ifstream  src(_outputFileName.c_str(), std::ios::binary);
        std::ofstream  dst(tempfile.c_str(), std::ios::binary);

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

        writer2.AppendPDFPagesFromPDF(tempfile, pageRange);
        writer2.EndPDF();
        std::remove(tempfile.c_str());
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

    //Php::out << "Creating " << destinationFile << std::endl;
    status = pdfWriter.StartPDF(destinationFile, ePDFVersion17);
    if (status != eSuccess) {
        Php::warning << "Unable to open " << destinationFile << std::flush;
        throw Php::Exception("Unable to open destination file");
    }

    PDFDocumentCopyingContext* copyingContext = NULL;

    for (auto &iter : params[0]) {
        if (file_exists(iter.second.stringValue()))
        {
            //Php::out << "\tWith: " << iter.second.stringValue() << std::endl;

            int mime = determineMimeType(iter.second.stringValue());
            if (UNSUPPORTED_TYPE == mime) {
                //Php::warning << iter.second.stringValue() << " is an unsupported file type" << std::flush;
                return false;
            }

            if (PDF_TYPE == mime) {
                //Php::warning << "PDF embedding: " << iter.second.stringValue() << std::flush;
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

                            // Php::out << "U: " << mediaBox.UpperRightX << "x" << mediaBox.UpperRightY << std::endl;
                            // Php::out << "L: " << mediaBox.LowerLeftX << "x" << mediaBox.LowerLeftY << std::endl;
                            // Php::out << "DIV: " << PAGE_WIDTH << "/" << width << " " << scaledWidth << std::endl;
                            // Php::out << "DIV: " << PAGE_HEIGHT << "/" << height << " " << scaledHeight << std::endl;

                            /* we scale the height to place the document in the middle of the page */
                            double heightPlacementAdjustment = 20;
                            double tmpAdjustment = ((PAGE_HEIGHT-height)*scaledHeight)/2;
                            if (height < PAGE_HEIGHT) {
                                //Php::out << "height < PAGE_HEIGHT " << height << " < " << PAGE_HEIGHT << std::endl;
                                heightPlacementAdjustment += tmpAdjustment;
                            }
                            // else if (height > PAGE_HEIGHT) {
                            //     //Php::out << "height > PAGE_HEIGHT " << height << " > " << PAGE_HEIGHT << std::endl;
                            //     heightPlacementAdjustment -= tmpAdjustment;
                            // }

                            // Php::out << "Adjustment Amount: " << tmpAdjustment << std::endl;
                            // Php::out << "heightPlacementAdjustment: " << heightPlacementAdjustment << std::endl;

                            // place scaled page
                            pageContent->q();
                            pageContent->cm(scaledWidth < scaledHeight ? scaledWidth:scaledHeight, 0, 0, scaledWidth < scaledHeight ? scaledWidth:scaledHeight, 20, heightPlacementAdjustment);
                            pageContent->Do(page->GetResourcesDictionary().AddFormXObjectMapping(reusableObjectID));
                            pageContent->Q();
                        }
                        else
                        {
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

                //Php::warning << "Image embedding: " << iter.second.stringValue() << std::flush;

                PDFPage* page = new PDFPage();
                page->SetMediaBox(PDFRectangle(0, 0, PAGE_WIDTH, PAGE_HEIGHT));
                PageContentContext* cxt = pdfWriter.StartPageContentContext(page);
                cxt->DrawImage(10, 10, iter.second.stringValue(), opt1);
                pdfWriter.EndPageContentContext(cxt);
                pdfWriter.WritePageAndRelease(page);
            }
        } else {
            pdfWriter.EndPDF();
            //Php::warning << "\t " << iter.second.stringValue() << " doesn't exist!" << std::flush;
            return false;
        }
    }

    pdfWriter.EndPDF();

    return true;
}

