/*
 * pdf-text.h
 *
 *  Created on: Dec 29, 2017
 *      Author: gnat
 */

#ifndef PDF_TEXT_H_
#define PDF_TEXT_H_

#include <phpcpp.h>

class PdfText : public Php::Base {
private:
    int64_t x = 0;
    int64_t y = 0;
    int64_t fontSize = 10;
    int64_t color = 0;
    std::string text;
    std::string font;
public:
    PdfText();
    PdfText(const PdfText &obj);
    void __construct(Php::Parameters &params);
    Php::Value getX();
    Php::Value getY();
    Php::Value getText();
    Php::Value getFontSize();
    Php::Value getFont();
    Php::Value getColor();
    void setColor(Php::Parameters &params);
};

#endif /* PDF_TEXT_H_ */
