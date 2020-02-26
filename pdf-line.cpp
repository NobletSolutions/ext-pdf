
#include <phpcpp.h>
#include <string.h>
#include "pdf-line.h"

PdfLine::PdfLine() = default;

void PdfLine::__construct(Php::Parameters &params) {
    x = params[0].numericValue();
    y = params[1].numericValue();
    x2 = params[2].numericValue();
    y2 = params[3].numericValue();

    if (params.size() > 4) {
        if (params.size() != 7) {
            throw Php::Exception("All RGB Value are required");
        }
        red = params[4].numericValue();
        green = params[5].numericValue();
        blue = params[6].numericValue();
    }
}

PdfLine::PdfLine(const PdfLine &obj) {
    x  = obj.x;
    y  = obj.y;
    x2 = obj.x2;
    y2 = obj.y2;
    red = obj.red;
    green = obj.green;
    blue = obj.blue;
    width = obj.width;
}

Php::Value PdfLine::getX() {
    return x;
}

Php::Value PdfLine::getY() {
    return y;
}

Php::Value PdfLine::getX2() {
    return x2;
}

Php::Value PdfLine::getY2() {
    return y2;
}

Php::Value PdfLine::getRed() {
    return red;
}

Php::Value PdfLine::getGreen() {
    return green;
}

Php::Value PdfLine::getBlue() {
    return blue;
}

void PdfLine::setWidth(Php::Parameters &params) {
    width = params[0].numericValue();
}

Php::Value PdfLine::getWidth() {
    return width;
}

unsigned long PdfLine::getColor() {
    return 65536*red + 256*green + blue;
}

