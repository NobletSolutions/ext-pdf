/*
 * pdf-writer.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: gnat
 */

#include <phpcpp.h>
#include <string.h>
#include <iostream>
#include "pdf-text.h"

PdfText::PdfText() = default;

void PdfText::__construct(Php::Parameters &params) {
    x = params[0].floatValue();
    y = params[1].floatValue();
    text = params[2].stringValue();

    if (params.size() >= 4) {
        fontSize = params[3].numericValue();
    }

    if (params.size() == 5) {
        font = params[4].stringValue();
    }
}

PdfText::PdfText(const PdfText &obj) {
	x = obj.x;
	y = obj.y;
	text = obj.text;
    color = obj.color;

	if (obj.fontSize) {
		fontSize = obj.fontSize;
	}

	if (!obj.font.empty()) {
		font = obj.font;
	}
}

double PdfText::getAdjustedX(int64_t pageWidth) {
    if (x > 1) {
        return x;
    }

    if (x == 0) {
        return 0;
    }

    if (x == 1) {
        return pageWidth;
    }

    return pageWidth * x;
}

double PdfText::getAdjustedY(int64_t pageHeight) {
    if (y > 1) {
        return y;
    }

    if (y == 0) {
        return 0;
    }

    if (y == 1) {
        return pageHeight;
    }

    return pageHeight * y;
}

Php::Value PdfText::getX() {
    return (int64_t)x;
}

Php::Value PdfText::getY() {
    return (int64_t)y;
}

Php::Value PdfText::getText() {
    return text;
}

Php::Value PdfText::__toString() {
    return getText();
}

Php::Value PdfText::getFontSize() {
    return fontSize;
}

Php::Value PdfText::getFont() {
    return font;
}

Php::Value PdfText::getColor() {
    return color;
}

void PdfText::setColor(Php::Parameters &params) {
    if (params.size() == 1) {
        color = params[0].numericValue();
        return;
    }

    if (params.size() == 3) {
        color =  65536*params[0].numericValue() + 256*params[1].numericValue() + params[2].numericValue();
        return;
    }

    Php::warning << "Invalid color: Pass a color as a single value or RGB" << std::flush;
    throw Php::Exception("Invalid color: Pass a color as a single value or RGB");
}