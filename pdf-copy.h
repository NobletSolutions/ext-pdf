
#ifndef PDF_COPY_H_
#define PDF_COPY_H_

#include <phpcpp.h>

class PdfCopy: public Php::Base {
private:
    std::string filePath;
    int copies = 1;
    std::string pages;
public:
    PdfCopy();
    PdfCopy(const PdfCopy &obj);
    void __construct(Php::Parameters &params);
    Php::Value getCopies();
    Php::Value getPages();
    std::vector<int> getTokenizedPages();
    Php::Value getFilePath();
    Php::Value __toString();
};

#endif /* PDF_COPY_H_ */
