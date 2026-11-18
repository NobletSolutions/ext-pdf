//  Created by Nathanael Noblet on 2019-06-29.
//  Copyright © 2019 Nathanael Noblet. All rights reserved.

#include "pdf-hasher.h"
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <PDFWriter/InputFile.h>
#include <PDFWriter/EStatusCode.h>
#include <PDFWriter/PDFParser.h>
#include <PDFWriter/PDFDictionary.h>
#include <PDFWriter/PDFIndirectObjectReference.h>
#include "openssl/sha.h"

const int BUFFERSIZE = 4096;

inline bool file_exists(const std::string &filename ) {
    struct stat statBuffer;
    return (stat(filename.c_str(), &statBuffer) == 0);
}

std::string hashPdfFileContents(std::string inputFilePath) {
    if (!file_exists(inputFilePath)) {
        Php::warning << "File " << inputFilePath << " doesn't exist" << std::flush;
        throw Php::Exception("File doesn't exist?");
    }

    long long infoStartPos = getInfoStartPosition(inputFilePath);
    std::FILE *inFile = std::fopen(inputFilePath.c_str(), "rb");

    char mdString[SHA256_DIGEST_LENGTH * 2 + 1];
    unsigned char md[SHA256_DIGEST_LENGTH];

    memset(mdString,'\0', SHA256_DIGEST_LENGTH);
    memset(md, '\0', SHA256_DIGEST_LENGTH);

    SHA256_CTX context;
    if (!SHA256_Init(&context)) {
        std::cerr << "Unable to initialize openssl context" << std::flush;
        return NULL;
    }

    char * buffer             = new char[BUFFERSIZE];
    long long currentPosition = 0;
    long long bytesRead       = 0;
    do {
        if (currentPosition < infoStartPos && currentPosition+BUFFERSIZE > infoStartPos) {
            long long bytesToPosition = infoStartPos-currentPosition;

            bytesRead = std::fread(buffer, sizeof(char), bytesToPosition, inFile);
            if(bytesRead != bytesToPosition) {
                std::cerr << "Didn't read the number of bytes we expected\n";
                std::cerr << "Read '" << bytesRead << "' but expected '" << bytesToPosition << "'\n";
                delete[] buffer;
                return NULL;
            }

            if (!SHA256_Update(&context, buffer, bytesToPosition)) {
                std::cerr << "Unable to add data to hash context" << std::flush;
                delete[] buffer;
                return NULL;
            }

            currentPosition += bytesRead;
            bytesRead = std::fread(buffer, sizeof(char), BUFFERSIZE, inFile);
            const char * endobj = std::strstr(buffer, "endobj");
            long endObjPos = 6+endobj-buffer;
            currentPosition += endObjPos;
            std::fseek(inFile, infoStartPos + endObjPos, SEEK_SET);
            continue;
        }

        bytesRead = std::fread(buffer, sizeof(unsigned char), BUFFERSIZE, inFile);

        if (!SHA256_Update(&context, buffer, bytesRead)) {
            std::cerr << "Unable to add data to hash context" << std::flush;
            delete[] buffer;
            return NULL;
        }

        currentPosition += bytesRead;
        if (bytesRead != BUFFERSIZE) {
            break;
        }
    } while(true);

    std::fclose(inFile);
    delete[] buffer;

    if (!SHA256_Final(md, &context)) {
        std::cerr << "Unable to finalize hash" << std::flush;
        return NULL;
    }

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&mdString[i*2], "%02x", (unsigned int)md[i]);
    }

    return std::string(mdString);
}

long long getInfoStartPosition(std::string inputFilePath) {
    long long infoBytePosition = 0;
    InputFile inFile;
    PDFParser parser;
    RefCountPtr<PDFDictionary> trailer;

    inFile.OpenFile(inputFilePath);
    PDFHummus::EStatusCode status = parser.StartPDFParsing(inFile.GetInputStream());

    if (status == PDFHummus::eSuccess) {
        trailer = parser.GetTrailer();
        if (trailer != NULL && trailer->Exists("Info")) {
            PDFIndirectObjectReference *info = (PDFIndirectObjectReference*)trailer->QueryDirectObject("Info");
            if (info != NULL) {
                XrefEntryInput *infoXrefEntry = parser.GetXrefEntry(info->mObjectID);
                infoBytePosition = (long long)infoXrefEntry->mObjectPosition;
                info->Release();
                parser.ResetParser();
            }
        }
    }

    inFile.CloseFile();

    return infoBytePosition;
}
