/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <string.h>
#include <iostream>
#include "pdf-copy.h"
#include <sys/stat.h>
#include <regex>

PdfCopy::PdfCopy() = default;

inline bool file_exists(const std::string &filename ) {
    struct stat statBuffer;
    return (stat(filename.c_str(), &statBuffer) == 0);
}

void PdfCopy::__construct(Php::Parameters &params) {
    if (!file_exists(params[0])) {
        Php::warning << "File " << params[0] << " doesn't exist" << std::flush;
        throw Php::Exception("File doesn't exist?");
    }
    
    filePath = params[0].stringValue();

    if (params.size() >= 2) {
        copies = params[1].numericValue();
    }

    if (params.size() == 3) {
        pages = params[2].stringValue();
        std::regex validChars("^[-, 0-9]*$");
        std::cmatch m;
        if (!std::regex_match(pages.c_str(), m,validChars)) {
            Php::warning << "Only 0-9, '-' and ',' characters are allowed" << std::flush;
            throw Php::Exception("Only 0-9, '-' and ',' characters are allowed");
        }
    }
}

PdfCopy::PdfCopy(const PdfCopy &obj) {
    if (!file_exists(obj.filePath)) {
        Php::warning << "File " << obj.filePath << " doesn't exist" << std::flush;
        throw Php::Exception("File doesn't exist?");
    }

    filePath = obj.filePath;
    copies = obj.copies;
    pages = obj.pages;
}


Php::Value PdfCopy::getCopies() {
    return copies;
}

Php::Value PdfCopy::getPages() {
    return pages;
}

std::vector<int> split(std::string &str, char c = ',')
{
    // std::cout << "Spliting: '" << str << "'" << std::endl;
    std::vector<int> result;
    std::size_t pos;
    std::size_t start = 0;
    std::string sub;
    int converted = 0;
    while((pos = str.find(c, start)) != std::string::npos){
        sub = str.substr(start, pos-start); 
        // std::cout << sub  << "= str.substr("<<start<<", "<<pos<<"-"<<start<<")"<<std::endl;
        start = pos+1;
        
        if (sub.find('-') != std::string::npos) {
            if(sub.size() <= 2) {
                Php::warning << "Invalid page range: " << sub << " found" << std::flush;
                throw Php::Exception("Invalid page range: " + sub + " found");
            }
            auto const range = split(sub, '-');
            if (range.size() != 2) {
                Php::warning << sub << " contains too many ranges" << std::flush;
                throw Php::Exception("Range " + sub + " contains too many ranges");
            }

            for (int start = range[0]; start <= range[1]; start++) {
                result.push_back(start);
            }
        } else {
            converted = atoi(sub.c_str());
            if (converted <= 0) {
                Php::warning << "Unable to convert '" << sub << "' to a page number" << std::flush;
                throw Php::Exception("Unable to convert '" + sub + "' to a page number");
            }

            result.push_back(converted);   
        }
    }

    sub = str.substr(start);
    converted = atoi(sub.c_str());
    if (converted <= 0) {
        Php::warning << "Unable to convert '" << sub << "' to a page number" << std::flush;
        throw Php::Exception("Unable to convert '" + sub + "' to a page number");
    }

    result.push_back(converted);

    return result;
}

std::vector<int> PdfCopy::getTokenizedPages() {
    std::vector<int> _pages;
    if (pages.empty()) {
        return _pages;
    }
    std::regex whitespace(" ");
    std::string cleanedPages = std::regex_replace(pages, whitespace, "");
    
    return split(cleanedPages, ',');
}

Php::Value PdfCopy::getFilePath() {
    return filePath;
}

Php::Value PdfCopy::__toString() {
    std::string ouput = filePath+ " Copies: "+std::to_string(copies)+" Pages: "+pages;
    return ouput;
}
