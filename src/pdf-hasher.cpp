//  Created by Nathanael Noblet on 2019-06-29.
//  Copyright © 2019 Nathanael Noblet. All rights reserved.

#include "pdf-hasher.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <memory>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <PDFWriter/PDFIndirectObjectReference.h>
#include <PDFWriter/PDFObjectCast.h>
#include <PDFWriter/PDFParser.h>
#include <PDFWriter/PDFSymbol.h>

namespace {

constexpr size_t bufferSize = 4096;
constexpr char endMarker[] = "endobj";

struct FileCloser {
    void operator()(std::FILE* file) const {
        std::fclose(file);
    }
};

class HashInput final : public IByteReaderWithPosition {
public:
    explicit HashInput(const std::string& path) {
        const int descriptor = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (descriptor < 0) {
            throw Php::Exception("Unable to open file for PDF hashing");
        }
        file.reset(fdopen(descriptor, "rb"));
        if (!file) {
            close(descriptor);
            throw Php::Exception("Unable to open file for PDF hashing");
        }

        struct stat status;
        if (fstat(descriptor, &status) != 0 || !S_ISREG(status.st_mode)) {
            throw Php::Exception("PDF hashing requires a regular file");
        }
        length = status.st_size;
    }

    LongFilePositionType size() const {
        return length;
    }

    LongBufferSizeType Read(Byte* buffer, LongBufferSizeType bytes) override {
        const auto count = std::fread(buffer, 1, bytes, file.get());
        if (std::ferror(file.get())) {
            throw Php::Exception("Unable to read file for PDF hashing");
        }
        return count;
    }

    bool NotEnded() override {
        return !std::feof(file.get()) && GetCurrentPosition() < length;
    }

    LongFilePositionType GetCurrentPosition() override {
        const auto position = ftello(file.get());
        if (position < 0) {
            throw Php::Exception("Unable to determine position while hashing PDF");
        }
        return position;
    }

    void SetPosition(LongFilePositionType position) override {
        if (position < 0 || position > length || fseeko(file.get(), position, SEEK_SET) != 0) {
            throw Php::Exception("Invalid file position while hashing PDF");
        }
    }

    void SetPositionFromEnd(LongFilePositionType offset) override {
        if (offset < 0) {
            throw Php::Exception("Invalid file position while hashing PDF");
        }
        // PDFWriter's backwards scan can request a block larger than the file.
        SetPosition(length - std::min(length, offset));
    }

    void Skip(LongBufferSizeType bytes) override {
        const auto position = GetCurrentPosition();
        if (position > length || bytes > static_cast<unsigned long long>(length - position)) {
            throw Php::Exception("Invalid file position while hashing PDF");
        }
        SetPosition(position + bytes);
    }

private:
    std::unique_ptr<std::FILE, FileCloser> file;
    LongFilePositionType length = 0;
};

void readExactly(HashInput& input, Byte* buffer, size_t size) {
    if (input.Read(buffer, size) != size) {
        throw Php::Exception("Unexpected end of file while hashing PDF");
    }
}

struct ExcludedRange {
    LongFilePositionType start;
    LongFilePositionType end;
};

ExcludedRange findExcludedRange(HashInput& input) {
    const ExcludedRange wholeFile{input.size(), input.size()};
    PDFParser parser;
    if (parser.StartPDFParsing(&input) != PDFHummus::eSuccess) {
        // Preserve the whole-file digest when PDFWriter cannot parse the file.
        return wholeFile;
    }

    auto* trailer = parser.GetTrailer();
    if (!trailer) {
        return wholeFile;
    }
    RefCountPtr<PDFObject> info(trailer->QueryDirectObject("Info"));
    if (!info || info->GetType() == PDFObject::ePDFObjectNull) {
        return wholeFile;
    }
    if (info->GetType() != PDFObject::ePDFObjectIndirectObjectReference) {
        throw Php::Exception("Invalid PDF Info reference");
    }

    const auto* reference = static_cast<PDFIndirectObjectReference*>(info.GetPtr());
    const auto* entry = parser.GetXrefEntry(reference->mObjectID);
    if (!entry || entry->mType == eXrefEntryDelete || entry->mType == eXrefEntryUndefined) {
        return wholeFile;
    }
    const bool compressed = entry->mType == eXrefEntryStreamObject;
    const auto start = entry->mObjectPosition;
    if (start <= 0 || start >= input.size()) {
        if (compressed) {
            return wholeFile;
        }
        throw Php::Exception("Invalid PDF Info position");
    }

    if (!compressed) {
        PDFObjectCastPtr<PDFDictionary> dictionary(parser.ParseNewObject(reference->mObjectID));
        PDFObjectCastPtr<PDFSymbol> end(dictionary.GetPtr() ? parser.GetObjectParser().ParseNewObject() : nullptr);
        if (!end || end->GetValue() != endMarker) {
            throw Php::Exception("Unable to read the PDF Info object");
        }
    }

    // Preserve legacy digests for block-aligned positions and compressed entries.
    // For a compressed entry the legacy code used the object-stream number as a byte offset.
    if (start % bufferSize == 0) {
        return wholeFile;
    }
    input.SetPosition(start);
    std::array<Byte, bufferSize + 1> window{};
    const auto windowSize = static_cast<size_t>(std::min<LongFilePositionType>(bufferSize, input.size() - start));
    readExactly(input, window.data(), windowSize);
    const auto* begin = reinterpret_cast<const char*>(window.data());
    if (const auto* marker = std::strstr(begin, endMarker)) {
        return {start, start + (marker - begin) + static_cast<LongFilePositionType>(sizeof(endMarker) - 1)};
    }
    if (compressed) {
        return wholeFile;
    }

    // A standalone object can extend beyond the legacy search window.
    input.SetPosition(start);
    PDFParserTokenizer tokenizer;
    tokenizer.SetReadStream(&input);
    for (auto token = tokenizer.GetNextToken(); token.first; token = tokenizer.GetNextToken()) {
        if (token.second == endMarker) {
            return {start, start + tokenizer.GetRecentTokenPosition() + static_cast<LongFilePositionType>(token.second.size())};
        }
    }
    throw Php::Exception("Unable to find the end of the PDF Info object");
}

void hashThrough(HashInput& input, EVP_MD_CTX* context, LongFilePositionType end) {
    std::array<Byte, bufferSize> buffer;
    auto remaining = end - input.GetCurrentPosition();
    while (remaining > 0) {
        const auto size = static_cast<size_t>(std::min<LongFilePositionType>(buffer.size(), remaining));
        readExactly(input, buffer.data(), size);
        if (EVP_DigestUpdate(context, buffer.data(), size) != 1) {
            throw Php::Exception("Unable to update PDF hash");
        }
        remaining -= size;
    }
}

} // namespace

std::string hashPdfFileContents(const std::string& inputFilePath) {
    HashInput input(inputFilePath);
    const auto excluded = findExcludedRange(input);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1) {
        throw Php::Exception("Unable to initialize PDF hash");
    }

    input.SetPosition(0);
    hashThrough(input, context.get(), excluded.start);
    input.SetPosition(excluded.end);
    hashThrough(input, context.get(), input.size());

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest;
    unsigned int digestSize = 0;
    if (EVP_DigestFinal_ex(context.get(), digest.data(), &digestSize) != 1) {
        throw Php::Exception("Unable to finalize PDF hash");
    }

    static constexpr char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(digestSize * 2);
    for (unsigned int index = 0; index < digestSize; ++index) {
        const auto byte = digest[index];
        result.push_back(hex[byte >> 4]);
        result.push_back(hex[byte & 0x0f]);
    }
    return result;
}
