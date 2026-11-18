/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <string.h>
#include <cerrno>
#include <climits>
#include <cstdlib>
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

static int parsePageNumber(const std::string& token, unsigned long pageCount) {
    const char* digits = token.c_str();
    char* end;
    errno = 0;
    const long page = std::strtol(digits, &end, 10);
    if (errno == ERANGE || page <= 0 || page > INT_MAX || end != digits + token.size()) {
        Php::warning << "Unable to convert '" << token << "' to a page number" << std::flush;
        throw Php::Exception("Unable to convert '" + token + "' to a page number");
    }
    if (static_cast<unsigned long>(page) > pageCount) {
        throw Php::Exception("Selected page is outside the PDF");
    }

    return static_cast<int>(page);
}

std::vector<int> PdfCopy::getTokenizedPages(unsigned long pageCount) {
    if (pages.empty()) {
        return {};
    }

    const std::string cleanedPages = std::regex_replace(pages, std::regex(" "), "");
    std::vector<int> result;
    size_t start = 0;
    while (true) {
        const size_t comma = cleanedPages.find(',', start);
        const std::string token = cleanedPages.substr(start, comma - start);
        const size_t dash = token.find('-');
        const int first = parsePageNumber(token.substr(0, dash), pageCount);
        const int last = dash == std::string::npos ? first : parsePageNumber(token.substr(dash + 1), pageCount);
        if (first > last) {
            throw Php::Exception("Invalid page range: " + token);
        }

        for (int page = first; page < last; ++page) {
            result.push_back(page);
        }
        result.push_back(last);

        if (comma == std::string::npos) {
            return result;
        }
        start = comma + 1;
    }
}

Php::Value PdfCopy::getFilePath() {
    return filePath;
}

Php::Value PdfCopy::__toString() {
    std::string ouput = filePath+ " Copies: "+std::to_string(copies)+" Pages: "+pages;
    return ouput;
}
