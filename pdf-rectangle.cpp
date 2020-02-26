
#include <phpcpp.h>
#include <string.h>
#include "pdf-rectangle.h"

PdfRectangle::PdfRectangle() = default;

void PdfRectangle::__construct(Php::Parameters &params) {
    x = params[0].numericValue();
    y = params[1].numericValue();
    width = params[2].numericValue();
    height = params[3].numericValue();

    if (params.size() > 4) {
	if (params.size() != 7) {
            throw Php::Exception("All RGB Value are required");	
	}
        red = params[4].numericValue();
        green = params[5].numericValue();
        blue = params[6].numericValue();
    }
}

PdfRectangle::PdfRectangle(const PdfRectangle &obj) {
    x = obj.x;
    y = obj.y;
    width = obj.width;
    height = obj.height;
    red = obj.red;
    green = obj.green;
    blue = obj.blue;
}

Php::Value PdfRectangle::getX() {
    return x;
}

Php::Value PdfRectangle::getY() {
    return y;
}

Php::Value PdfRectangle::getWidth() {
    return width;
}

Php::Value PdfRectangle::getHeight() {
    return height;
}

Php::Value PdfRectangle::getRed() {
    return red;
}

Php::Value PdfRectangle::getGreen() {
    return green;
}

Php::Value PdfRectangle::getBlue() {
    return blue;
}

unsigned long PdfRectangle::getColor() {
    return 65536*red + 256*green + blue;
}

