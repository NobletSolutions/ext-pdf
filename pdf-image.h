/*
 * pdf-image.h
 *
 *  Created on: Dec 29, 2017
 *      Author: gnat
 */

#ifndef PDF_IMAGE_H_
#define PDF_IMAGE_H_

#include <phpcpp.h>

class PdfImage : public Php::Base {
private:
    int64_t x = 0;
    int64_t y = 0;
    int64_t width = 0;
    int64_t height = 0;
    int64_t index = 0;
    std::string imagepath;
public:
    PdfImage();
    PdfImage(PdfImage &obj);
    void __construct(Php::Parameters &params);
    Php::Value getX();
    Php::Value getY();
    Php::Value getIndex();
    Php::Value getWidth();
    Php::Value getHeight();
    Php::Value getImagePath();
};

#endif /* PDF_IMAGE_H_ */
